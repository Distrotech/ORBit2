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

    if(curnode->flags & MN_NSROOT)
      break;

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
marshal_populate_in(IDL_tree tree, OIDL_Marshal_Node *parent)
{
  OIDL_Marshal_Node *retval = NULL;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_INTEGER:
    retval = oidl_marshal_node_new(parent, MARSHAL_CONST, NULL);
    retval->u.const_info.amount = IDL_INTEGER(tree).value;
    break;
  case IDLN_TYPE_FLOAT:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL);
    switch(IDL_TYPE_FLOAT(tree).f_type) {
    case IDL_FLOAT_TYPE_FLOAT:
      retval->u.datum_info.datum_size = sizeof(CORBA_float);
      break;
    case IDL_FLOAT_TYPE_DOUBLE:
      retval->u.datum_info.datum_size = sizeof(CORBA_double);
      break;
    case IDL_FLOAT_TYPE_LONGDOUBLE:
      retval->u.datum_info.datum_size = sizeof(CORBA_long_double);
      break;
    default:
      g_assert(0);
      break;
    }
    break;
  case IDLN_TYPE_INTEGER:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL);
    switch(IDL_TYPE_INTEGER(tree).f_type) {
    case IDL_INTEGER_TYPE_SHORT:
      retval->u.datum_info.datum_size = sizeof(CORBA_short);
      break;
    case IDL_INTEGER_TYPE_LONG:
      retval->u.datum_info.datum_size = sizeof(CORBA_long);
      break;
    case IDL_INTEGER_TYPE_LONGLONG:
      retval->u.datum_info.datum_size = sizeof(CORBA_long_long);
      break;
    default:
      g_assert(0);
      break;
    }
    break;
  case IDLN_TYPE_STRING:
  case IDLN_TYPE_WIDE_STRING:
    retval = oidl_marshal_node_new(parent, MARSHAL_LOOP, NULL);
    retval->u.loop_info.is_string = 1;
    retval->u.loop_info.length_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL);
    retval->u.loop_info.length_var->u.datum_info.datum_size = sizeof(CORBA_long);
    retval->u.loop_info.length_var->flags |= MN_NEED_TMPVAR;
    break;
  case IDLN_TYPE_ENUM:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL);
    retval->u.datum_info.datum_size = sizeof(CORBA_long);
    break;
  case IDLN_TYPE_ARRAY:
    {
      OIDL_Marshal_Node *cursub = parent, *newsub;
      IDL_tree curlevel;

      for(curlevel = IDL_TYPE_ARRAY(tree).size_list; curlevel; curlevel = IDL_LIST(curlevel).next) {
	newsub = oidl_marshal_node_new(cursub, MARSHAL_LOOP, NULL);

	cursub = newsub;

	if(!retval)
	  retval = cursub;
	else
	  cursub->u.loop_info.contents = newsub;

	cursub->u.loop_info.loop_var = oidl_marshal_node_new(cursub, MARSHAL_DATUM, NULL);
	cursub->u.loop_info.loop_var->u.datum_info.datum_size = sizeof(CORBA_long);
	cursub->u.loop_info.loop_var->flags |= MN_NOMARSHAL|MN_NEED_TMPVAR;

	cursub->u.loop_info.length_var = marshal_populate_in(IDL_LIST(curlevel).data, cursub);
	cursub->u.loop_info.length_var->flags |= MN_NOMARSHAL;
      }

      cursub->u.loop_info.contents = marshal_populate_in(orbit_idl_get_array_type(tree), cursub);
    }
    break;
  case IDLN_TYPE_SEQUENCE:
    retval = oidl_marshal_node_new(parent, MARSHAL_LOOP, NULL);

    retval->u.loop_info.loop_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL);
    retval->u.loop_info.loop_var->u.datum_info.datum_size = sizeof(CORBA_unsigned_long);
    retval->u.loop_info.loop_var->flags |= MN_NOMARSHAL|MN_NEED_TMPVAR;

    retval->u.loop_info.length_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL);
    retval->u.loop_info.length_var->u.datum_info.datum_size = sizeof(CORBA_unsigned_long);
    retval->u.loop_info.length_var->name = "_length";

    retval->u.loop_info.contents = marshal_populate_in(IDL_TYPE_SEQUENCE(tree).simple_type_spec, retval);
    break;
  case IDLN_TYPE_STRUCT:
    {
      IDL_tree curitem;
      retval = oidl_marshal_node_new(parent, MARSHAL_SET, NULL);

      for(curitem = IDL_TYPE_STRUCT(tree).member_list; curitem; curitem = IDL_LIST(curitem).next) {
	retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes,
						     marshal_populate_in(IDL_LIST(curitem).data, retval));
      }
    }
    break;
  case IDLN_MEMBER:
    {
      IDL_tree curitem, curnode;
      OIDL_Marshal_Node *tnode;

      retval = oidl_marshal_node_new(parent, MARSHAL_SET, NULL);
      for(curitem = IDL_MEMBER(tree).dcls; curitem; curitem = IDL_LIST(curitem).next) {
	curnode = IDL_LIST(curitem).data;

	if(IDL_NODE_TYPE(curnode) == IDLN_IDENT) {
	  tnode = marshal_populate_in(IDL_MEMBER(tree).type_spec, retval);
	} else if(IDL_NODE_TYPE(curnode) == IDLN_TYPE_ARRAY) {
	  tnode = marshal_populate_in(curnode, retval);
	} else
	  g_error("A member that is not an ident nor an array?");

	tnode->name = orbit_idl_member_get_name(curnode);

	retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, tnode);
      }
    }
    break;
  case IDLN_TYPE_UNION:
    retval = oidl_marshal_node_new(parent, MARSHAL_SWITCH, NULL);
    break;
  case IDLN_IDENT:
    retval = marshal_populate_in(IDL_get_parent_node(tree, IDLN_ANY, NULL), parent);
    break;
  case IDLN_TYPE_DCL:
    break;
  case IDLN_PARAM_DCL:
    retval = marshal_populate_in(IDL_PARAM_DCL(tree).param_type_spec, parent);
    g_assert(retval);
    g_assert(!retval->name);
    retval->name = IDL_IDENT(IDL_PARAM_DCL(tree).simple_declarator).str;
    break;
  default:
    break;
  }

  g_return_val_if_fail(retval, retval);

  return retval;
}

