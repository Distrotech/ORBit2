/**************************************************************************

    orbit-idl-backends.c (Backend directory & loading)

    Copyright (C) 1999 Elliot Lee

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id$

***************************************************************************/

#include "config.h"
#include "orbit-idl2.h"
#include "orbit-idl-c-backend.h"
#include <dirent.h>
#if 0
#include <gmodule.h>
#endif
#include <string.h>

static OIDL_Backend_Info orbit_idl_builtin_backends[] = {
  {"c", &orbit_idl_output_c},
  {NULL, NULL}
};

OIDL_Backend_Info *orbit_idl_backend_for_lang(const char *lang,const char* backend_dir)
{
  int i;
#if 0
  int ret;
  char *fname, *ctmp;
  GModule *gmod;
  OIDL_Backend_Info *retval = NULL;
#endif

  for(i = 0; orbit_idl_builtin_backends[i].name; i++) {
    if(!strcmp(lang, orbit_idl_builtin_backends[i].name))
      return &orbit_idl_builtin_backends[i];
  }

  return NULL;
#if 0
  g_return_val_if_fail(g_module_supported(), NULL);

  ctmp = alloca(sizeof("orbit-idl--backend") + strlen(lang));
  sprintf(ctmp, "orbit-idl-%s-backend", lang);
  fname = g_module_build_path(backend_dir, ctmp);
  g_assert(fname);
  gmod = g_module_open(fname, G_MODULE_BIND_LAZY);

  if(!gmod) {
	g_warning("Module load failed: %s", g_module_error());
	return NULL;
  }

  g_module_make_resident(gmod);

  ret = g_module_symbol(gmod,
			"orbit_idl_backend",
			(gpointer *)&retval);
  if(!ret) {
	g_warning("Symbol lookup failed: %s", g_module_error());
	return NULL;
  }

  return retval;
#endif
}
