#include "orbit-idl3.h"
#include "orbit-idl-marshal.h"
#include <string.h>

static void orbit_idl_tree_populate(IDL_tree tree, IDL_ns ns);

int orbit_idl_run(const char *filename, OIDL_Run_Info *rinfo)
{
  IDL_ns namespace;
  IDL_tree tree;
  int errcode;

  errcode = IDL_parse_filename(filename, rinfo->cpp_args, NULL,
			       &tree, &namespace,
			       IDLF_TYPECODES|IDLF_CODEFRAGS,
			       IDL_WARNINGMAX);
  if(rinfo->debug_level > 1)
    orbit_idl_print_node(tree, 0);

  if(IDL_SUCCESS != errcode) {
    if(errcode == -1)
      g_warning("Parse of %s failed: %s", filename, g_strerror(errno));

    return errcode;
  }

  orbit_idl_tree_populate(tree, namespace);

  rinfo->marshal_ctx = oidl_marshal_context_new(tree);
  oidl_marshal_context_dump(rinfo->marshal_ctx);

  orbit_idl_output_c(tree, rinfo);

  return 0;
}

static void
orbit_idl_tree_populate(IDL_tree tree, IDL_ns ns)
{
  IDL_tree node;

  if(!tree) return;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_LIST:
    for(node = tree; node; node = IDL_LIST(node).next)
      orbit_idl_tree_populate(IDL_LIST(node).data, ns);
    break;
  case IDLN_MODULE:
    orbit_idl_tree_populate(IDL_MODULE(tree).definition_list, ns);
    break;
  case IDLN_INTERFACE:
    orbit_idl_tree_populate(IDL_INTERFACE(tree).body, ns);
    break;
  case IDLN_ATTR_DCL:
    {
      IDL_tree curnode, attr_name;

      orbit_idl_attr_fake_ops(tree, ns);

      for(curnode = IDL_ATTR_DCL(tree).simple_declarations; curnode; curnode = IDL_LIST(curnode).next) {
	attr_name = IDL_LIST(curnode).data;

	orbit_idl_tree_populate(((OIDL_Attr_Info *)attr_name->data)->op1, ns);
	if(((OIDL_Attr_Info *)attr_name->data)->op2)
	  orbit_idl_tree_populate(((OIDL_Attr_Info *)attr_name->data)->op2, ns);
      }
    }
    break;
  default:
    break;
  }
}
