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

#include <crystalspace.h>
#include "../apparesed.h"
#include "uimanager.h"
#include "filereq.h"
#include "newproject.h"
#include "celldialog.h"
#include "dynfactdialog.h"

/* Fun fact: should occur after csutil/event.h, otherwise, gcc may report
 * missing csMouseEventHelper symbols. */
#include <wx/wx.h>
#include <wx/imaglist.h>
#include <wx/treectrl.h>
#include <wx/textctrl.h>
#include <wx/notebook.h>
#include <wx/xrc/xmlres.h>

//------------------------------------------------------------------------------

UIDialog::UIDialog (wxWindow* parent, const char* title) : wxDialog (parent, -1,
      wxString::FromUTF8 (title))
{
  sizer = new wxBoxSizer (wxVERTICAL);
  SetSizer (sizer);
  lastRowSizer = 0;
  okCancelAdded = false;
}

UIDialog::~UIDialog ()
{
  for (size_t i = 0 ; i < buttons.GetSize () ; i++)
    buttons[i]->Disconnect (wxEVT_COMMAND_BUTTON_CLICKED,
	wxCommandEventHandler (UIDialog::OnButtonClicked), 0, this);
}

void UIDialog::AddOkCancel ()
{
  if (okCancelAdded) return;
  okCancelAdded = true;
  AddRow ();
  AddSpace ();
  AddButton ("Ok");
  AddButton ("Cancel");
}

void UIDialog::AddRow ()
{
  lastRowSizer = new wxBoxSizer (wxHORIZONTAL);
  sizer->Add (lastRowSizer, 0, wxEXPAND, 5);
}

