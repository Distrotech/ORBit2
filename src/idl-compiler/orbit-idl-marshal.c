#include "orbit-idl2.h"
#include "orbit-idl-marshal.h"

static const char *marshal_methods[] = {
  "inline", "func", "any"
};

static OIDL_Marshal_Method
pick_preferred_method(OIDL_Marshal_Method method)
{
  if(method & MARSHAL_INLINE)
    return MARSHAL_INLINE;
  else if(method & MARSHAL_FUNC)
    return MARSHAL_FUNC;
  else
    return MARSHAL_ANY;
}

static void
decide_tmi(gpointer key, gpointer value, gpointer data)
{
  OIDL_Type_Marshal_Info *tmi = value;
  int i, n;

  if(tmi->use_count > 1)
    {
      tmi->mtype &= ~MARSHAL_INLINE;
      tmi->dmtype &= ~MARSHAL_INLINE;
    }
  if(tmi->size > INLINE_SIZE_THRESHOLD)
    {
      tmi->mtype &= ~(MARSHAL_INLINE|MARSHAL_FUNC);
      tmi->dmtype &= ~(MARSHAL_INLINE|MARSHAL_FUNC);
    }
  for(i = n = 0; i < MARSHAL_NUM; i++)
    if((tmi->mtype & (1<<i))
       || (tmi->dmtype & (1<<i)))
      n++;

  if(n > 1)
    {
      tmi->mtype = pick_preferred_method(tmi->mtype);
      tmi->dmtype = pick_preferred_method(tmi->dmtype);
    }
}

static void
oidl_marshal_context_decide(OIDL_Marshal_Context *ctxt)
{
  g_hash_table_foreach(ctxt->type_marshal_info, decide_tmi, NULL);
}

static OIDL_Marshal_Method
oidl_marshal_methods_avail(void)
{
  return MARSHAL_FUNC|MARSHAL_INLINE|MARSHAL_ANY;
}

static gint
oidl_marshal_type_info(OIDL_Marshal_Context *ctxt, IDL_tree node, gint count_add, gboolean named_type)
{
  gboolean add_it = FALSE;
  OIDL_Marshal_Method mtype, dmtype;
  OIDL_Type_Marshal_Info *tmi = NULL;
  gint add_size = 0;
  gint retval = 1;
  IDL_tree realnode = node;

  if(!node)
    return 0;

  if(node->data)
    return 1;

  realnode = orbit_cbe_get_typespec(node);
  if(IDL_NODE_TYPE(node) == IDLN_TYPE_DCL)
    node = realnode;

  switch(IDL_NODE_TYPE(node))
    {
    case IDLN_IDENT:
      if(IDL_NODE_TYPE(IDL_NODE_UP(node)) == IDLN_LIST)
	node = IDL_NODE_UP(node);
      return oidl_marshal_type_info(ctxt, IDL_NODE_UP(node), count_add, TRUE);
      break;

    case IDLN_TYPE_SEQUENCE:
    case IDLN_TYPE_UNION:
    case IDLN_TYPE_STRUCT:
    case IDLN_TYPE_ARRAY:
      add_it = TRUE;
      break;
    default:
      break;
    }

  mtype = dmtype = oidl_marshal_methods_avail();

  if(add_it)
    {
      if(IDL_tree_is_recursive(node, NULL))
	{
	  count_add++; /* We have to count the function calling itself */
	  mtype &= ~MARSHAL_INLINE;
	  dmtype &= ~MARSHAL_INLINE;
	}

      tmi = g_hash_table_lookup(ctxt->type_marshal_info, realnode);
      if(!tmi)
	{
	  tmi = g_new0(OIDL_Type_Marshal_Info, 1);
	  tmi->mtype = mtype;
	  tmi->dmtype = dmtype;
	  tmi->tree = node;
	  g_hash_table_insert(ctxt->type_marshal_info, realnode, tmi);
	}

      tmi->use_count += count_add;
    }

  node->data = GUINT_TO_POINTER(0x1);

  switch(IDL_NODE_TYPE(node))
    {
    case IDLN_LIST:
      {
	IDL_tree curnode;
	for(curnode = node; curnode;
	    curnode = IDL_LIST(curnode).next)
	  add_size += oidl_marshal_type_info(ctxt, IDL_LIST(curnode).data, count_add, named_type);
	retval = add_size;
      }
      break;
    case IDLN_TYPE_UNION:
      add_size = 1; /* Overhead for the switch() */
      add_size += oidl_marshal_type_info(ctxt, IDL_TYPE_UNION(node).switch_type_spec, count_add, FALSE);
      add_size += oidl_marshal_type_info(ctxt, IDL_TYPE_UNION(node).switch_body, count_add, FALSE);
      break;
    case IDLN_TYPE_STRUCT:
      add_size += oidl_marshal_type_info(ctxt, IDL_TYPE_STRUCT(node).member_list, count_add, FALSE);
      break;
    case IDLN_EXCEPT_DCL:
      add_size += oidl_marshal_type_info(ctxt, IDL_EXCEPT_DCL(node).members, count_add, FALSE);
      break;
    case IDLN_TYPE_SEQUENCE:
      add_size = 1; /* Overhead for the loop */
      add_size += oidl_marshal_type_info(ctxt, IDL_TYPE_SEQUENCE(node).simple_type_spec, count_add, FALSE);
      break;
    case IDLN_CASE_STMT:
      add_size += oidl_marshal_type_info(ctxt, IDL_CASE_STMT(node).element_spec, count_add, FALSE);
      retval = add_size;
      break;
    case IDLN_TYPE_DCL:
      add_size += oidl_marshal_type_info(ctxt, IDL_TYPE_DCL(node).type_spec, count_add*IDL_list_length(IDL_TYPE_DCL(node).dcls), FALSE);
      retval = add_size;
      break;
    case IDLN_MEMBER:
      add_size += oidl_marshal_type_info(ctxt, IDL_MEMBER(node).type_spec, count_add*IDL_list_length(IDL_MEMBER(node).dcls), FALSE) * IDL_list_length(IDL_MEMBER(node).dcls);
      retval = add_size;
      break;
    case IDLN_TYPE_ARRAY:
      add_size += oidl_marshal_type_info(ctxt, IDL_TYPE_DCL(IDL_NODE_UP(IDL_NODE_UP(node))).type_spec, count_add, FALSE);
      add_size += 1; /* Overhead for the loop */
      retval = add_size;
      break;
    case IDLN_TYPE_STRING:
    case IDLN_TYPE_WIDE_STRING:
      retval = add_size = 2;
      break;
    default:
      break;
    }

  node->data = NULL;

  if(add_it)
    tmi->size = add_size;

  return retval;
}

