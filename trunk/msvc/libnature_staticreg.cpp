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

namespace csStaticPluginInit
{
static char const metainfo_nature[] =
"<?xml version=\"1.0\"?>"
"<!-- nature.csplugin -->"
"<plugin>"
"  <scf>"
"    <classes>"
"      <class>"
"        <name>utility.nature</name>"
"        <implementation>Nature</implementation>"
"        <description>Nature Plugin</description>"
"      </class>"
"    </classes>"
"  </scf>"
"</plugin>"
;
  #ifndef Nature_FACTORY_REGISTER_DEFINED 
  #define Nature_FACTORY_REGISTER_DEFINED 
    SCF_DEFINE_FACTORY_FUNC_REGISTRATION(Nature) 
  #endif

class nature
{
SCF_REGISTER_STATIC_LIBRARY(nature,metainfo_nature)
  #ifndef Nature_FACTORY_REGISTERED 
  #define Nature_FACTORY_REGISTERED 
    Nature_StaticInit Nature_static_init__; 
  #endif
public:
 nature();
};
nature::nature() {}

}