void UIDialog::AddLabel (const char* str)
{
  CS_ASSERT (lastRowSizer != 0);
  wxStaticText* label = new wxStaticText (this, wxID_ANY, wxString::FromUTF8 (str),
      wxDefaultPosition, wxDefaultSize, 0);
  label->Wrap (-1);
  lastRowSizer->Add (label, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
}

void UIDialog::AddText (const char* name)
{
  CS_ASSERT (lastRowSizer != 0);
  wxTextCtrl* text = new wxTextCtrl (this, wxID_ANY, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, 0);
  lastRowSizer->Add (text, 1, wxEXPAND | wxALL | wxALIGN_CENTER_VERTICAL, 5);
  textFields.Put (name, text);
}

void UIDialog::AddMultiText (const char* name)
{
  CS_ASSERT (lastRowSizer != 0);
  wxTextCtrl* text = new wxTextCtrl (this, wxID_ANY, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
  text->SetMinSize (wxSize (-1,50));
  lastRowSizer->Add (text, 1, wxEXPAND | wxALL | wxALIGN_CENTER_VERTICAL, 5);
  textFields.Put (name, text);
}

void UIDialog::AddChoice (const char* name, ...)
{
  CS_ASSERT (lastRowSizer != 0);
  va_list args;
  va_start (args, name);
  csDirtyAccessArray<wxString> choices;
  const char* c = va_arg (args, char*);
  while (c != (const char*)0)
  {
    choices.Push (wxString::FromUTF8 (c));
    c = va_arg (args, char*);
  }
  va_end (args);
  wxChoice* choice = new wxChoice (this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      choices.GetSize (), choices.GetArray (), 0);
  choice->SetSelection (0);
  lastRowSizer->Add (choice, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  choiceFields.Put (name, choice);
}

void UIDialog::AddButton (const char* str)
{
  CS_ASSERT (lastRowSizer != 0);
  wxButton* button = new wxButton (this, wxID_ANY, wxString::FromUTF8 (str),
      wxDefaultPosition, wxDefaultSize, 0);
  lastRowSizer->Add (button, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  button->Connect (wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler (
	UIDialog::OnButtonClicked), 0, this);
  buttons.Push (button);
}

void UIDialog::SetText (const char* name, const char* value)
{
  wxTextCtrl* text = textFields.Get (name, 0);
  if (!text)
  {
    SetChoice (name, value);
    return;
  }
  text->SetValue (wxString::FromUTF8 (value));
}

void UIDialog::SetChoice (const char* name, const char* value)
{
  wxChoice* choice = choiceFields.Get (name, 0);
  if (!choice) return;
  choice->SetStringSelection (wxString::FromUTF8 (value));
}

void UIDialog::Clear ()
{
  csHash<wxTextCtrl*,csString>::GlobalIterator itText = textFields.GetIterator ();
  while (itText.HasNext ())
  {
    csString name;
    wxTextCtrl* text = itText.Next (name);
    text->SetValue (wxT (""));
  }
  csHash<wxChoice*,csString>::GlobalIterator itCh = choiceFields.GetIterator ();
  while (itCh.HasNext ())
  {
    csString name;
    wxChoice* choice = itCh.Next (name);
    choice->SetSelection (0);
  }
}

void UIDialog::OnButtonClicked (wxCommandEvent& event)
{
  wxButton* button = static_cast<wxButton*> (event.GetEventObject ());
  csString buttonLabel = (const char*)button->GetLabel ().mb_str (wxConvUTF8);

  fieldContents.DeleteAll ();
  csHash<wxTextCtrl*,csString>::GlobalIterator itText = textFields.GetIterator ();
  while (itText.HasNext ())
  {
    csString name;
    wxTextCtrl* text = itText.Next (name);
    csString value = (const char*)text->GetValue ().mb_str (wxConvUTF8);
    fieldContents.Put (name, value);
  }
  csHash<wxChoice*,csString>::GlobalIterator itCh = choiceFields.GetIterator ();
  while (itCh.HasNext ())
  {
    csString name;
    wxChoice* choice = itCh.Next (name);
    csString value = (const char*)choice->GetStringSelection ().mb_str (wxConvUTF8);
    fieldContents.Put (name, value);
  }

  if (buttonLabel == "Ok")
  {
    EndModal (1);
    return;
  }
  else if (buttonLabel == "Cancel")
  {
    EndModal (0);
    return;
  }

  if (callback) callback->ButtonPressed (this, buttonLabel);
}

void UIDialog::AddSpace ()
{
  CS_ASSERT (lastRowSizer != 0);
  lastRowSizer->Add (0, 0, 1, wxEXPAND, 5);
}

int UIDialog::Show (UIDialogCallback* cb)
{
  callback = cb;
  AddOkCancel ();
  Layout ();
  Fit ();
  Centre (wxBOTH);
  return ShowModal ();
}


//------------------------------------------------------------------------------

UIManager::UIManager (AppAresEditWX* app, wxWindow* parent) :
  app (app), parent (parent)
{
  filereqDialog = new FileReq (parent, app->GetVFS (), "/saves");
  newprojectDialog = new NewProjectDialog (parent, this, app->GetVFS ());
  cellDialog = new CellDialog (parent, this);
  dynfactDialog = new DynfactDialog (parent, this);
  contextMenuID = ID_FirstContextItem;
}

UIManager::~UIManager ()
{
  delete filereqDialog;
  delete newprojectDialog;
  delete cellDialog;
  delete dynfactDialog;
}

void UIManager::Message (const char* description, ...)
{
  va_list args;
  va_start (args, description);
  csString msg;
  msg.FormatV (description, args);
  wxMessageBox (wxString::FromUTF8 (msg), wxT("Message"),
      wxICON_INFORMATION, parent);
  va_end (args);
}

void UIManager::Error (const char* description, ...)
{
  va_list args;
  va_start (args, description);
  csString msg;
  msg.FormatV (description, args);
  wxMessageBox (wxString::FromUTF8 (msg), wxT("Error!"),
      wxICON_ERROR, parent);
  va_end (args);
}

UIDialog* UIManager::CreateDialog (const char* title)
{
  UIDialog* dialog = new UIDialog (parent, title);
  return dialog;
}