static void
oidl_marshal_context_populate(OIDL_Marshal_Context *ctxt, IDL_tree node)
{
  IDL_tree curnode;

  if(!node)
    return;

  switch(IDL_NODE_TYPE(node))
    {
    case IDLN_LIST:
      for(curnode = node; curnode;
	  curnode = IDL_LIST(curnode).next)
	oidl_marshal_context_populate(ctxt, IDL_LIST(curnode).data);
      break;

    case IDLN_ATTR_DCL:
      oidl_marshal_context_populate(ctxt, IDL_ATTR_DCL(node).simple_declarations);
      for(curnode = IDL_ATTR_DCL(node).simple_declarations; curnode; curnode = IDL_LIST(curnode).next) {
	IDL_tree attr_name;

	attr_name = IDL_LIST(curnode).data;

	oidl_marshal_context_populate(ctxt, ((OIDL_Attr_Info *)attr_name->data)->op1);
	if(((OIDL_Attr_Info *)attr_name->data)->op2)
	  oidl_marshal_context_populate(ctxt, ((OIDL_Attr_Info *)attr_name->data)->op2);
      }
      break;

    case IDLN_OP_DCL:
      if(IDL_OP_DCL(node).op_type_spec)
	oidl_marshal_type_info(ctxt, IDL_OP_DCL(node).op_type_spec, 1, FALSE);
      for(curnode = IDL_OP_DCL(node).parameter_dcls; curnode; curnode = IDL_LIST(curnode).next)
	{
	  IDL_tree param = IDL_LIST(curnode).data;

	  oidl_marshal_type_info(ctxt, IDL_PARAM_DCL(param).param_type_spec,
				 (IDL_PARAM_DCL(param).attr == IDL_PARAM_INOUT)?2:1, FALSE);
	}
      for(curnode = IDL_OP_DCL(node).raises_expr; curnode; curnode = IDL_LIST(curnode).next)
	{
	  IDL_tree exc = IDL_LIST(curnode).data;

	  oidl_marshal_type_info(ctxt, exc, 1, FALSE);
	}
      break;

    case IDLN_INTERFACE:
      oidl_marshal_context_populate(ctxt, IDL_INTERFACE(node).body);
      break;

    case IDLN_MODULE:
      oidl_marshal_context_populate(ctxt, IDL_MODULE(node).definition_list);
      break;

    default:
      break;
    }
}

static IDL_tree
idl_tree_normalize(IDL_tree t1)
{
  switch(IDL_NODE_TYPE(t1))
    {
    case IDLN_IDENT:
      if(IDL_NODE_TYPE(IDL_NODE_UP(t1)) == IDLN_LIST)
	t1 = IDL_NODE_UP(t1);
      return idl_tree_normalize(IDL_NODE_UP(t1));
      break;

    case IDLN_TYPE_DCL:
      return idl_tree_normalize(IDL_TYPE_DCL(t1).type_spec);
      break;

    default:
      break;
    }

  return t1;
}

static guint
idl_tree_hash(gconstpointer k1)
{
  IDL_tree t1 = (gpointer)k1;

  t1 = idl_tree_normalize(t1);

  return IDL_NODE_TYPE(t1);
}

