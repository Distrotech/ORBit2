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

#define INDENT_INCREMENT_1 2
#define INDENT_INCREMENT_2 4

static void do_indent(int level) {
  int i;
  for(i = 0; i < level; i++) g_print(" ");
}

static const char * const nodenames[] = {
  "DATUM",
  "LOOP",
  "SWITCH",
  "COMPLEX",
  "UPDATE",
  "CONST",
  "SET",
  "ALLOCATE",
  NULL
};

void
orbit_idl_print_node(IDL_tree node, int indent_level)
{
  IDL_tree curnode;
  char *s;

  do_indent(indent_level);

  if(node == NULL) {
    g_print("(null)\n");
    return;
  }

  g_print("[%d] ", IDL_NODE_REFS(node));

  switch(IDL_NODE_TYPE(node)) {

  case IDLN_NONE:
    g_print("NONE\n");
    break;

  case IDLN_LIST:
    g_print("LIST:\n");
    for(curnode = node; curnode;
	curnode = IDL_LIST(curnode).next) {
      orbit_idl_print_node(IDL_LIST(curnode).data, indent_level + INDENT_INCREMENT_1);
    }
    break;

  case IDLN_GENTREE:
    g_print("GENTREE:\n");
#if 0
    /* Changed in libIDL.  But don't need it here anyway. */
    orbit_idl_print_node(IDL_GENTREE(node).data, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1);
    g_print("children:\n");
    orbit_idl_print_node(IDL_GENTREE(node).children, indent_level + INDENT_INCREMENT_2);
#endif
    break;

  case IDLN_INTEGER:
    g_print("INTEGER: %" IDL_LL "d\n", IDL_INTEGER(node).value);
    break;

  case IDLN_STRING:
    g_print("STRING: %s\n", IDL_STRING(node).value);
    break;

  case IDLN_WIDE_STRING:
    g_print("WIDE STRING: %ls\n", IDL_WIDE_STRING(node).value);
    break;

  case IDLN_CHAR:
    g_print("CHAR: %s\n", IDL_CHAR(node).value);
    break;

  case IDLN_WIDE_CHAR:
    g_print("WIDE CHAR: %ls\n", IDL_WIDE_CHAR(node).value);
    break;

  case IDLN_FIXED:
    g_print("FIXED: %s\n", IDL_FIXED(node).value);
    break;

  case IDLN_FLOAT:
    g_print("FLOAT: %f\n", IDL_FLOAT(node).value);
    break;

  case IDLN_BOOLEAN:
    g_print("BOOLEAN: %s\n", (IDL_BOOLEAN(node).value)?"True":"False");
    break;

  case IDLN_IDENT:
    s = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(node), "_", 0);
    g_print("IDENT: %s NSQ: %s RID: \"%s\"\n",
	    IDL_IDENT(node).str, s,
	    IDL_IDENT_REPO_ID(node) ? IDL_IDENT_REPO_ID(node) : "");
    g_free(s);
    break;

  case IDLN_TYPE_DCL:
    g_print("TYPE DCL:\n");
    orbit_idl_print_node(IDL_TYPE_DCL(node).type_spec, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("decls:\n");
    orbit_idl_print_node(IDL_TYPE_DCL(node).dcls, indent_level + INDENT_INCREMENT_2);
    break;

  case IDLN_CONST_DCL:
    g_print("CONST DCL:\n");
    orbit_idl_print_node(IDL_CONST_DCL(node).const_type, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("ident:\n");
    orbit_idl_print_node(IDL_CONST_DCL(node).ident, indent_level + INDENT_INCREMENT_2);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("const_exp:\n");
    orbit_idl_print_node(IDL_CONST_DCL(node).const_exp, indent_level + INDENT_INCREMENT_2);
    break;

  case IDLN_EXCEPT_DCL:
    g_print("EXCEPT DCL:\n");
    orbit_idl_print_node(IDL_EXCEPT_DCL(node).ident, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("members:\n");
    orbit_idl_print_node(IDL_EXCEPT_DCL(node).members, indent_level + INDENT_INCREMENT_2);
    break;

  case IDLN_ATTR_DCL:
    g_print("ATTR_DCL (%s):\n", (IDL_ATTR_DCL(node).f_readonly)?"readonly":"rw");
    orbit_idl_print_node(IDL_ATTR_DCL(node).param_type_spec, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("simple_declarations:\n");
    orbit_idl_print_node(IDL_ATTR_DCL(node).simple_declarations, indent_level + INDENT_INCREMENT_2);
    break;

  case IDLN_OP_DCL:
    g_print("OP DCL (%s):\n", (IDL_OP_DCL(node).f_oneway)?"oneway":"normal");
    orbit_idl_print_node(IDL_OP_DCL(node).ident, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("op_type_spec:\n");
    orbit_idl_print_node(IDL_OP_DCL(node).op_type_spec, indent_level + INDENT_INCREMENT_2);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("parameter_dcls:\n");
    orbit_idl_print_node(IDL_OP_DCL(node).parameter_dcls, indent_level + INDENT_INCREMENT_2);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("raises_expr:\n");
    orbit_idl_print_node(IDL_OP_DCL(node).raises_expr, indent_level + INDENT_INCREMENT_2);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("context_expr:\n");
    orbit_idl_print_node(IDL_OP_DCL(node).context_expr, indent_level + INDENT_INCREMENT_2);
    break;

  case IDLN_PARAM_DCL:
    g_print("PARAM DCL: ");
    switch(IDL_PARAM_DCL(node).attr) {
    case IDL_PARAM_IN: g_print("(in)\n"); break;
    case IDL_PARAM_OUT: g_print("(out)\n"); break;
    case IDL_PARAM_INOUT: g_print("(inout)\n"); break;
    }
    orbit_idl_print_node(IDL_PARAM_DCL(node).param_type_spec, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("simple_declarator:\n");
    orbit_idl_print_node(IDL_PARAM_DCL(node).simple_declarator, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_FORWARD_DCL:
    g_print("FORWARD DCL:\n");
    orbit_idl_print_node(IDL_FORWARD_DCL(node).ident, indent_level + INDENT_INCREMENT_1);
    break;
  case IDLN_INTERFACE:
    g_print("INTERFACE:\n");
    orbit_idl_print_node(IDL_INTERFACE(node).ident, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("inheritance_spec:\n");
    orbit_idl_print_node(IDL_INTERFACE(node).inheritance_spec, indent_level + INDENT_INCREMENT_2);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("body:\n");
    orbit_idl_print_node(IDL_INTERFACE(node).body, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_MODULE:
    g_print("MODULE:\n");
    orbit_idl_print_node(IDL_MODULE(node).ident, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("definition_list:\n");
    orbit_idl_print_node(IDL_MODULE(node).definition_list, indent_level + INDENT_INCREMENT_2);
    break;

  case IDLN_TYPE_INTEGER:
    if(!IDL_TYPE_INTEGER(node).f_signed) g_print("TYPE unsigned ");
    switch(IDL_TYPE_INTEGER(node).f_type) {
    case IDL_INTEGER_TYPE_SHORT: g_print("short\n"); break;
    case IDL_INTEGER_TYPE_LONG: g_print("long\n"); break;
    case IDL_INTEGER_TYPE_LONGLONG: g_print("long long\n"); break;
    }
    break;
  case IDLN_TYPE_FLOAT:
    switch(IDL_TYPE_FLOAT(node).f_type) {
    case IDL_FLOAT_TYPE_FLOAT: g_print("TYPE float\n"); break;
    case IDL_FLOAT_TYPE_DOUBLE: g_print("TYPE double\n"); break;
    case IDL_FLOAT_TYPE_LONGDOUBLE: g_print("TYPE long double\n"); break;
    }
    break;
  case IDLN_TYPE_FIXED:
    g_print("TYPE fixed:\n");
    orbit_idl_print_node(IDL_TYPE_FIXED(node).positive_int_const, indent_level + INDENT_INCREMENT_1);
    orbit_idl_print_node(IDL_TYPE_FIXED(node).integer_lit, indent_level + INDENT_INCREMENT_1);
    break;
  case IDLN_TYPE_STRING:
    g_print("TYPE string:\n");
    orbit_idl_print_node(IDL_TYPE_STRING(node).positive_int_const, indent_level + INDENT_INCREMENT_1);
    break;
  case IDLN_TYPE_WIDE_STRING:
    g_print("TYPE wide string:\n");
    orbit_idl_print_node(IDL_TYPE_WIDE_STRING(node).positive_int_const, indent_level + INDENT_INCREMENT_1);
    break;
  case IDLN_TYPE_ENUM:
    g_print("TYPE enum:\n");
    orbit_idl_print_node(IDL_TYPE_ENUM(node).ident, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("enumerator_list:\n");
    orbit_idl_print_node(IDL_TYPE_ENUM(node).enumerator_list, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_TYPE_ARRAY:
    g_print("TYPE array:\n");
    orbit_idl_print_node(IDL_TYPE_ARRAY(node).ident, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("size_list:\n");
    orbit_idl_print_node(IDL_TYPE_ARRAY(node).size_list, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_TYPE_SEQUENCE:
    g_print("TYPE sequence:\n");
    orbit_idl_print_node(IDL_TYPE_SEQUENCE(node).simple_type_spec, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("positive_int_const:\n");
    orbit_idl_print_node(IDL_TYPE_SEQUENCE(node).positive_int_const, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_TYPE_STRUCT:
    g_print("TYPE struct:\n");
    orbit_idl_print_node(IDL_TYPE_STRUCT(node).ident, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("member_list:\n");
    orbit_idl_print_node(IDL_TYPE_STRUCT(node).member_list, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_TYPE_UNION:
    g_print("TYPE union:\n");
    orbit_idl_print_node(IDL_TYPE_UNION(node).ident, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("switch_type_spec:\n");
    orbit_idl_print_node(IDL_TYPE_UNION(node).switch_type_spec, indent_level + INDENT_INCREMENT_2);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("switch_body:\n");
    orbit_idl_print_node(IDL_TYPE_UNION(node).switch_body, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_MEMBER:
    g_print("MEMBER:\n");
    orbit_idl_print_node(IDL_MEMBER(node).type_spec, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("dcls:\n");
    orbit_idl_print_node(IDL_MEMBER(node).dcls, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_CASE_STMT:
    g_print("CASE_STMT:\n");
    orbit_idl_print_node(IDL_CASE_STMT(node).labels, indent_level + INDENT_INCREMENT_1);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("element_spec:\n");
    orbit_idl_print_node(IDL_CASE_STMT(node).element_spec, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_BINOP:
    g_print("BINOP ");
    switch(IDL_BINOP(node).op) {
    case IDL_BINOP_OR: g_print("or:\n"); break;
    case IDL_BINOP_XOR: g_print("xor:\n"); break;
    case IDL_BINOP_AND: g_print("and:\n"); break;
    case IDL_BINOP_SHR: g_print("shr:\n"); break;
    case IDL_BINOP_SHL: g_print("shl:\n"); break;
    case IDL_BINOP_ADD: g_print("add:\n"); break;
    case IDL_BINOP_SUB: g_print("sub:\n"); break;
    case IDL_BINOP_MULT: g_print("mult:\n"); break;
    case IDL_BINOP_DIV: g_print("div:\n"); break;
    case IDL_BINOP_MOD: g_print("mod:\n"); break;
    }
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("left:\n");
    orbit_idl_print_node(IDL_BINOP(node).left, indent_level + INDENT_INCREMENT_2);
    do_indent(indent_level + INDENT_INCREMENT_1); g_print("right:\n");
    orbit_idl_print_node(IDL_BINOP(node).right, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_UNARYOP:
    g_print("UNARYOP ");
    switch(IDL_UNARYOP(node).op) {
    case IDL_UNARYOP_PLUS: g_print("plus:\n"); break;
    case IDL_UNARYOP_MINUS: g_print("minus:\n"); break;
    case IDL_UNARYOP_COMPLEMENT: g_print("complement:\n"); break;
    }
    orbit_idl_print_node(IDL_UNARYOP(node).operand, indent_level + INDENT_INCREMENT_1);
    break;
  case IDLN_TYPE_CHAR:
    g_print("TYPE char\n");
    break;
  case IDLN_TYPE_WIDE_CHAR:
    g_print("TYPE wide char\n");
    break;
  case IDLN_TYPE_BOOLEAN:
    g_print("TYPE boolean\n");
    break;
  case IDLN_TYPE_OCTET:
    g_print("TYPE octet\n");
    break;
  case IDLN_TYPE_OBJECT:
    g_print("TYPE object\n");
    break;
  case IDLN_TYPE_ANY:
    g_print("TYPE any\n");
    break;
  case IDLN_TYPE_TYPECODE:
    g_print("TYPE TypeCode\n");
    break;
  default:
    g_print("unhandled %d\n", IDL_NODE_TYPE(node));
  }
}

void
oidl_marshal_tree_dump(IDL_tree tree, int indent_level)
{
  IDL_tree node;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_LIST:
    for(node = tree; node; node = IDL_LIST(node).next) {
      oidl_marshal_tree_dump(IDL_LIST(node).data, indent_level);
    }
    break;
  case IDLN_MODULE:
    do_indent(indent_level);
    g_print("Module %s:\n", IDL_IDENT(IDL_MODULE(tree).ident).str);
    oidl_marshal_tree_dump(IDL_MODULE(tree).definition_list, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_INTERFACE:
    do_indent(indent_level);
    g_print("Interface %s:\n", IDL_IDENT(IDL_INTERFACE(tree).ident).str);
    oidl_marshal_tree_dump(IDL_INTERFACE(tree).body, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_OP_DCL:
    do_indent(indent_level);
    g_print("Operation %s:\n", IDL_IDENT(IDL_OP_DCL(tree).ident).str);
    oidl_marshal_node_dump(((OIDL_Op_Info *)tree->data)->in, indent_level + INDENT_INCREMENT_2);
    oidl_marshal_node_dump(((OIDL_Op_Info *)tree->data)->out, indent_level + INDENT_INCREMENT_2);
    break;
  case IDLN_ATTR_DCL:
    oidl_marshal_tree_dump(((OIDL_Attr_Info *)tree->data)->op1, indent_level);
    if(((OIDL_Attr_Info *)tree->data)->op2)
      oidl_marshal_tree_dump(((OIDL_Attr_Info *)tree->data)->op2, indent_level);
    break;
  default:
    break;
  }
}

void
oidl_marshal_node_dump(OIDL_Marshal_Node *tree, int indent_level)
{
  do_indent(indent_level);

  if(!tree) { g_print("Nil\n"); return; }

  if(tree->name)
    g_print("\"%s\" (\"%s\") ", tree->name, oidl_marshal_node_fqn(tree));
  else {
    char *ctmp;

    ctmp = oidl_marshal_node_fqn(tree);
    if(ctmp)
      g_print("(\"%s\") ", ctmp);
  }

  g_print("(%s %p): [", nodenames[tree->type], tree);
  if(tree->flags & MN_POINTER_VAR)
    g_print("POINTER_VAR ");
  if(tree->flags & MN_INOUT)
    g_print("INOUT ");
  if(tree->flags & MN_NSROOT)
    g_print("NSROOT ");
  if(tree->flags & MN_NEED_TMPVAR)
    g_print("NEED_TMPVAR ");
  if(tree->flags & MN_NOMARSHAL)
    g_print("NOMARSHAL ");
  if(tree->flags & MN_ISSEQ)
    g_print("ISSEQ ");
  if(tree->flags & MN_ISSTRING)
    g_print("ISSTRING ");
  g_print("]\n");

  switch(tree->type) {
  case MARSHAL_LOOP:
    do_indent(indent_level + INDENT_INCREMENT_1);
    g_print("loop_var:\n");
    oidl_marshal_node_dump(tree->u.loop_info.loop_var, indent_level + INDENT_INCREMENT_2);
    do_indent(indent_level + INDENT_INCREMENT_1);
    g_print("length_var:\n");
    oidl_marshal_node_dump(tree->u.loop_info.length_var, indent_level + INDENT_INCREMENT_2);
    do_indent(indent_level + INDENT_INCREMENT_1);
    g_print("contents:\n");
    oidl_marshal_node_dump(tree->u.loop_info.contents, indent_level + INDENT_INCREMENT_2);
    break;
  case MARSHAL_DATUM:
    do_indent(indent_level + INDENT_INCREMENT_1);
    g_print("datum_size: %d\n", tree->u.datum_info.datum_size);
    break;
  case MARSHAL_SET:
    {
      GSList *ltmp;
      do_indent(indent_level + INDENT_INCREMENT_1);
      g_print("subnodes:\n");
      for(ltmp = tree->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp)) {
	oidl_marshal_node_dump(ltmp->data, indent_level + INDENT_INCREMENT_2);
	g_print("\n");
      }
    }
    break;
  case MARSHAL_ALLOCATE:
    break;
  case MARSHAL_SWITCH:
    do_indent(indent_level + INDENT_INCREMENT_1);
    g_print("discrim:\n");
    oidl_marshal_node_dump(tree->u.switch_info.discrim, indent_level + INDENT_INCREMENT_2);
    do_indent(indent_level + INDENT_INCREMENT_1);
    g_print("contents:\n");
    oidl_marshal_node_dump(tree->u.switch_info.contents, indent_level + INDENT_INCREMENT_2);
    break;
  case MARSHAL_COMPLEX:
    break;
  case MARSHAL_UPDATE:
    do_indent(indent_level + INDENT_INCREMENT_1);
    g_print("update amount:\n");
    oidl_marshal_node_dump(tree->u.update_info.amount, indent_level + INDENT_INCREMENT_2);
    break;
  case MARSHAL_CONST:
    do_indent(indent_level + INDENT_INCREMENT_1);
    g_print("amount: %d\n", tree->u.const_info.amount);
    break;
  default:
    g_warning("Don't know any details about %s nodes", nodenames[tree->type]);
    break;
  }
} 

IDL_tree
orbit_idl_get_array_type(IDL_tree tree)
{
  IDL_tree parent;

  parent = IDL_get_parent_node(tree, IDLN_ANY, NULL);
  g_assert(IDL_NODE_TYPE(parent) == IDLN_LIST);

  parent = IDL_get_parent_node(parent, IDLN_ANY, NULL);
  switch(IDL_NODE_TYPE(parent)) {
  case IDLN_MEMBER:
    return IDL_MEMBER(parent).type_spec;
    break;
  case IDLN_TYPE_DCL:
    return IDL_TYPE_DCL(parent).type_spec;
    break;
  default:
    g_assert(IDL_NODE_TYPE(parent) == IDLN_MEMBER
	       || IDL_NODE_TYPE(parent) == IDLN_TYPE_DCL);
  }

  return NULL;
}

char *
orbit_idl_member_get_name(IDL_tree tree)
{
  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_TYPE_ARRAY:
    return orbit_idl_member_get_name(IDL_TYPE_ARRAY(tree).ident);
    break;
  case IDLN_IDENT:
    return IDL_IDENT(tree).str;
    break;
  default:
    g_error("Don't know how to get member name of a %s", IDL_tree_type_names[IDL_NODE_TYPE(tree)]);
  }

  return NULL;
}

void
orbit_idl_node_foreach(OIDL_Marshal_Node *node, GFunc func, gpointer user_data)
{
  if(!node) return;

  func(node, user_data);

  switch(node->type) {
  case MARSHAL_LOOP:
    orbit_idl_node_foreach(node->u.loop_info.loop_var, func, user_data);
    orbit_idl_node_foreach(node->u.loop_info.length_var, func, user_data);
    orbit_idl_node_foreach(node->u.loop_info.contents, func, user_data);
    break;
  case MARSHAL_SWITCH:
    orbit_idl_node_foreach(node->u.switch_info.discrim, func, user_data);
    orbit_idl_node_foreach(node->u.switch_info.contents, func, user_data);
    break;
  case MARSHAL_UPDATE:
    orbit_idl_node_foreach(node->u.update_info.amount, func, user_data);
    break;
  case MARSHAL_SET:
    {
      GSList *ltmp;

      for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp))
	orbit_idl_node_foreach((OIDL_Marshal_Node *)ltmp->data, func, user_data);
    }
    break;
  default:
    break;
  }
}

static void
IDL_tree_traverse_helper(IDL_tree p, GFunc f,
			 gconstpointer func_data,
			 GHashTable *visited_nodes)
{
	IDL_tree curitem;

	if(g_hash_table_lookup(visited_nodes, p))
		return;

	g_hash_table_insert(visited_nodes, p, ((gpointer)1));

	for(curitem = IDL_INTERFACE(p).inheritance_spec; curitem;
	    curitem = IDL_LIST(curitem).next) {
		IDL_tree_traverse_helper(IDL_get_parent_node(IDL_LIST(curitem).data, IDLN_INTERFACE, NULL), f, func_data, visited_nodes);
	}

	f(p, (gpointer)func_data);
}

void
IDL_tree_traverse_parents(IDL_tree p,
			  GFunc f,
			  gconstpointer func_data)
{
	GHashTable *visited_nodes = g_hash_table_new(NULL, g_direct_equal);

	if(!(p && f))
		return;

	if(IDL_NODE_TYPE(p) != IDLN_INTERFACE)
		p = IDL_get_parent_node(p, IDLN_INTERFACE, NULL);

	if(!p)
		return;

	IDL_tree_traverse_helper(p, f, func_data, visited_nodes);

	g_hash_table_destroy(visited_nodes);
}
