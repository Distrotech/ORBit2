#include "config.h"
#include "orbit-idl2.h"

typedef void (*OIDL_Pass_Func)(IDL_tree tree);

static void oidl_pass_make_allocs(IDL_tree tree);
static void oidl_pass_make_updates(IDL_tree tree);

static struct {
  const char *name;
  OIDL_Pass_Func func;
} idl_passes[] = {
  {"Allocation", oidl_pass_make_allocs},
  {"Position updates", oidl_pass_make_updates},
  {NULL, NULL}
};

void
orbit_idl_do_passes(IDL_tree tree)
{
  int i;

  for(i = 0; idl_passes[i].name; i++)
    idl_passes[i].func(tree);
}

static void
oidl_pass_make_allocs(IDL_tree tree)
{
}

static void
oidl_pass_make_updates(IDL_tree tree)
{
}

