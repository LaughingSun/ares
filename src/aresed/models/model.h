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

#ifndef __appares_model_h
#define __appares_model_h

#include <csutil/hash.h>
#include <csutil/stringarray.h>

#include <wx/event.h>

class wxWindow;
class wxTextCtrl;
class wxCheckBox;
class wxListCtrl;
class wxStaticText;
class wxPanel;

namespace Ares
{

class Value;

/**
 * Listen to changes in a value.
 */
struct ValueChangeListener : public csRefCount
{
  /**
   * Called if the value changes.
   */
  virtual void ValueChanged (Value* value) = 0;
};

/**
 * All possible value types.
 */
enum ValueType
{
  VALUE_STRING,
  VALUE_LONG,
  VALUE_BOOL,
  VALUE_FLOAT,

  /**
   * A collection of values. In this situation the
   * children have no specific name and can only
   * be accessed by iterating over them.
   */
  VALUE_COLLECTION,

  /**
   * A collection of values. In this situation the
   * children have to be accessed by name.
   */
  VALUE_COMPOSITE,
};


/**
 * An interface representing a domain value.
 */
class Value : public csRefCount
{
protected:
  csRefArray<ValueChangeListener> listeners;

  void FireValueChanged ()
  {
    for (size_t i = 0 ; i < listeners.GetSize () ; i++)
      listeners[i]->ValueChanged (this);
  }

public:
  /**
   * Get the type of the value.
   */
  virtual ValueType GetType () const { return VALUE_STRING; }

  /**
   * Get the name of this value (used if this value is a child of
   * a VALUE_COMPOSITE).
   */
  virtual const char* GetName () const { return 0; }

  /**
   * Get the value if the type is VALUE_STRING.
   * Returns 0 if the value does not represent a string.
   */
  virtual const char* GetStringValue () { return 0; }

  /**
   * Set the value as a string. Does nothing if the value does
   * not represent a string.
   */
  virtual void SetStringValue (const char* str) { }

  /**
   * Get the value if the type is VALUE_LONG.
   * Returns 0 if the value does not represent a long.
   */
  virtual long GetLongValue () { return 0; }

  /**
   * Set the value as a long. Does nothing if the value does
   * not represent a long.
   */
  virtual void SetLongValue (long v) { }

  /**
   * Get the value if the type is VALUE_BOOL.
   * Returns false if the value does not represent a bool.
   */
  virtual bool GetBoolValue () { return false; }

  /**
   * Set the value as a bool. Does nothing if the value does
   * not represent a bool.
   */
  virtual void SetBoolValue (bool v) { }

  /**
   * Get the value if the type is VALUE_FLOAT.
   * Returns 0.0f if the value does not represent a float.
   */
  virtual float GetFloatValue () { return 0.0f; }

  /**
   * Set the value as a float. Does nothing if the value does
   * not represent a float.
   */
  virtual void SetFloatValue (float v) { }

  /**
   * Add a listener for value changes. Note that a value does not
   * have to support this.
   */
  virtual void AddValueChangeListener (ValueChangeListener* listener)
  {
    listeners.Push (listener);
  }
  
  /**
   * Remove a listener.
   */
  virtual void RemoveValueChangeListener (ValueChangeListener* listener)
  {
    listeners.Delete (listener);
  }

  // -----------------------------------------------------------------------------

  /**
   * If the type of this value is VALUE_COLLECTION or VALUE_COMPOSITE,
   * then you can get the children using ResetIterator(), HasChildren(),
   * and NextChild().
   */
  virtual void ResetIterator () { }

  /**
   * Check if there are still children to process.
   */
  virtual bool HasChildren () { return false; }

  /**
   * Get the next child.
   */
  virtual Value* NextChild () { return 0; }

  /**
   * If the type of this value is VALUE_COMPOSITE or VALUE_COLLECTION then you
   * can sometimes get a child by index here.
   */
  virtual Value* GetChild (size_t idx) { return 0; }

  /**
   * If the type of this value is VALUE_COMPOSITE then you can get
   * a child by name here.
   */
  virtual Value* GetChild (const char* name) { return 0; }

  // -----------------------------------------------------------------------------

  /**
   * Delete a child value from this value. Returns false if this value could not
   * be deleted for some reason.
   */
  virtual bool DeleteValue (Value* child) { return false; }

  /**
   * Add a child value to this value. Returns false if this value could
   * not be added for some reason.
   */
  virtual bool AddValue (Value* child) { return false; }

  /**
   * Update a child. Returns false if this child could not be updated.
   * The default implementation just calls DeleteChild() first and then
   * AddChild() with the new child.
   */
  virtual bool UpdateValue (Value* oldChild, Value* child)
  {
    if (!DeleteValue (oldChild)) return false;
    if (!AddValue (child))
    {
      AddValue (oldChild);
      return false;
    }
    return true;
  }
};

class BufferValueChangeListener;

/**
 * This value can be used as the detail value for children of a
 * collection. It listens to the current selection of the list and synchronizes
 * the value corresponding to that selection with this buffered value.
 * It can also buffer values (as caused by changes in the UI components)
 * which can then later be applied to the real value in the collection.
 * A buffered value only works on a collection value containing composite
 * values with itself having primitive children.
 */
class BufferedValue : public wxEvtHandler, public Value
{
  friend class BufferValueChangeListener;

private:
  wxListCtrl* listCtrl;
  csRef<Value> collectionValue;

