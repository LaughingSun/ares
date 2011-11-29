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

#ifndef __appares_newproject_h
#define __appares_newproject_h

#include <crystalspace.h>

#include <wx/wx.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/xrc/xmlres.h>

class UIManager;
class Asset;

struct NewProjectCallback : public csRefCount
{
  virtual void OkPressed (const csArray<Asset>& assets) = 0;
};

class NewProjectDialog : public wxDialog
{
private:
  UIManager* uiManager;
  iVFS* vfs;

  void OnOkButton (wxCommandEvent& event);
  void OnCancelButton (wxCommandEvent& event);
  void OnSearchFileButton (wxCommandEvent& event);
  void OnAddAssetButton (wxCommandEvent& event);
  void OnDelAssetButton (wxCommandEvent& event);
  void OnAssetSelected (wxListEvent& event);
  void OnAssetDeselected (wxListEvent& event);

  long selIndex;
  csRef<NewProjectCallback> callback;

  void SetPathFile (const char* path, const char* file);
  void ScanCSNode (csString& msg, iDocumentNode* node);

public:
  NewProjectDialog (wxWindow* parent, UIManager* uiManager, iVFS* vfs);
  ~NewProjectDialog();

  void Show (NewProjectCallback* cb);
  void SetFilename (const char* filename);

  DECLARE_EVENT_TABLE ();
};

#endif // __appares_newproject_h
