#include "config.h"

#include "orbit-idl2.h"

void
orbit_idl_attr_fake_ops(IDL_tree attr)
{
  IDL_tree attr_name, ident, curnode, op1, op2;
  GString *attrname;
  OIDL_Attr_Info *setme;

  g_assert(attr && IDL_NODE_TYPE(attr) == IDLN_ATTR_DCL);

  attrname = g_string_new(NULL);

  for(curnode = IDL_ATTR_DCL(attr).simple_declarations; curnode; curnode = IDL_LIST(curnode).next) {
    attr_name = IDL_LIST(curnode).data;

    g_string_sprintf(attrname, "_get_%s",
		     IDL_IDENT(attr_name).str);
    ident = IDL_ident_new(g_strdup(attrname->str));
    IDL_IDENT_TO_NS(ident) = IDL_IDENT_TO_NS(attr_name);
    op1 = IDL_op_dcl_new(0, IDL_ATTR_DCL(attr).param_type_spec, ident, NULL, NULL, NULL);
    IDL_NODE_UP(op1) = IDL_NODE_UP(attr);

    if(!IDL_ATTR_DCL(attr).f_readonly) {
      g_string_sprintf(attrname, "_set_%s",
		       IDL_IDENT(attr_name).str);
      ident = IDL_ident_new(g_strdup(attrname->str));
      IDL_IDENT_TO_NS(ident) = IDL_IDENT_TO_NS(attr_name);
      op2 = IDL_op_dcl_new(0, NULL, ident, NULL, NULL, NULL);
      IDL_NODE_UP(op2) = IDL_NODE_UP(attr);
      IDL_OP_DCL(op2).parameter_dcls = IDL_list_new(
						    IDL_param_dcl_new(IDL_PARAM_IN,
								      IDL_ATTR_DCL(attr).param_type_spec,
								      IDL_ident_new(g_strdup("value"))));
    }

    setme = g_new0(OIDL_Attr_Info, 1);
    setme->op1 = op1;
    setme->op2 = op2;
    attr_name->data = setme;
  }
}
