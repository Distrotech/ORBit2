#include "config.h"

#include "orbit-idl2.h"

static OIDL_Marshal_Node *
oidl_marshal_node_new(OIDL_Marshal_Node *parent, OIDL_Marshal_Node_Type type, const char *name)
{
  OIDL_Marshal_Node *retval;

  retval = g_new0(OIDL_Marshal_Node, 1);

  retval->up = parent;
  retval->type = type;
  retval->name = (char *)name;

  return retval;
}

static gboolean
oidl_marshal_node_is_arrayel(OIDL_Marshal_Node *node, OIDL_Marshal_Node **loopvar_ret)
{
  OIDL_Marshal_Node *lv = NULL, *curnode;
  gboolean retval = FALSE;

  g_assert(node);

  for(curnode = node; curnode && !curnode->name; curnode = curnode->up) /* */;

  g_return_val_if_fail(curnode, FALSE);

  for(curnode = curnode->up; curnode && !curnode->name && curnode->type != MARSHAL_LOOP; curnode = curnode->up) /* */;

  if(curnode->type == MARSHAL_LOOP)
     retval = TRUE;

  if(loopvar_ret)
    *loopvar_ret = lv;
  
  return retval;
}

/* If we are trying to produce C code to access a specific variable, then we need to be able get the C-compilable
   string that refers to that var */
static char *
oidl_marshal_node_fqn(OIDL_Marshal_Node *node)
{
  GString *tmpstr;
  char *retval;
  OIDL_Marshal_Node *curnode, *tnode;

  tmpstr = g_string_new(NULL);

  retval = tmpstr->str;

  curnode = node;
  while(curnode) {
    if(curnode->name) {
      if(oidl_marshal_node_is_arrayel(curnode, &tnode)) {
	char *ctmp;
	ctmp = oidl_marshal_node_fqn(tnode);
	g_string_prepend_c(tmpstr, ']');
	g_string_prepend(tmpstr, ctmp);
	g_string_prepend_c(tmpstr, '[');
      }

      if(curnode != node) {
	if(curnode->flags & MN_POINTER_VAR)
	  g_string_prepend(tmpstr, "->");
	else
	  g_string_prepend_c(tmpstr, '.');
      }

      g_string_prepend(tmpstr, node->name);
    }

    curnode = curnode->up;
  }

  if(node->flags & MN_POINTER_VAR) {
    g_string_prepend(tmpstr, "*(");
    g_string_append_c(tmpstr, ')');
  }

  g_string_free(tmpstr, FALSE);

  return retval;
}

static OIDL_Marshal_Node *
marshal_populate_in(IDL_tree tree)
{
  switch(IDL_NODE_TYPE(tree)) {
  default:
    break;
  }

  return NULL;
}

OIDL_Marshal_Node *
orbit_idl_marshal_populate_in(IDL_tree tree)
{
  OIDL_Marshal_Node *retval;
  IDL_tree curitem, curnode, curparam;

  g_assert(IDL_NODE_TYPE(tree) == IDLN_OP_DCL);

  g_return_val_if_fail(IDL_OP_DCL(tree).parameter_dcls, NULL);

  retval = oidl_marshal_node_new(NULL, MARSHAL_SET, NULL);

  for(curitem = IDL_OP_DCL(tree).parameter_dcls; curitem; curitem = IDL_LIST(curitem).next) {
    curparam = IDL_LIST(curnode).data;
    if(IDL_PARAM_DCL(curparam).attr == IDL_PARAM_IN)
      continue;

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes,
						 marshal_populate_in(curparam));
  }

  return retval;
}

static OIDL_Marshal_Node *
marshal_populate_out(IDL_tree tree)
{
  return NULL;
}

OIDL_Marshal_Node *
orbit_idl_marshal_populate_out(IDL_tree tree)
{
  OIDL_Marshal_Node *retval, *rvnode;
  IDL_tree curitem, curnode, curparam;

  g_assert(IDL_NODE_TYPE(tree) == IDLN_OP_DCL);

  retval = oidl_marshal_node_new(NULL, MARSHAL_SET, NULL);

  if(IDL_OP_DCL(tree).op_type_spec) {
    rvnode = marshal_populate_out(IDL_OP_DCL(tree).op_type_spec);
    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, rvnode);
    g_assert(! rvnode->name);

    rvnode->name = ORBIT_RETVAL_VAR_NAME;
  }

  for(curitem = IDL_OP_DCL(tree).parameter_dcls; curitem; curitem = IDL_LIST(curitem).next) {
    curparam = IDL_LIST(curnode).data;
    if(IDL_PARAM_DCL(curparam).attr == IDL_PARAM_IN)
      continue;

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes,
						 marshal_populate_out(curparam));
  }

  return retval;
}

gboolean
orbit_idl_marshal_endian_dependant_p(OIDL_Marshal_Node *node)
{
  GSList *ltmp;
  gboolean btmp = FALSE;

  switch(node->type) {
  case MARSHAL_DATUM:
    btmp = (node->u.datum_info.datum_size > 1);
    break;
  case MARSHAL_SET:
    for(ltmp = node->u.set_info.subnodes; !btmp && ltmp; ltmp = g_slist_next(ltmp)) {
      btmp |= orbit_idl_marshal_endian_dependant_p(ltmp->data);
    }
    break;
  case MARSHAL_LOOP:
    btmp =
      orbit_idl_marshal_endian_dependant_p(node->u.loop_info.loop_var)
      || orbit_idl_marshal_endian_dependant_p(node->u.loop_info.length_var)
      || orbit_idl_marshal_endian_dependant_p(node->u.loop_info.contents);
    break;
  case MARSHAL_SWITCH:
    btmp =
      orbit_idl_marshal_endian_dependant_p(node->u.switch_info.discrim)
      || orbit_idl_marshal_endian_dependant_p(node->u.switch_info.contents);
    break;
  default:
    btmp = FALSE;
    break;
  }

  return btmp;
}