  /// The selected item in the list.
  long selection;
  /// The value from the collection we are showing.
  Value* currentValue;

  csRef<BufferValueChangeListener> changeListener;

  // Called when the value from the collection changes.
  void ValueChanged (Value* value);

  void OnSelectionChange (wxCommandEvent& event);
  void UpdateToSelection ();

public:
  BufferedValue (wxListCtrl* listCtrl, Value* collectionValue);
  virtual ~BufferedValue ();

  virtual ValueType GetType () const { return VALUE_COMPOSITE; }
  virtual const char* GetName () const { return 0; }
  virtual const char* GetStringValue () { return 0; }
  virtual void SetStringValue (const char* str) { }
  virtual long GetLongValue () { return 0; }
  virtual void SetLongValue (long v) { }
  virtual bool GetBoolValue () { return false; }
  virtual void SetBoolValue (bool v) { }
  virtual float GetFloatValue () { return 0.0f; }
  virtual void SetFloatValue (float v) { }
};



class ViewValueChangeListener;

/**
 * A view. This class keeps track of the bindings between
 * models and WX controls for a given logical unit (frame, dialog, panel, ...).
 */
class View : public wxEvtHandler, public csRefCount
{
  friend class ViewValueChangeListener;

private:
  wxWindow* parent;

  // All the bindings.
  struct Binding
  {
    csRef<Value> value;
    wxWindow* component;
    wxEventType eventType;
    Binding () : component (0), eventType (wxEVT_NULL) { }
  };
  typedef csHash<Binding,csPtrKey<wxWindow> > ComponentToBinding;
  typedef csHash<Binding,csPtrKey<Value> > ValueToBinding;
  ComponentToBinding bindingsByComponent;
  ValueToBinding bindingsByValue;

  // Listeners.
  csRef<ViewValueChangeListener> changeListener;

  // Keep track of list headings.
  struct ListHeading
  {
    csStringArray heading;
    csStringArray names;
  };
  typedef csHash<ListHeading,csPtrKey<wxListCtrl> > ListToHeading;
  ListToHeading listToHeading;

  /**
   * Construct a value (composite or single) to a string array as
   * compatible for a given list.
   */
  csStringArray ConstructListRow (const ListHeading& lh, Value* value);

  /// Called by components when they change. Will update the corresponding Value.
  void OnComponentChanged (wxCommandEvent& event);
  /// Called by values when they change. Will update the corresponding component.
  void ValueChanged (Value* value);

  /**
   * Recursively find a component by name. Also supports the '_' notation
   * in the name of the components.
   */
  wxWindow* FindComponentByName (wxWindow* container, const char* name);

public:
  View (wxWindow* parent);
  ~View ();

  /**
   * Bind a value to a WX component. This function will try to find the
   * best match between the given value and the component.
   * Can fail (return false) under the following conditions:
   * - Component has an unsupported type.
   * - Value type is not compatible with component type.
   */
  bool Bind (Value* value, wxWindow* component);

  /**
   * Bind a value to a WX component. This function will try to find the
   * best match between the given value and the component.
   * Can fail (return false) under the following conditions:
   * - Component could not be found by this name.
   * - Component has an unsupported type.
   * - Value type is not compatible with component type.
   */
  bool Bind (Value* value, const char* compName);

  /**
   * Bind a value directly to a text control.
   * Can fail (return false) under the following conditions:
   * - Value type is not compatible with component type.
   */
  bool Bind (Value* value, wxTextCtrl* component);

  /**
   * Bind a value directly to a panel. This only works with
   * values of type VALUE_COMPOSITE. Note that it will try
   * to find GUI components on the panel by scanning the names.
   * If a name of such a component contains an underscore ('_')
   * then it will only look at the part of the name before the
   * underscore.
   * The result of binding a value with a panel is that the
   * children of the value will be bound with corresponding
   * interface components on the panel.
   * Can fail (return false) under the following conditions:
   * - Value type is not compatible with component type.
   */
  bool Bind (Value* value, wxPanel* component);

  /**
   * Bind a value directly to a list control. This only works with
   * values of type VALUE_COLLECTION.
   * Can fail (return false) under the following conditions:
   * - Value type is not compatible with component type.
   */
  bool Bind (Value* value, wxListCtrl* component);

  /**
   * Define a heading for a list control. For every column in the list
   * there is a logical name which will be used as the name of the
   * child value in case the list is populated with composite values.
   * Every column also has a heading which will be used as a heading
   * on the list component.
   * @param heading is a comma separated string with the heading for the list.
   * @param names is a comma separated string with the names of the children.
   * @return false on failure (component could not be found or is not a list).
   */
  bool DefineHeading (const char* listName, const char* heading,
      const char* names);
  bool DefineHeading (wxListCtrl* listCtrl, const char* heading,
      const char* names);
};

} // namespace Ares

#endif // __appares_model_h
