#include "config.h"
#include "orbit-idl2.h"

typedef void (*OIDL_Pass_Func)(IDL_tree tree, gpointer data);

static void oidl_pass_make_allocs(IDL_tree tree);
static void oidl_pass_make_updates(IDL_tree tree);
static void oidl_pass_tmpvars(IDL_tree tree);
static void oidl_pass_run_for_ops(IDL_tree tree, GFunc func);

static struct {
  const char *name;
  OIDL_Pass_Func func;
  gpointer data;
} idl_passes[] = {
  {"Allocation", (OIDL_Pass_Func)oidl_pass_make_allocs, NULL},
  {"Position updates", (OIDL_Pass_Func)oidl_pass_make_updates, NULL},
  {"Variable assignment", (OIDL_Pass_Func)oidl_pass_tmpvars, NULL},
  {"Set collapsing", (OIDL_Pass_Func)oidl_pass_run_for_ops, orbit_idl_collapse_sets},
  {NULL, NULL}
};

void
orbit_idl_do_passes(IDL_tree tree)
{
  int i;

#if defined(DEBUG) && 0
  oidl_marshal_tree_dump(tree, 0);
#endif

  for(i = 0; idl_passes[i].name; i++) {

    idl_passes[i].func(tree, idl_passes[i].data);
  }

#if defined(DEBUG) && 0
  oidl_marshal_tree_dump(tree, 0);
#endif
}

static void
oidl_pass_make_allocs(IDL_tree tree)
{
}

static void
oidl_pass_make_updates(IDL_tree tree)
{
}

static void
oidl_pass_tmpvars(IDL_tree tree)
{
  IDL_tree node;

  if(!tree) return;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_LIST:
    for(node = tree; node; node = IDL_LIST(node).next) {
      oidl_pass_tmpvars(IDL_LIST(node).data);
    }
    break;
  case IDLN_MODULE:
    oidl_pass_tmpvars(IDL_MODULE(tree).definition_list);
    break;
  case IDLN_INTERFACE:
    oidl_pass_tmpvars(IDL_INTERFACE(tree).body);
    break;
  case IDLN_OP_DCL:
    {
      int counter = 0;

      orbit_idl_tmpvars_assign(((OIDL_Op_Info *)tree->data)->in, &counter);
      orbit_idl_tmpvars_assign(((OIDL_Op_Info *)tree->data)->out, &counter);
    }
    break;
  case IDLN_ATTR_DCL:
    {
      IDL_tree curnode, attr_name;

      for(curnode = IDL_ATTR_DCL(tree).simple_declarations; curnode; curnode = IDL_LIST(curnode).next) {
	attr_name = IDL_LIST(curnode).data;

	oidl_pass_tmpvars(((OIDL_Attr_Info *)attr_name->data)->op1);
	if(((OIDL_Attr_Info *)attr_name->data)->op2)
	  oidl_pass_tmpvars(((OIDL_Attr_Info *)attr_name->data)->op2);
      }
    }
    break;
  default:
    break;
  }
}

static void
oidl_pass_run_for_ops(IDL_tree tree, GFunc func)
{
  IDL_tree node;

  if(!tree) return;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_LIST:
    for(node = tree; node; node = IDL_LIST(node).next) {
      oidl_pass_run_for_ops(IDL_LIST(node).data, func);
    }
    break;
  case IDLN_MODULE:
    oidl_pass_run_for_ops(IDL_MODULE(tree).definition_list, func);
    break;
  case IDLN_INTERFACE:
    oidl_pass_run_for_ops(IDL_INTERFACE(tree).body, func);
    break;
  case IDLN_OP_DCL:
    {
      OIDL_Op_Info *oi = (OIDL_Op_Info *)tree->data;

      if(oi->in)
	func(oi->in, NULL);

      if(oi->out)
	func(oi->out, NULL);
    }
    break;
  case IDLN_ATTR_DCL:
    {
      IDL_tree curnode, attr_name;

      for(curnode = IDL_ATTR_DCL(tree).simple_declarations; curnode; curnode = IDL_LIST(curnode).next) {
	attr_name = IDL_LIST(curnode).data;

	oidl_pass_run_for_ops(((OIDL_Attr_Info *)attr_name->data)->op1, func);
	if(((OIDL_Attr_Info *)attr_name->data)->op2)
	  oidl_pass_run_for_ops(((OIDL_Attr_Info *)attr_name->data)->op2, func);
      }
    }
    break;
  default:
    break;
  }  
}
