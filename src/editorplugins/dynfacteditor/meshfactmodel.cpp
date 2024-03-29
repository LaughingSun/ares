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

#include "meshfactmodel.h"
#include "edcommon/tools.h"
#include "propclass/dynworld.h"

using namespace Ares;


void MeshCollectionValue::UpdateChildren ()
{
  if (!dirty) return;
  dirty = false;
  ReleaseChildren ();
  iMeshFactoryList* list = engine->GetMeshFactories ();
  csStringArray names;
  for (size_t i = 0 ; i < size_t (list->GetCount ()) ; i++)
  {
    iMeshFactoryWrapper* fact = list->Get (i);
    const char* name = fact->QueryObject ()->GetName ();
    if (!dynworld->FindFactory (name))
      names.Push (name);
  }
  names.Sort ();

  for (size_t i = 0 ; i < names.GetSize () ; i++)
  {
    NewCompositeChild (VALUE_STRING, "name", names[i], VALUE_NONE);
  }
}

