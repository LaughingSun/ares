/*
The MIT License

Copyright (c) 2012 by Jorrit Tyberghein

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

#ifndef __CEL_PF_GAMECONTROL__
#define __CEL_PF_GAMECONTROL__

#include "cstypes.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "csutil/scf.h"
#include "physicallayer/propclas.h"
#include "physicallayer/propfact.h"
#include "physicallayer/facttmpl.h"
#include "propclass/dynworld.h"
#include "celtool/stdpcimp.h"
#include "celtool/stdparams.h"
#include "include/igamecontrol.h"
#include "tools/uitools/inventory.h"

struct iCelEntity;
struct iObjectRegistry;
struct iPcCamera;
struct iMouseDriver;
struct iGraphics3D;
struct iGraphics2D;
struct iPcDynamicWorld;
struct iPcDynamicMove;
struct iDynamicObject;
struct iFont;
struct iEngine;
struct iVirtualClock;
struct iPcInventory;
struct iPcMessenger;
class csSimplePixmap;

/**
 * Factory for game controller.
 */
CEL_DECLARE_FACTORY (GameController)

enum DragType
{
  DRAGTYPE_NORMAL = 0,
  DRAGTYPE_ROTY
};

/**
 * This is a game controller property class.
 */
class celPcGameController : public scfImplementationExt1<
	celPcGameController, celPcCommon, iPcGameController>
{
private:
  // For SendMessage parameters.
  static csStringID id_name;
  static csStringID id_template;
  static csStringID id_factory;
  static csStringID id_destination;

  csRef<iGraphics3D> g3d;
  csRef<iGraphics2D> g2d;
  csRef<iVirtualClock> vc;
  csRef<iEngine> engine;
  csRef<iCelEntity> player;
  csRef<iPcMessenger> messenger;
  csRef<iUIInventory> uiInventory;
  csRef<CS::Physics::iPhysicalSystem> dyn;

  /// Find the object that is pointed at in the center of the screen.
  iDynamicObject* FindCenterObject (CS::Physics::iRigidBody*& hitBody,
      csVector3& start, csVector3& isect);

  // For dragging.
  csRef<iMouseDriver> mouse;
  csRef<CS::Physics::iPhysicalSector> dynSys;
  csRef<CS::Physics::iJoint> dragJoint;
  float oldLinearDampening;
  float oldAngularDampening;
  float dragDistance;	// Distance depends on type of dragging.
  iDynamicObject* dragobj;
  csVector3 dragOrigin;
  csVector3 dragAnchor;
  DragType dragType;

  // Icons.
  csSimplePixmap* iconCursor;
  csSimplePixmap* iconEye;
  csSimplePixmap* iconBook;
  csSimplePixmap* iconDot;
  csSimplePixmap* iconCheck;
  void LoadIcons ();

  // ID's
  csStringID classNoteID;
  csStringID classInfoID;
  csStringID classPickUpID;
  csStringID classNoActivate;
  csStringID attrDragType;
  csStringID msgActivate;

  csRef<iPcDynamicWorld> dynworld;
  void TryGetDynworld ();

  csWeakRef<iPcCamera> pccamera;
  csWeakRef<iPcDynamicMove> pcdynmove;
  void TryGetCamera ();
  void TryGetDynmove ();
  bool FindPlayer ();

  // For actions.
  enum actionids
  {
    action_startdrag = 0,
    action_stopdrag,
    action_examine,
    action_pickup,
    action_activate,
    action_spawn,
    action_createentity,
    action_inventory,
    action_teleport,
  };

  // For properties.
  //enum propids
  //{
    //propid_counter = 0,
    //propid_max
  //};
  static PropertyHolder propinfo;

  //csRef<iMessageDispatcher> dispatcher_print;

  void PickUpDynObj (iDynamicObject* dynobj);

  void FindSiblingPropertyClasses ();

public:
  celPcGameController (iObjectRegistry* object_reg);
  virtual ~celPcGameController ();

  void SelectEntity (iCelEntity* entity, const char* command);
  void SelectTemplate (iCelEntityTemplate* tpl, const char* command);

  iPcDynamicWorld* GetDynWorld () { TryGetDynworld (); return dynworld; }

  virtual void Examine ();
  virtual bool StartDrag ();
  virtual void StopDrag ();
  virtual void PickUp ();
  virtual void Activate ();
  virtual void Inventory ();
  virtual void Spawn (const char* factname);
  virtual void CreateEntity (const char* tmpname, const char* name);
  virtual void Teleport (const char* entityname);

  virtual bool PerformActionIndexed (int idx,
      iCelParameterBlock* params, celData& ret);

  virtual void TickEveryFrame ();
};

#endif // __CEL_PF_GAMECONTROL__

