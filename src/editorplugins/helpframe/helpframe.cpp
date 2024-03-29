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

#include <crystalspace.h>
#include "helpframe.h"
#include "physicallayer/pl.h"

#include <wx/html/htmlwin.h>

SCF_IMPLEMENT_FACTORY (HelpFrame)

//--------------------------------------------------------------------------

BEGIN_EVENT_TABLE(HelpFrame, wxFrame)
  EVT_CLOSE(HelpFrame :: OnClose)
  //EVT_TOOL(id, func)
END_EVENT_TABLE()

csStringID HelpFrame::ID_Show = csInvalidStringID;

//--------------------------------------------------------------------------

HelpFrame::HelpFrame (iBase* parent) :
  scfImplementationType (this, parent)
{
}

void HelpFrame::OnClose (wxCloseEvent& event)
{
  if (event.CanVeto ())
  {
    wxFrame::Show (false);
    event.Veto ();
  }
  else
  {
    Destroy ();
  }
}

void HelpFrame::LoadHelpFile (const char* helpfile)
{
  wxHtmlWindow* help_Html = XRCCTRL (*this, "help_Html", wxHtmlWindow);
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  csRef<iDataBuffer> path = vfs->GetRealPath (helpfile);
  help_Html->LoadPage (wxString::FromUTF8 (path->GetData ()));
}

void HelpFrame::Show ()
{
  wxFrame::Show (true);
  Raise ();
  LoadHelpFile ("/aresdocs/index.html");
}

bool HelpFrame::Initialize (iObjectRegistry* object_reg)
{
  HelpFrame::object_reg = object_reg;
  csRef<iCelPlLayer> pl = csQueryRegistry<iCelPlLayer> (object_reg);
  ID_Show = pl->FetchStringID ("Show");
  return true;
}

void HelpFrame::SetApplication (iAresEditor* app)
{
  HelpFrame::app = app;
}

void HelpFrame::SetTopLevelParent (wxWindow* parent)
{
  wxXmlResource::Get()->LoadFrame (this, parent, wxT ("HelpFrame"));

  LoadHelpFile ("/aresdocs/index.html");

  Layout ();
  Fit ();
  SetSize (900, 800);
}

HelpFrame::~HelpFrame ()
{
}


