#include "orbit-idl2.h"
#include "backends/c/orbit-idl-c-backend.h"

static OIDL_Backend_Info orbit_idl_builtin_backends[] = {
  {"c", /* &orbit_idl_output_c */},
  {NULL, NULL}
};

OIDL_Backend_Info *orbit_idl_backend_for_lang(const char *lang)
{
  int i;

  for(i = 0; orbit_idl_builtin_backends[i].name; i++) {
    if(!strcmp(lang, orbit_idl_builtin_backends[i].name))
      return &orbit_idl_builtin_backends[i];
  }

  /* XXX TODO - dlopen backend modules */

  return NULL;
}