static gint
idl_tree_equal(gconstpointer k1, gconstpointer k2)
{
  IDL_tree t1 = (gpointer)k1, t2 = (gpointer)k2;

  if(k1 == k2)
    return TRUE;

  t1 = idl_tree_normalize(t1);
  t2 = idl_tree_normalize(t2);

  if(IDL_NODE_TYPE(t1) != IDL_NODE_TYPE(t2))
    return FALSE;

  switch(IDL_NODE_TYPE(t1))
    {
    case IDLN_TYPE_SEQUENCE:
      return idl_tree_equal(IDL_TYPE_SEQUENCE(t1).simple_type_spec, IDL_TYPE_SEQUENCE(t2).simple_type_spec);
      break;

    case IDLN_TYPE_OCTET:
    case IDLN_TYPE_STRING:
    case IDLN_TYPE_BOOLEAN:
    case IDLN_TYPE_OBJECT:
    case IDLN_INTERFACE:
    case IDLN_TYPE_TYPECODE:
    case IDLN_TYPE_CHAR:
    case IDLN_TYPE_WIDE_CHAR:
    case IDLN_TYPE_WIDE_STRING:
      return TRUE;
      break;

    case IDLN_TYPE_FLOAT:
      break;

    case IDLN_TYPE_INTEGER:
      break;

    default:
      break;
    }

  return (k1 == k2);
}

OIDL_Marshal_Context *
oidl_marshal_context_new(IDL_tree tree)
{
  OIDL_Marshal_Context *retval;

  retval = g_new0(OIDL_Marshal_Context, 1);

  retval->type_marshal_info = g_hash_table_new(idl_tree_hash, idl_tree_equal);

  oidl_marshal_context_populate(retval, tree);
  oidl_marshal_context_decide(retval);

  return retval;
}

static void
free_tmi(gpointer key, gpointer value, gpointer data)
{
  g_free(value);
}

void
oidl_marshal_context_free(OIDL_Marshal_Context *ctxt)
{
  g_hash_table_foreach(ctxt->type_marshal_info, free_tmi, NULL);
}

static void
print_mm(OIDL_Marshal_Method mm)
{
  int i;

  for(i = 0; i < MARSHAL_NUM; i++)
    {
      if(mm & ((1 << i) - 1))
	g_print("|");

      if(mm & (1 << i))
	g_print("%s", marshal_methods[i]);
    }
  g_print("%s", mm?" ":". ");
}

static void
dump_tmi(gpointer key, gpointer value, gpointer data)
{
  IDL_tree tree = key, ident = NULL;
  OIDL_Type_Marshal_Info *tmi = value;

  switch(IDL_NODE_TYPE(tree))
    {
    case IDLN_TYPE_STRUCT:
      ident = IDL_TYPE_STRUCT(tree).ident;
      break;
    case IDLN_TYPE_UNION:
      ident = IDL_TYPE_UNION(tree).ident;
      break;
    case IDLN_EXCEPT_DCL:
      ident = IDL_EXCEPT_DCL(tree).ident;
      break;
    case IDLN_TYPE_ARRAY:
      ident = IDL_TYPE_ARRAY(tree).ident;
      break;
    default:
      break;
    }
  if(!ident && IDL_NODE_TYPE(tree) == IDLN_TYPE_SEQUENCE)
    {
      g_print("sequence-of-");
      ident = IDL_TYPE_SEQUENCE(tree).simple_type_spec;
    }
  if(ident)
    switch(IDL_NODE_TYPE(ident))
      {
      case IDLN_IDENT:
	g_print("%s ", IDL_IDENT(ident).str);
	break;
      default:
	g_print("%s ", IDL_tree_type_names[IDL_NODE_TYPE(ident)]);
	break;
      }

  g_print("(%s) [%p] ", IDL_tree_type_names[IDL_NODE_TYPE(tree)], tree);
  g_print("mtype ");
  print_mm(tmi->mtype);
  g_print("dmtype ");
  print_mm(tmi->dmtype);
  g_print("used %d size %d size-impact %d\n", tmi->use_count, tmi->size, tmi->use_count * tmi->size);
}

void
oidl_marshal_context_dump(OIDL_Marshal_Context *ctxt)
{
  g_hash_table_foreach(ctxt->type_marshal_info, dump_tmi, NULL);
}

OIDL_Type_Marshal_Info *
oidl_marshal_context_find(OIDL_Marshal_Context *ctxt, IDL_tree tree)
{
  if(!tree)
    return NULL;

  if(IDL_NODE_TYPE(tree) == IDLN_IDENT)
    {
      tree = IDL_NODE_UP(tree);
      if(IDL_NODE_TYPE(tree) == IDLN_LIST)
	tree = IDL_NODE_UP(tree);
    }

  return g_hash_table_lookup(ctxt->type_marshal_info, tree);
}
