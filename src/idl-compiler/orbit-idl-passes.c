#include "config.h"
#include "orbit-idl2.h"

typedef void (*OIDL_Pass_Func)(IDL_tree tree);

static void oidl_pass_make_allocs(IDL_tree tree);
static void oidl_pass_make_updates(IDL_tree tree);
static void oidl_pass_tmpvars(IDL_tree tree);

static struct {
  const char *name;
  OIDL_Pass_Func func;
} idl_passes[] = {
  {"Allocation", oidl_pass_make_allocs},
  {"Position updates", oidl_pass_make_updates},
  {"Variable assignment", oidl_pass_tmpvars},
  {NULL, NULL}
};

void
orbit_idl_do_passes(IDL_tree tree)
{
  int i;

#ifdef DEBUG
  oidl_marshal_tree_dump(tree, 0);
#endif

  for(i = 0; idl_passes[i].name; i++) {

    idl_passes[i].func(tree);
  }

#ifdef DEBUG
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
    oidl_pass_tmpvars(((OIDL_Attr_Info *)tree->data)->op1);
    if(((OIDL_Attr_Info *)tree->data)->op2)
      oidl_pass_tmpvars(((OIDL_Attr_Info *)tree->data)->op2);
    break;
  default:
    break;
  }
}