OIDL_Marshal_Node *
orbit_idl_marshal_populate_in(IDL_tree tree)
{
  OIDL_Marshal_Node *retval;
  IDL_tree curitem, curparam;

  g_assert(IDL_NODE_TYPE(tree) == IDLN_OP_DCL);

  g_return_val_if_fail(IDL_OP_DCL(tree).parameter_dcls, NULL);

  retval = oidl_marshal_node_new(NULL, MARSHAL_SET, NULL);

  for(curitem = IDL_OP_DCL(tree).parameter_dcls; curitem; curitem = IDL_LIST(curitem).next) {
    curparam = IDL_LIST(curitem).data;

    if(IDL_PARAM_DCL(curparam).attr == IDL_PARAM_OUT)
      continue;

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes,
						 marshal_populate_in(curparam, retval));
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
  IDL_tree curitem, curparam;

  g_assert(IDL_NODE_TYPE(tree) == IDLN_OP_DCL);

  retval = oidl_marshal_node_new(NULL, MARSHAL_SET, NULL);

  if(IDL_OP_DCL(tree).op_type_spec) {
    rvnode = marshal_populate_out(IDL_OP_DCL(tree).op_type_spec);
    if(!rvnode) goto out1;

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, rvnode);

    g_assert(! rvnode->name);

    rvnode->name = ORBIT_RETVAL_VAR_NAME;
  }
 out1:

  for(curitem = IDL_OP_DCL(tree).parameter_dcls; curitem; curitem = IDL_LIST(curitem).next) {
    curparam = IDL_LIST(curitem).data;
    if(IDL_PARAM_DCL(curparam).attr == IDL_PARAM_IN)
      continue;

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes,
						 marshal_populate_out(curparam));
  }

  return retval;
}

static void
orbit_idl_assign_tmpvar_names(OIDL_Marshal_Node *node, int *counter)
{
  if(!(node->flags & MN_NEED_TMPVAR))
    return;

  node->name = g_strdup_printf("v%d", *counter);

  (*counter)++;
}

void
orbit_idl_tmpvars_assign(OIDL_Marshal_Node *top, int *counter)
{
  orbit_idl_node_foreach(top, (GFunc)orbit_idl_assign_tmpvar_names, counter);
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
