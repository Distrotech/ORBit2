/**************************************************************************

    orbit-idl-driver.c (Dispatch parsed tree to various backends)

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

#include "orbit-idl2.h"
#include <string.h>

/****************
  orbit_idl_to_backend:
     Input: IDL filename, and any arguments to be passed to CPP
     Output: Number of IDL files (1 or 0) that were successfully processed
     Does: Reads in 'filename' & parses it into a tree, using libIDL.
	   Calls the backend producer.
****************/
int
orbit_idl_to_backend(const char *filename, OIDL_Run_Info *rinfo)
{
  OIDL_Backend_Info *binfo;
  IDL_ns namespace;
  IDL_tree tree;
  int errcode;
  char *basename, *ctmp;
  OIDL_Output_Tree otree;

  binfo = orbit_idl_backend_for_lang(rinfo->output_language);

  g_return_val_if_fail(binfo && binfo->op_output, 0);

  errcode = IDL_parse_filename(filename, rinfo->cpp_args, NULL,
			       &tree, &namespace,
			       IDLF_TYPECODES|IDLF_CODEFRAGS,
			       rinfo->debug_level);

  if(IDL_SUCCESS != errcode) {
    if(errcode == -1)
      g_warning("Parse of %s failed: %s", filename, g_strerror(errno));

    return 0;
  }

  basename = alloca(strlen(filename) + 1);
  strcpy(basename, filename);
  ctmp = strrchr(basename, '.');
  if(ctmp) *ctmp = '\0';

  otree.tree = tree;
  otree.pass_info = NULL;

  binfo->op_output(&otree, rinfo);

  return 1;
}
