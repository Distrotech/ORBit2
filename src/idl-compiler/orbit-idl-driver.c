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

#include "config.h"
#include "orbit-idl2.h"
#include <string.h>

static void orbit_idl_tree_populate(IDL_tree tree, IDL_ns ns, OIDL_Output_Tree *rinfo, gboolean attrs_only);

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
  char *fbasename, *ctmp;
  OIDL_Output_Tree otree;

  binfo = orbit_idl_backend_for_lang(rinfo->output_language,rinfo->backend_directory);

  g_return_val_if_fail(binfo && binfo->op_output, 0);

  errcode = IDL_parse_filename(filename, rinfo->cpp_args, NULL,
			       &tree, &namespace,
			       (rinfo->show_cpp_errors?IDLF_SHOW_CPP_ERRORS:0)
			       |IDLF_TYPECODES
			       |IDLF_SRCFILES
			       |(rinfo->is_pidl?IDLF_XPIDL:0)
			       |(rinfo->onlytop
			         ?(IDLF_INHIBIT_INCLUDES):0)
			       |IDLF_CODEFRAGS,
			       rinfo->idl_warn_level);
  rinfo->ns = namespace;

  if(rinfo->debug_level > 3)
    orbit_idl_print_node(tree, 0);

  if(IDL_SUCCESS != errcode) {
    if(errcode == -1)
      g_warning("Parse of %s failed: %s", filename, g_strerror(errno));

    return 0;
  }

  fbasename = g_alloca(strlen(filename) + 1);
  strcpy(fbasename, filename);
  ctmp = strrchr(fbasename, '.');
  if(ctmp) *ctmp = '\0';

  otree.tree = tree;

  orbit_idl_tree_populate(otree.tree, namespace, &otree, TRUE);
  otree.ctxt = oidl_marshal_context_new (tree);
  if(rinfo->debug_level > 2)
    {
      g_print("                        ----- Marshal context dump: -----\n");
      oidl_marshal_context_dump(otree.ctxt);
    }

  orbit_idl_tree_populate(otree.tree, namespace, &otree, FALSE);
  orbit_idl_do_passes(otree.tree, rinfo);

  binfo->op_output(&otree, rinfo);
  oidl_marshal_context_free(otree.ctxt);

  return 1;
}

static void orbit_idl_op_populate(IDL_tree tree, OIDL_Output_Tree *rinfo)
{
  OIDL_Op_Info *setme;

  setme = g_new0(OIDL_Op_Info, 1);

  setme->in_stubs = orbit_idl_marshal_populate_in(tree, FALSE, rinfo->ctxt);
  setme->out_stubs = orbit_idl_marshal_populate_out(tree, FALSE, rinfo->ctxt);
  setme->in_skels = orbit_idl_marshal_populate_in(tree, TRUE, rinfo->ctxt);
  setme->out_skels = orbit_idl_marshal_populate_out(tree, TRUE, rinfo->ctxt);
  setme->counter = 0;

  tree->data = setme;
}

static void orbit_idl_except_populate(IDL_tree tree, OIDL_Output_Tree *rinfo)
{
  OIDL_Except_Info *setme;

  setme = g_new0(OIDL_Except_Info, 1);

  setme->marshal = orbit_idl_marshal_populate_except_marshal(tree, rinfo->ctxt);
  setme->demarshal = orbit_idl_marshal_populate_except_demarshal(tree, rinfo->ctxt);

  tree->data = setme;
}

static void
orbit_idl_tree_populate(IDL_tree tree, IDL_ns ns, OIDL_Output_Tree *rinfo, gboolean attrs_only)
{
  IDL_tree node;

  if(!tree) return;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_LIST:
    for(node = tree; node; node = IDL_LIST(node).next) {
      orbit_idl_tree_populate(IDL_LIST(node).data, ns, rinfo, attrs_only);
    }
    break;
  case IDLN_MODULE:
    orbit_idl_tree_populate(IDL_MODULE(tree).definition_list, ns, rinfo, attrs_only);
    break;
  case IDLN_INTERFACE:
    orbit_idl_tree_populate(IDL_INTERFACE(tree).body, ns, rinfo, attrs_only);
    break;
  case IDLN_OP_DCL:
    if(!attrs_only)
      orbit_idl_op_populate(tree, rinfo);
    break;
  case IDLN_EXCEPT_DCL:
    if(!attrs_only)
      orbit_idl_except_populate(tree, rinfo);
    break;
  case IDLN_ATTR_DCL:
    {
      IDL_tree curnode, attr_name;

      if(attrs_only)
	orbit_idl_attr_fake_ops(tree, ns);
      for(curnode = IDL_ATTR_DCL(tree).simple_declarations; curnode; curnode = IDL_LIST(curnode).next) {
	attr_name = IDL_LIST(curnode).data;

	orbit_idl_tree_populate(((OIDL_Attr_Info *)attr_name->data)->op1, ns, rinfo, attrs_only);
	if(((OIDL_Attr_Info *)attr_name->data)->op2)
	  orbit_idl_tree_populate(((OIDL_Attr_Info *)attr_name->data)->op2, ns, rinfo, attrs_only);
      }
    }
    break;
  default:
    break;
  }
}
