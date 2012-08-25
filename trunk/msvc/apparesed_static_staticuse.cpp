// This file is automatically generated.
#include "cssysdef.h"
#include "csutil/scf.h"

// Put static linking stuff into own section.
// The idea is that this allows the section to be swapped out but not
// swapped in again b/c something else in it was needed.
#if !defined(CS_DEBUG) && defined(CS_COMPILER_MSVC)
#pragma const_seg(".CSmetai")
#pragma comment(linker, "/section:.CSmetai,r")
#pragma code_seg(".CSmeta")
#pragma comment(linker, "/section:.CSmeta,er")
#pragma comment(linker, "/merge:.CSmetai=.CSmeta")
#endif
struct _static_use { _static_use (); };
_static_use::_static_use () {}
SCF_USE_STATIC_PLUGIN(curvemode)
SCF_USE_STATIC_PLUGIN(dynfacteditor)
SCF_USE_STATIC_PLUGIN(entitymode)
SCF_USE_STATIC_PLUGIN(foliagemode)
SCF_USE_STATIC_PLUGIN(helpframe)
SCF_USE_STATIC_PLUGIN(mainmode)
SCF_USE_STATIC_PLUGIN(playmode)
SCF_USE_STATIC_PLUGIN(roommode)

