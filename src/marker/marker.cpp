/*
The MIT License

Copyright (c) 2011 by Jorrit Tyberghein

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "cssysdef.h"

#include "marker.h"

#include "csutil/event.h"
#include "csgeom/math3d.h"
#include "iutil/objreg.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "iengine/sector.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/mesh.h"
#include "cstool/pen.h"


CS_PLUGIN_NAMESPACE_BEGIN(MarkerManager)
{

static float SqDistance2d (const csVector2& p1, const csVector2& p2)
{
  csVector2 d = p1 - p2;
  return d*d;
}

//--------------------------------------------------------------------------------

//#define PUSH_FACTOR 2.5f
#define PUSH_FACTOR 0.1f
#define PULL_FACTOR 0.1f
#define STRING_LENGTH 200.0f

// Always return 1 or -1.
static int PosSign (float a, bool sgnNeg)
{
  if (a < 0) return -1;
  else if (a > 0) return 1;
  else return sgnNeg ? -1 : 1;
}

GraphView::GraphView (MarkerManager* mgr) : scfImplementationType (this), mgr (mgr), visible (false)
{
  draggingMarker = 0;
}

#if 0
csVector2 GraphView::CalculatePush (const csVector2& self, const csVector2& other, float fw, float fh)
{
  csVector2 push;
  //push.x = PUSH_FACTOR / (self.x - other.x + PosSign (self.x - other.x, self.x > fw/2.0f));
  //push.y = PUSH_FACTOR / (self.y - other.y + PosSign (self.y - other.y, self.y > fh/2.0f));
  bool r = rand () % 2;
  push.x = PUSH_FACTOR / (self.x - other.x + PosSign (self.x - other.x, r));
  r = rand () % 2;
  push.y = PUSH_FACTOR / (self.y - other.y + PosSign (self.y - other.y, r));
  return push;
}
#else
csVector2 GraphView::CalculatePush (const csVector2& self, const csVector2& other, float fw, float fh)
{
  csVector2 push;
  //push.x = PUSH_FACTOR / (self.x - other.x + PosSign (self.x - other.x, self.x > fw/2.0f));
  //push.y = PUSH_FACTOR / (self.y - other.y + PosSign (self.y - other.y, self.y > fh/2.0f));
  float dist = sqrt (SqDistance2d (self, other));
  if (dist < 0.00001f)
    return csVector2 (float (rand () % 10) / 10.0f, float (rand () % 10) / 10.0f);	//@@@
  push = PUSH_FACTOR * (self-other) / dist;
  return push;
}
#endif

bool GraphView::IsLinked (const char* n1, const char* n2)
{
  csHash<GraphLink,csString>::GlobalIterator it = links.GetIterator ();
  while (it.HasNext ())
  {
    GraphLink& l = it.Next ();
    if (l.node1 == n1 && l.node2 == n2) return true;
    if (l.node2 == n1 && l.node1 == n2) return true;
  }
  return false;
}

void GraphView::UpdateFrame ()
{
  int fw = mgr->GetG2D ()->GetWidth ();
  int fh = mgr->GetG2D ()->GetHeight ();
  float seconds = mgr->GetVC ()->GetElapsedSeconds ();

  csHash<GraphNode,csString>::GlobalIterator it = nodes.GetIterator ();
  while (it.HasNext ())
  {
    csString key;
    GraphNode& node = it.Next (key);
    if (node.marker == draggingMarker) continue;
    csVector2 pos = node.marker->GetPosition ();

    csVector2 push (0, 0);
    push += CalculatePush (pos, csVector2 (-5, pos.y), fw, fh);
    push += CalculatePush (pos, csVector2 (fw+5, pos.y), fw, fh);
    push += CalculatePush (pos, csVector2 (pos.x, -5), fw, fh);
    push += CalculatePush (pos, csVector2 (pos.x, fh+5), fw, fh);
    int cnt = 4;

    csHash<GraphNode,csString>::GlobalIterator it2 = nodes.GetIterator ();
    while (it2.HasNext ())
    {
      csString key2;
      GraphNode& node2 = it2.Next (key2);
      if (node.marker != node2.marker)
      {
	csVector2 pos2 = node2.marker->GetPosition ();
	csVector2 p = CalculatePush (pos, pos2, fw, fh);
	//printf ("p=%g,%g\n", p.x, p.y);
	if (IsLinked (key, key2))
	{
	  float dist = sqrt (SqDistance2d (pos, pos2));
	  if (dist > STRING_LENGTH)
	  {
	    csVector2 up = p.Unit ();
	    float pull = PULL_FACTOR * (((dist-STRING_LENGTH) / STRING_LENGTH) + 1.0f);
	    //printf ("pull=%f\n", pull);
	    p -= up * pull;
	  }
	}
	push += p;
	cnt++;
      }
    }
    node.push = push / float (cnt);
    pos += node.push * (seconds * 5000.0f);
    if (pos.x > fw-76) pos.x = fw-76;
    else if (pos.x < 76) pos.x = 76;
    if (pos.y > fh-14) pos.y = fh-14;
    else if (pos.y < 14) pos.y = 14;
    node.marker->SetPosition (pos);
  }
}

void GraphView::Render3D ()
{
  MarkerColor* white = static_cast<MarkerColor*> (mgr->FindMarkerColor ("white"));
  GraphNode n;
  csHash<GraphLink,csString>::GlobalIterator it = links.GetIterator ();
  while (it.HasNext ())
  {
    GraphLink& l = it.Next ();
    GraphNode& node1 = nodes.Get (l.node1, n);
    GraphNode& node2 = nodes.Get (l.node2, n);
    csVector2 pos1 = node1.marker->GetPosition ();
    csVector2 pos2 = node2.marker->GetPosition ();
    csPen* pen = white->GetPen (1);
    pen->DrawLine (pos1.x, pos1.y, pos2.x, pos2.y);
  }
}

void GraphView::SetVisible (bool v)
{
  visible = v;
  csHash<GraphNode,csString>::GlobalIterator it = nodes.GetIterator ();
  while (it.HasNext ())
  {
    iMarker* marker = it.Next ().marker;
    marker->SetVisible (v);
  }
}

void GraphView::Clear ()
{
  csHash<GraphNode,csString>::GlobalIterator it = nodes.GetIterator ();
  while (it.HasNext ())
  {
    iMarker* marker = it.Next ().marker;
    mgr->DestroyMarker (marker);
  }
  nodes.DeleteAll ();
  links.DeleteAll ();
}

class MarkerCallback : public scfImplementation1<MarkerCallback,iMarkerCallback>
{
private:
  GraphView* view;

public:
  MarkerCallback (GraphView* view) : scfImplementationType (this),
    view (view) { }
  virtual ~MarkerCallback () { }
  virtual void StartDragging (iMarker* marker, iMarkerHitArea* area,
      const csVector3& pos, uint button, uint32 modifiers)
  {
    view->SetDraggingMarker (marker);
  }
  virtual void MarkerWantsMove (iMarker* marker, iMarkerHitArea* area,
      const csVector3& pos)
  {
    marker->SetPosition (csVector2 (pos.x, pos.y));
  }
  virtual void MarkerWantsRotate (iMarker* marker, iMarkerHitArea* area,
      const csReversibleTransform& transform) { }
  virtual void StopDragging (iMarker* marker, iMarkerHitArea* area)
  {
    view->SetDraggingMarker (0);
  }
};

void GraphView::CreateNode (const char* name)
{
  int fw = mgr->GetG2D ()->GetWidth ();
  int fh = mgr->GetG2D ()->GetHeight ();
  iMarker* marker = mgr->CreateMarker ();
  // @@@ Color should come from outside.
  iMarkerColor* white = mgr->FindMarkerColor ("white");
  marker->RoundedBox2D (MARKER_2D, csVector3 (-75, -13, 0),
    csVector3 (75, 13, 0), 10, white);
  marker->Text (MARKER_2D, csVector3 (0, 0, 0), name, white, true);
  marker->SetSelectionLevel (1);
  marker->SetVisible (false);
  marker->SetPosition (csVector2 (fw / 2, fh / 2));

  iMarkerColor* yellow = mgr->FindMarkerColor ("yellow");
  iMarkerHitArea* hitArea = marker->HitArea (MARKER_2D, csVector3 (0, 0), 13, 0, yellow);
  csRef<MarkerCallback> cb;
  cb.AttachNew (new MarkerCallback (this));
  hitArea->DefineDrag (0, 0, MARKER_2D, CONSTRAIN_NONE, cb);

  GraphNode node;
  node.marker = marker;
  node.push.Set (0, 0);
  nodes.Put (name, node);
}

void GraphView::RemoveNode (const char* name)
{
  GraphNode node = nodes.Get (name, GraphNode ());
  if (node.marker)
  {
    nodes.DeleteAll (name);
    mgr->DestroyMarker (node.marker);
  }
}

//--------------------------------------------------------------------------------

csPen* MarkerColor::GetOrCreatePen (int level)
{
  csPen* pen;
  if (size_t (level) >= pens.GetSize () || pens[level] == 0)
  {
    pen = new csPen (mgr->g2d, mgr->g3d);
    pens.Put (level, pen);
  }
  else
  {
    pen = pens[level];
  }
  return pen;
}

void MarkerColor::SetRGBColor (int selectionLevel, float r, float g, float b, float a)
{
  csPen* pen = GetOrCreatePen (selectionLevel);
  pen->SetColor (r, g, b, a);
}

void MarkerColor::SetPenWidth (int selectionLevel, float width)
{
  csPen* pen = GetOrCreatePen (selectionLevel);
  pen->SetPenWidth (width);
}

//---------------------------------------------------------------------------------------

static csVector3 TransPointWorld (
    const csOrthoTransform& camtrans,
    const csReversibleTransform& meshtrans,
    MarkerSpace space,
    const csVector3& point)
{
  if (space == MARKER_CAMERA)
    return camtrans.This2Other (point);
  else if (space == MARKER_OBJECT)
    return meshtrans.This2Other (point);
  else if (space == MARKER_WORLD)
    return meshtrans.This2Other (point);
  else
    return point;
}

static csVector3 TransPointCam (
    const csOrthoTransform& camtrans,
    const csReversibleTransform& meshtrans,
    MarkerSpace space,
    const csVector3& point)
{
  if (space == MARKER_CAMERA)
    return point;
  else if (space == MARKER_OBJECT)
    return camtrans.Other2This (meshtrans.This2Other (point));
  else if (space == MARKER_WORLD)
    return camtrans.Other2This (meshtrans.This2Other (point));
  else if (space == MARKER_CAMERA_AT_MESH)
  {
    csReversibleTransform tr;
    tr.SetOrigin (meshtrans.GetOrigin ());
    return camtrans.Other2This (tr.This2Other (point));
  }
  else
    return point;
}

csVector3 MarkerHitArea::GetWorldCenter () const
{
  MarkerManager* mgr = marker->GetMarkerManager ();
  const csOrthoTransform& camtrans = mgr->camera->GetTransform ();
  const csReversibleTransform& meshtrans = marker->GetTransform ();
  return TransPointWorld (camtrans, meshtrans, GetSpace (), GetCenter ());
}

csVector2 MarkerHitArea::GetPerspectiveRadius (iView* view, float z) const
{
  iCamera* camera = view->GetCamera ();
  csVector4 r4 (radius, radius, z, 1.0f);
  csVector4 t = camera->GetProjectionMatrix () * r4;
  csVector2 r (t.x / t.w - 1.0f, t.y / t.w - 1.0f);
  return view->NormalizedToScreen (r);
}

void MarkerHitArea::DefineDrag (uint button, uint32 modifiers,
      MarkerSpace constrainSpace, uint32 constrainPlane,
      iMarkerCallback* cb)
{
  MarkerDraggingMode* dm = new MarkerDraggingMode ();
  dm->cb = cb;
  dm->button = button;
  dm->modifiers = modifiers;
  dm->constrainSpace = constrainSpace;
  dm->constrainPlane = constrainPlane;
  draggingModes.Push (dm);
}

MarkerDraggingMode* MarkerHitArea::FindDraggingMode (uint button, uint32 modifiers) const
{
  for (size_t i = 0 ; i < draggingModes.GetSize () ; i++)
  {
    MarkerDraggingMode* dm = draggingModes[i];
    if (dm->button == button &&
	(dm->modifiers & CSMASK_MODIFIERS) == (modifiers & CSMASK_MODIFIERS))
    {
      return dm;
    }
  }
  return 0;
}

//---------------------------------------------------------------------------------------

const csReversibleTransform& Marker::GetTransform () const
{
  if (attachedMesh) return attachedMesh->GetMovable ()->GetTransform ();
  else return trans;
}

void Marker::Render3D ()
{
  if (!visible) return;

  const csOrthoTransform& camtrans = mgr->camera->GetTransform ();
  const csReversibleTransform& meshtrans = GetTransform ();
  for (size_t i = 0 ; i < lines.GetSize () ; i++)
  {
    MarkerLine& line = lines[i];
    csVector3 v1 = TransPointCam (camtrans, meshtrans, line.space, line.v1);
    csVector3 v2 = TransPointCam (camtrans, meshtrans, line.space, line.v2);
    // @@@ Do proper clipping?
    if (line.space == MARKER_2D || (v1.z > .5 && v2.z > .5))
    {
      int h = mgr->g2d->GetHeight ();
      csPen* pen = line.color->GetPen (selectionLevel);
      csVector2 s1, s2;
      if (line.space != MARKER_2D)
      {
        s1 = mgr->camera->Perspective (v1);
        s2 = mgr->camera->Perspective (v2);
      }
      else
      {
	s1.Set (v1.x, h-v1.y);
	s2.Set (v2.x, h-v2.y);
      }
 
      int x1 = int (s1.x);
      int y1 = h - int (s1.y);
      int x2 = int (s2.x);
      int y2 = h - int (s2.y);

      pen->DrawLine (pos.x + x1, pos.y + y1, pos.x + x2, pos.y + y2);

      if (line.arrow)
      {
        //float d = sqrt (SqDistance2d (s1, s2));
        int dx = (x2-x1) / 4;
        int dy = (y2-y1) / 4;
        int dxr = -(y2-y1) / 4;
        int dyr = (x2-x1) / 4;
        pen->DrawLine (pos.x + x2, pos.y + y2, pos.x + x2-dx+dxr, pos.y + y2-dy+dyr);
        pen->DrawLine (pos.x + x2, pos.y + y2, pos.x + x2-dx-dxr, pos.y + y2-dy-dyr);
      }
    }
  }

  for (size_t i = 0 ; i < roundedBoxes.GetSize () ; i++)
  {
    MarkerRoundedBox& box = roundedBoxes[i];
    csVector3 v1 = TransPointCam (camtrans, meshtrans, box.space, box.v1);
    csVector3 v2 = TransPointCam (camtrans, meshtrans, box.space, box.v2);
    // @@@ Do proper clipping?
    if (box.space == MARKER_2D || (v1.z > .5 && v2.z > .5))
    {
      int h = mgr->g2d->GetHeight ();
      csPen* pen = box.color->GetPen (selectionLevel);
      csVector2 s1, s2;
      if (box.space != MARKER_2D)
      {
        s1 = mgr->camera->Perspective (v1);
        s2 = mgr->camera->Perspective (v2);
      }
      else
      {
	s1.Set (v1.x, h-v1.y);
	s2.Set (v2.x, h-v2.y);
      }
 
      int x1 = int (s1.x);
      int y1 = h - int (s1.y);
      int x2 = int (s2.x);
      int y2 = h - int (s2.y);

      pen->DrawRoundedRect (pos.x + x1, pos.y + y1, pos.x + x2, pos.y + y2,
	  box.roundness);
    }
  }

  for (size_t i = 0 ; i < hitAreas.GetSize () ; i++)
  {
    MarkerHitArea* ha = hitAreas[i];
    if (ha->GetColor ())
    {
      const csVector3& center = ha->GetCenter ();
      csVector3 c = TransPointCam (camtrans, meshtrans, ha->GetSpace (), center);
      if (ha->GetSpace () == MARKER_2D || c.z > .5)
      {
        int h = mgr->g2d->GetHeight ();
	csVector2 s;
	if (ha->GetSpace () != MARKER_2D)
          s = mgr->camera->Perspective (c) + pos;
	else
          s.Set (pos.x + c.x, h - (pos.y + c.y));
	int x = int (s.x);
	int y = h - int (s.y);
	int mouseX = mgr->GetMouseX ();
	int mouseY = mgr->GetMouseY ();
	csVector2 r;
	if (ha->GetSpace () != MARKER_2D)
          r = ha->GetPerspectiveRadius (mgr->view, c.z);
	else
	  r.Set (ha->GetRadius (), ha->GetRadius ());
	bool selected = mouseX >= (x-r.x) && mouseX <= (x+r.x) &&
	  mouseY >= (y-r.y) && mouseY <= (y+r.y);
        csPen* pen = ha->GetColor ()->GetPen (
	    selected ? SELECTION_SELECTED : SELECTION_NONE);
	pen->DrawArc (x-r.x, y-r.y, x+r.x, y+r.y);
      }
    }
  }
}

void Marker::Render2D ()
{
  if (!visible) return;

  const csOrthoTransform& camtrans = mgr->camera->GetTransform ();
  const csReversibleTransform& meshtrans = GetTransform ();

  for (size_t i = 0 ; i < texts.GetSize () ; i++)
  {
    MarkerText& text = texts[i];
    csVector3 v = TransPointCam (camtrans, meshtrans, text.space, text.pos);
    // @@@ Do proper clipping?
    if (text.space == MARKER_2D || v.z > .5)
    {
      int h = mgr->g2d->GetHeight ();
      csPen* pen = text.color->GetPen (selectionLevel);
      csVector2 s;
      if (text.space != MARKER_2D)
        s = mgr->camera->Perspective (v);
      else
	s.Set (v.x, h-v.y);
 
      int x1 = int (s.x);
      int y1 = h - int (s.y);

      char* t = const_cast<char*>((const char*)text.text);
      if (text.centered)
        pen->WriteBoxed (mgr->GetFont (), pos.x + x1, pos.y + y1,
	  pos.x + x1, pos.y + y1, CS_PEN_TA_CENTER, CS_PEN_TA_CENTER, t);
      else
        pen->Write (mgr->GetFont (), pos.x + x1, pos.y + y1, t);
    }
  }
}

void Marker::Line (MarkerSpace space,
      const csVector3& v1, const csVector3& v2, iMarkerColor* color,
      bool arrow)
{
  MarkerLine line;
  line.space = space;
  line.v1 = v1;
  line.v2 = v2;
  line.color = static_cast<MarkerColor*> (color);
  line.arrow = arrow;
  lines.Push (line);
}

void Marker::RoundedBox2D (MarkerSpace space,
      const csVector3& corner1, const csVector3& corner2, int roundness,
      iMarkerColor* color)
{
  MarkerRoundedBox box;
  box.space = space;
  box.v1 = corner1;
  box.v2 = corner2;
  box.color = static_cast<MarkerColor*> (color);
  box.roundness = roundness;
  roundedBoxes.Push (box);
}

void Marker::Text (MarkerSpace space, const csVector3& pos,
      const char* text, iMarkerColor* color, bool centered)
{
  MarkerText txt;
  txt.space = space;
  txt.pos = pos;
  txt.color = static_cast<MarkerColor*> (color);
  txt.text = text;
  txt.centered = centered;
  texts.Push (txt);
}

void Marker::Clear ()
{
  lines.Empty ();
  roundedBoxes.Empty ();
  texts.Empty ();
}

iMarkerHitArea* Marker::HitArea (MarkerSpace space, const csVector3& center,
      float radius, int data, iMarkerColor* color)
{
  csRef<MarkerHitArea> hitArea;
  hitArea.AttachNew (new MarkerHitArea (this));
  hitArea->SetSpace (space);
  hitArea->SetCenter (center);
  hitArea->SetRadius (radius);
  hitArea->SetData (data);
  hitArea->SetColor (static_cast<MarkerColor*> (color));
  hitAreas.Push (hitArea);
  return hitArea;
}

void Marker::ClearHitAreas ()
{
  hitAreas.Empty ();
}

float Marker::CheckHitAreas (int x, int y, MarkerHitArea*& bestHitArea)
{
  const csOrthoTransform& camtrans = mgr->camera->GetTransform ();
  const csReversibleTransform& meshtrans = GetTransform ();
  csVector2 f (float (x), float (mgr->g2d->GetHeight () - y));
  bestHitArea = 0;
  float bestRadius = 10000000.0f;
  bool hit = false;
  for (size_t i = 0 ; i < hitAreas.GetSize () ; i++)
  {
    MarkerHitArea* hitArea = hitAreas[i];
    csVector3 c = TransPointCam (camtrans, meshtrans, hitArea->GetSpace (),
	hitArea->GetCenter ());
    if (hitArea->GetSpace () == MARKER_2D || c.z > .5)
    {
      csVector2 s;
      if (hitArea->GetSpace () != MARKER_2D)
        s = mgr->camera->Perspective (c) + pos;
      else
        s.Set (pos.x + c.x, mgr->g2d->GetHeight () - (pos.y + c.y));
      float d = sqrt (SqDistance2d (s, f));
      csVector2 r;
      if (hitArea->GetSpace () != MARKER_2D)
        r = hitArea->GetPerspectiveRadius (mgr->view, c.z);
      else
	r.Set (hitArea->GetRadius (), hitArea->GetRadius ());
      float radius = (r.x+r.y) / 2.0f;
      if (d <= radius && d <= bestRadius)
      {
	bestRadius = d;
	bestHitArea = hitArea;
	hit = true;
      }
    }
  }
  if (hit) return bestRadius;
  else return -1.0f;
}

//---------------------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (MarkerManager)

MarkerManager::MarkerManager (iBase *iParent)
  : scfImplementationType (this, iParent)
{  
  object_reg = 0;
  camera = 0;
  currentDraggingHitArea = 0;
  currentDraggingMode = 0;
}

MarkerManager::~MarkerManager ()
{
  // First remove the views before the markers because destroying the
  // views also causes markers to be deleted.
  graphViews.DeleteAll ();
  markers.DeleteAll ();
}

bool MarkerManager::Initialize (iObjectRegistry *object_reg)
{
  MarkerManager::object_reg = object_reg;
  engine = csQueryRegistry<iEngine> (object_reg);
  vc = csQueryRegistry<iVirtualClock> (object_reg);
  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  g2d = g3d->GetDriver2D ();

  font = g3d->GetDriver2D ()->GetFontServer ()->LoadFont (CSFONT_LARGE);

  return true;
}

void MarkerManager::SetView (iView* view)
{
  MarkerManager::view = view;
  MarkerManager::camera = view->GetCamera ();
}

static bool CheckConstrain (bool constrain, float start, float end, float restr)
{
  if (!constrain) return true;
  if (fabs (start - end) < 0.1f) return false;
  if (end < start && restr > start) return false;
  if (end > start && restr < start) return false;
  return true;
}

bool MarkerManager::FindPlanarIntersection (const csVector3& start, const csVector3& end,
    csVector3& newpos)
{
  uint32 cp = currentDraggingMode->constrainPlane;
  bool cpx = cp & CONSTRAIN_XPLANE;
  bool cpy = cp & CONSTRAIN_YPLANE;
  bool cpz = cp & CONSTRAIN_ZPLANE;

  if (!CheckConstrain (cpx, start.x, end.x, dragRestrict.x)) return false;
  if (!CheckConstrain (cpy, start.y, end.y, dragRestrict.y)) return false;
  if (!CheckConstrain (cpz, start.z, end.z, dragRestrict.z)) return false;

  iMarker* marker = currentDraggingHitArea->GetMarker ();

  // @@@ TODO! Only WORLD and OBJECT supported here!
  csVector3 st, en, dr;
  if (currentDraggingMode->constrainSpace == MARKER_OBJECT)
  {
    dr = marker->GetTransform ().Other2This (dragRestrict);
    st = marker->GetTransform ().Other2This (start);
    en = marker->GetTransform ().Other2This (end);
  }
  else if (currentDraggingMode->constrainSpace == MARKER_WORLD)
  {
    dr = dragRestrict;
    st = start;
    en = end;
  }
  else
  {
    printf ("Unsupported dragging mode %d!\n", currentDraggingMode->constrainSpace);
    fflush (stdout);
    return false;
  }

  if (cpx)
  {
    float dist = csIntersect3::SegmentXPlane (st, en, dr.x, newpos);
    if (dist > 0.8f)
    {
      newpos = st + (en-st).Unit () * 80.0f;
      newpos.x = dr.x;
    }
  }
  if (cpy)
  {
    float dist = csIntersect3::SegmentYPlane (st, en, dr.y, newpos);
    if (dist > 0.8f)
    {
      newpos = st + (en-st).Unit () * 80.0f;
      newpos.y = dr.y;
    }
  }
  if (cpz)
  {
    float dist = csIntersect3::SegmentZPlane (st, en, dr.z, newpos);
    if (dist > 0.8f)
    {
      newpos = st + (en-st).Unit () * 80.0f;
      newpos.z = dr.z;
    }
  }

  if (currentDraggingMode->constrainSpace == MARKER_OBJECT)
  {
    newpos = marker->GetTransform ().This2Other (newpos);
  }
  return true;
}

bool MarkerManager::HandlePlanarRotation (const csVector3& newpos, csMatrix3& m)
{
  uint32 cp = currentDraggingMode->constrainPlane;
  bool cprotx = cp & CONSTRAIN_ROTATEX;
  bool cproty = cp & CONSTRAIN_ROTATEY;
  bool cprotz = cp & CONSTRAIN_ROTATEZ;
  bool cpx = cp & CONSTRAIN_XPLANE;
  bool cpy = cp & CONSTRAIN_YPLANE;
  bool cpz = cp & CONSTRAIN_ZPLANE;

  iMarker* marker = currentDraggingHitArea->GetMarker ();
  csReversibleTransform newrot = marker->GetTransform ();
  csVector3 rel = newpos - newrot.GetOrigin ();
  if (rel < 0.01f) return false;
  csVector3 front = newrot.GetFront (), up = newrot.GetUp (), right = newrot.GetRight ();
  if (cproty)
  {
    up = rel.Unit ();
    if (cpx) front = right % up;
    else if (cpz) right = up % front;
    else return false;
  }
  else if (cprotz)
  {
    front = rel.Unit ();
    if (cpy) right = up % front;
    else if (cpx) up = front % right;
    else return false;
  }
  else if (cprotx)
  {
    right = rel.Unit ();
    if (cpz) up = front % right;
    else if (cpy) front = right % up;
    else return false;
  }
  else return false;
  m.Set (right.x, right.y, right.z, up.x, up.y, up.z, front.x, front.y, front.z);
  return true;
}

void MarkerManager::HandleDrag ()
{
  if (currentDraggingMode)
  {
    iMarker* marker = currentDraggingHitArea->GetMarker ();
    csVector3 newpos;
    bool do_drag = true;
    uint32 cp = currentDraggingMode->constrainPlane;
    if (currentDraggingMode->constrainSpace == MARKER_2D)
    {
      newpos.x = mouseX;
      newpos.y = mouseY;
      newpos.z = 0;
    }
    else
    {
      csVector2 v2d (mouseX, g2d->GetHeight () - mouseY);
      csVector3 v3d = camera->InvPerspective (v2d, 100.0f);
      csVector3 start = camera->GetTransform ().GetOrigin ();
      csVector3 end = camera->GetTransform ().This2Other (v3d);
      if (cp & CONSTRAIN_MESH)
      {
        iMeshWrapper* mesh = currentDraggingHitArea->GetMarker ()->GetAttachedMesh ();
        csFlags oldFlags = mesh->GetFlags ();
        mesh->GetFlags ().Set (CS_ENTITY_NOHITBEAM);
        csSectorHitBeamResult result = camera->GetSector ()->HitBeamPortals (
	    start, end);
        mesh->GetFlags ().SetAll (oldFlags.Get ());
        if (!result.mesh) do_drag = false;	// Safety
        newpos = result.isect;
      }
      else if (cp & CONSTRAIN_PLANE)
      {
        if (!FindPlanarIntersection (start, end, newpos))
	  return;
      }
      else
      {
        newpos = end - start;
        newpos.Normalize ();
        newpos = camera->GetTransform ().GetOrigin () + newpos * dragDistance;
      }
    }

    if (do_drag && currentDraggingMode->cb)
    {
      if (cp & CONSTRAIN_ROTATE)
      {
	csMatrix3 m;
	if (HandlePlanarRotation (newpos, m))
	{
	  csReversibleTransform newrot = marker->GetTransform ();
	  newrot.SetO2T (m);
	  currentDraggingMode->cb->MarkerWantsRotate (marker, currentDraggingHitArea, newrot);
	}
      }
      else
      {
	currentDraggingMode->cb->MarkerWantsMove (marker, currentDraggingHitArea, newpos);
      }
    }
  }
}

void MarkerManager::Frame2D ()
{
  for (size_t i = 0 ; i < graphViews.GetSize () ; i++)
    if (graphViews[i]->IsVisible ())
      graphViews[i]->UpdateFrame ();

  HandleDrag ();
  for (size_t i = 0 ; i < markers.GetSize () ; i++)
    markers[i]->Render2D ();
}

void MarkerManager::Frame3D ()
{
  for (size_t i = 0 ; i < graphViews.GetSize () ; i++)
    if (graphViews[i]->IsVisible ())
      graphViews[i]->Render3D ();

  for (size_t i = 0 ; i < markers.GetSize () ; i++)
    markers[i]->Render3D ();
}

void MarkerManager::StopDrag ()
{
  if (currentDraggingMode)
  {
    if (currentDraggingMode->cb)
      currentDraggingMode->cb->StopDragging (currentDraggingHitArea->GetMarker (),
	  currentDraggingHitArea);
    currentDraggingMode = 0;
    currentDraggingHitArea = 0;
  }
}

bool MarkerManager::OnMouseDown (iEvent& ev, uint but, int mouseX, int mouseY)
{
  MarkerHitArea* hitArea = FindHitArea (mouseX, mouseY);
  if (hitArea)
  {
    uint32 mod = csMouseEventHelper::GetModifiers (&ev);
    MarkerDraggingMode* dm = hitArea->FindDraggingMode (but, mod);
    if (dm)
    {
      currentDraggingHitArea = hitArea;
      currentDraggingMode = dm;
      const csOrthoTransform& camtrans = camera->GetTransform ();
      dragRestrict = hitArea->GetWorldCenter ();
      dragDistance = (dragRestrict - camtrans.GetOrigin ()).Norm ();
      if (dm->cb)
        dm->cb->StartDragging (hitArea->GetMarker (), hitArea, dragRestrict, but, mod);
      return true;
    }
  }

  return false;
}

bool MarkerManager::OnMouseUp (iEvent& ev, uint but, int mouseX, int mouseY)
{
  StopDrag ();
  return false;
}

bool MarkerManager::OnMouseMove (iEvent& ev, int mouseX, int mouseY)
{
  MarkerManager::mouseX = mouseX;
  MarkerManager::mouseY = mouseY;
  return false;
}

MarkerHitArea* MarkerManager::FindHitArea (int x, int y)
{
  MarkerHitArea* bestHitArea = 0;
  float bestRadius = 10000000.0f;
  for (size_t i = 0 ; i < markers.GetSize () ; i++)
    if (markers[i]->IsVisible ())
    {
      MarkerHitArea* hitArea;
      float d = markers[i]->CheckHitAreas (x, y, hitArea);
      if (d >= 0.0f && d < bestRadius)
      {
        bestRadius = d;
        bestHitArea = hitArea;
      }
    }
  return bestHitArea;
}

iMarker* MarkerManager::FindHitMarker (int x, int y, int& data)
{
  iMarker* bestMarker = 0;
  float bestRadius = 10000000.0f;
  for (size_t i = 0 ; i < markers.GetSize () ; i++)
    if (markers[i]->IsVisible ())
    {
      MarkerHitArea* hitArea;
      float d = markers[i]->CheckHitAreas (x, y, hitArea);
      if (d >= 0.0f && d < bestRadius)
      {
        bestRadius = d;
        bestMarker = markers[i];
        data = hitArea->GetData ();
      }
    }
  return bestMarker;
}

void MarkerManager::SetSelectionLevel (int level)
{
  for (size_t i = 0 ; i < markers.GetSize () ; i++)
    markers[i]->SetSelectionLevel (level);
}

iMarkerColor* MarkerManager::FindMarkerColor (const char* name)
{
  for (size_t i = 0 ; i < markerColors.GetSize () ; i++)
    if (markerColors[i]->GetNameString () == name)
      return markerColors[i];
  return 0;
}

iGraphView* MarkerManager::CreateGraphView ()
{
  csRef<GraphView> gv;
  gv.AttachNew (new GraphView (this));
  graphViews.Push (gv);
  return gv;
}

void MarkerManager::DestroyGraphView (iGraphView* view)
{
  graphViews.Delete (static_cast<GraphView*> (view));
}

}
CS_PLUGIN_NAMESPACE_END(MarkerManager)
