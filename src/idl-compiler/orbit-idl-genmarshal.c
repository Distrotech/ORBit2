#include "config.h"
#include "orbit-idl2.h"

/* OK, the deal is that node->where should tell possible allocation strategies when demarshalling

   What determines 'where'?
   . If we are an atomic type, it is the container
   . If we are a complex type, it is the Populate_Info + container

   . For inout params, we can only use MW_Msg for sequence buffers and strings
   . We can't use MW_Auto for inout stuff, and MW_Alloca requires some conniving.

   Are there any cases where a marshal method is valid in a child but not a parent?
   . MW_Msg only applies to coalescable items (loop or set containers, or 

   The process of figuring out where to stick a variable goes like this:
      The container always has to do the allocating, not the contained.

      Types of containers are loop, switch, toplevel, and set.

      A loop container just has to allocate space for its immediate contents (e.g. using sizeof) except in MW_Msg case
      where no allocation necessary.
      A set container has to know about coalescibility.
      A switch container & set container both use the same allocation thing for children.
      A toplevel container will know how to do var setup and allocation.
Done:
. figure out exactly what node->where will determine
  It will determine how the children are allocated by the container.
. fix all the code to work according to above decision

*/

static OIDL_Marshal_Node *
oidl_marshal_node_new(OIDL_Marshal_Node *parent, OIDL_Marshal_Node_Type type, const char *name, OIDL_Populate_Info *pi)
{
  OIDL_Marshal_Node *retval;

  retval = g_new0(OIDL_Marshal_Node, 1);

  retval->up = parent;
  retval->type = type;
  retval->name = (char *)name;
  retval->arch_head_align = retval->arch_tail_align = retval->iiop_head_align = retval->iiop_tail_align = 1;
  retval->where = pi->where;

  if(parent)
    retval->flags |= (parent->flags & (MN_LOOPED));

  switch(type)
    {
    case MARSHAL_DATUM:
      retval->flags |= MN_NEED_CURPTR_LOCAL;
      break;
    case MARSHAL_COMPLEX:
      retval->flags |= MN_NEED_CURPTR_RECVBUF;
      break;
    case MARSHAL_CONST:
      retval->flags |= MN_NOMARSHAL;
      break;
    default:
      break;
    }

  return retval;
}

/* A node is an array element if it is
   Underneath a MARSHAL_LOOP and zero or more nameless nodes.
 */
static gboolean
oidl_marshal_node_is_arrayel(OIDL_Marshal_Node *node, OIDL_Marshal_Node **loopvar_ret) G_GNUC_UNUSED;
static gboolean
oidl_marshal_node_is_arrayel(OIDL_Marshal_Node *node, OIDL_Marshal_Node **loopvar_ret)
{
  OIDL_Marshal_Node *curnode;
  gboolean retval = FALSE;

  g_assert(node);

  for(curnode = node->up; curnode && !curnode->name && curnode->type != MARSHAL_LOOP; curnode = curnode->up) /* */;

  if(!curnode) return FALSE;

  if(curnode
     && (curnode->type == MARSHAL_LOOP)
     && curnode->u.loop_info.loop_var != node
     && curnode->u.loop_info.length_var != node) {
     retval = TRUE;

     if(loopvar_ret)
       *loopvar_ret = curnode->u.loop_info.loop_var;
  } else if(loopvar_ret)
    *loopvar_ret = NULL;
  
  return retval;
}

/**
   Returns the name, with the C language of a C variable
   that exists within some function. Used within C code (marshalling
   and unmarshalling) to read/write the variable.

  This routine is one of the most difficult ones to get right. 
  Dragons be here.
**/
char *
oidl_marshal_node_fqn(OIDL_Marshal_Node *node)
{
  GString *tmpstr;
  char *retval, *ctmp;
  OIDL_Marshal_Node *curnode;
  int nptrs = 0;
  int i;
  gboolean did_append = FALSE, new_did_append;

  if(!node->name
     && (
	 (node->flags & MN_NEED_TMPVAR)
	 || (node->type == MARSHAL_CONST))) {
    return g_strdup("<Unassigned>");
  }

  tmpstr = g_string_new("");

  for(curnode = node; curnode; curnode = curnode->up) {
    new_did_append = FALSE;

    if(curnode->up
       && !(curnode->flags & MN_NSROOT)) {
      switch(curnode->up->type) {
      case MARSHAL_LOOP:
	if(curnode == curnode->up->u.loop_info.contents) {
	  ctmp = oidl_marshal_node_fqn(curnode->up->u.loop_info.loop_var);
	  
	  if(did_append) {
	    g_string_prepend_c(tmpstr, '.');
	    did_append = FALSE;
	  }

	  g_string_prepend_c(tmpstr, ']');
	  g_string_prepend(tmpstr, ctmp);
	  g_string_prepend_c(tmpstr, '[');
	  g_free(ctmp);
	  if(curnode->up->flags & MN_ISSEQ) {
	    g_string_prepend(tmpstr, "._buffer");
	    did_append = FALSE;
	  }
	}
	break;
      default:
	break;
      }
    }
    if(curnode->name) {
      int my_numptrs;

      if(did_append)
	g_string_prepend_c(tmpstr, '.');

      my_numptrs = curnode->nptrs;
      if((curnode->flags & MN_ISSLICE)
	 && (curnode == node))
	my_numptrs++;

      nptrs += my_numptrs;

      for(i = 0; i < my_numptrs; i++)
	g_string_prepend_c(tmpstr, ')');

      g_string_prepend(tmpstr, curnode->name);

      for(i = 0; i < my_numptrs; i++)
	g_string_prepend_c(tmpstr, '*');

      new_did_append = TRUE;

      did_append = new_did_append;
    }

    if(curnode->flags & MN_NSROOT)
      break;
  }

  for(i = 0; i < nptrs; i++)
    g_string_prepend_c(tmpstr, '(');

  retval = tmpstr->str;

  g_string_free(tmpstr, FALSE);

  return retval;
}

OIDL_Marshal_Node *
marshal_populate(IDL_tree tree, OIDL_Marshal_Node *parent, OIDL_Populate_Info *pi)
{
  OIDL_Marshal_Node *retval = NULL;
  OIDL_Populate_Info subpi = *pi;

  if(!tree) return NULL;

  if(!(pi->flags & PI_BUILD_FUNC))
    {
      OIDL_Type_Marshal_Info *tmi;
      tmi = oidl_marshal_context_find(pi->ctxt, tree);

      if(tmi)
	switch(tmi->mtype)
	  {
	  case MARSHAL_INLINE:
	    break;
	  case MARSHAL_FUNC:
	  case MARSHAL_ANY:
	    retval = oidl_marshal_node_new(parent, MARSHAL_COMPLEX, NULL, pi);
	    retval->u.complex_info.type = CX_MARSHAL_METHOD;
	    retval->tree = tree;
	    goto out;
	    break;
	  default:
	    g_assert_not_reached();
	    break;
	  }
    }

  subpi.flags &= ~PI_BUILD_FUNC;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_INTEGER:
    retval = oidl_marshal_node_new(parent, MARSHAL_CONST, NULL, pi);
    retval->u.const_info.amount = IDL_INTEGER(tree).value;
    retval->tree = tree;
    retval->flags |= MN_NOMARSHAL;
    break;
  case IDLN_TYPE_OCTET:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL, pi);
    retval->u.datum_info.datum_size = sizeof(CORBA_octet);
    retval->tree = tree;
    break;
  case IDLN_TYPE_BOOLEAN:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL, pi);
    retval->u.datum_info.datum_size = sizeof(CORBA_boolean);
    retval->tree = tree;
    break;
  case IDLN_TYPE_CHAR:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL, pi);
    retval->u.datum_info.datum_size = sizeof(CORBA_char);
    retval->tree = tree;
    break;
  case IDLN_TYPE_WIDE_CHAR:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL, pi);
    retval->u.datum_info.datum_size = sizeof(CORBA_wchar);
    retval->tree = tree;
    break;
  case IDLN_TYPE_FLOAT:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL, pi);
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
    retval->tree = tree;
    break;
  case IDLN_TYPE_INTEGER:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL, pi);
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
    retval->tree = tree;
    break;
  case IDLN_TYPE_ENUM:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL, pi);
    retval->u.datum_info.datum_size = sizeof(CORBA_long);
    retval->tree = tree;
    break;
  case IDLN_TYPE_STRING:
  case IDLN_TYPE_WIDE_STRING:
    retval = oidl_marshal_node_new(parent, MARSHAL_LOOP, NULL, pi);
    retval->flags |= MN_ISSTRING;
    retval->u.loop_info.length_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL, &subpi);
    retval->u.loop_info.length_var->u.datum_info.datum_size = sizeof(CORBA_long);
    retval->u.loop_info.length_var->flags |= MN_NEED_TMPVAR;
    retval->u.loop_info.loop_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL, &subpi);
    retval->u.loop_info.loop_var->u.datum_info.datum_size = sizeof(CORBA_long);
    retval->u.loop_info.loop_var->flags |= MN_NEED_TMPVAR|MN_NOMARSHAL|MN_LOOPED;
    retval->u.loop_info.contents = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL, &subpi);
    retval->u.loop_info.contents->u.datum_info.datum_size = sizeof(CORBA_octet);
    retval->u.loop_info.contents->where &= MW_Msg|MW_Heap;
    retval->tree = tree;
    if(parent)
      retval->flags &= ~(parent->flags & MN_LOOPED);
    break;
  case IDLN_TYPE_ARRAY:
    {
      OIDL_Marshal_Node *cursub = NULL, *newsub;
      IDL_tree curlevel;

      for(curlevel = IDL_TYPE_ARRAY(tree).size_list; curlevel; curlevel = IDL_LIST(curlevel).next) {
	newsub = oidl_marshal_node_new(cursub?cursub:parent, MARSHAL_LOOP, NULL, retval?pi:&subpi);
	if(cursub != parent)
	  newsub->flags |= MN_LOOPED;

	if(!retval)
	  retval = newsub;

	if(cursub)
	  cursub->u.loop_info.contents = newsub;
	cursub = newsub;

	cursub->u.loop_info.loop_var = oidl_marshal_node_new(cursub, MARSHAL_DATUM, NULL, &subpi);
	cursub->u.loop_info.loop_var->u.datum_info.datum_size = sizeof(CORBA_long);
	cursub->u.loop_info.loop_var->flags |= MN_NOMARSHAL|MN_NEED_TMPVAR;

	cursub->u.loop_info.length_var = marshal_populate(IDL_LIST(curlevel).data, cursub, &subpi);
	cursub->u.loop_info.length_var->flags |= MN_NOMARSHAL;
      }

      cursub->u.loop_info.contents = marshal_populate(orbit_idl_get_array_type(tree), cursub, &subpi);
    }
    retval->tree = tree;
    break;
  case IDLN_TYPE_SEQUENCE:
    retval = oidl_marshal_node_new(parent, MARSHAL_LOOP, NULL, pi);
    retval->flags |= MN_ISSEQ|MN_LOOPED;

    retval->u.loop_info.loop_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL, pi);
    retval->u.loop_info.loop_var->u.datum_info.datum_size = sizeof(CORBA_unsigned_long);
    retval->u.loop_info.loop_var->flags |= MN_NOMARSHAL|MN_NEED_TMPVAR;

    retval->u.loop_info.length_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL, pi);
    retval->u.loop_info.length_var->u.datum_info.datum_size = sizeof(CORBA_unsigned_long);
    retval->u.loop_info.length_var->name = "_length";

    retval->u.loop_info.contents = marshal_populate(IDL_TYPE_SEQUENCE(tree).simple_type_spec, retval, &subpi);
    retval->u.loop_info.contents->flags |= MN_LOOPED;
    retval->tree = tree;
    if(parent)
      retval->flags &= ~(parent->flags & MN_LOOPED);
    break;
  case IDLN_TYPE_STRUCT:
  case IDLN_EXCEPT_DCL:
    {
      IDL_tree curitem;
      /* IDL_tree ident = IDL_NODE_TYPE(tree)==IDLN_TYPE_STRUCT 
       * ? IDL_TYPE_STRUCT(tree).ident : IDL_EXCEPT_DCL(tree).ident; */
      gboolean isRecur = IDL_tree_is_recursive( tree, /*hasRecur*/NULL);
      if ( isRecur && tree->data) {
	return tree->data;
      } else {
	  retval = oidl_marshal_node_new(parent, MARSHAL_SET, NULL, pi);
	  retval->tree = tree;
	  if(isRecur)
	    retval->flags |= MN_RECURSIVE_TOP;
	  tree->data = retval;
	  for(curitem = IDL_TYPE_STRUCT(tree).member_list; curitem; curitem = IDL_LIST(curitem).next) {
	    OIDL_Marshal_Node *newnode
	     = marshal_populate(IDL_LIST(curitem).data, retval, &subpi);
	    retval->u.set_info.subnodes 
	      = g_slist_append(retval->u.set_info.subnodes, newnode);
	  }
	  tree->data = NULL;
      }
    }
    retval->tree = tree;
    break;
  case IDLN_TYPE_UNION:
    {
      IDL_tree ntmp;
      gboolean isRecur = IDL_tree_is_recursive( tree, /*hasRecur*/NULL);
      if ( isRecur && tree->data) {
	return tree->data;
      } else {
	  retval = oidl_marshal_node_new(parent, MARSHAL_SWITCH, NULL, pi);
	  retval->tree = tree;
	  if(isRecur)
	    retval->flags |= MN_RECURSIVE_TOP;
	  tree->data = retval;
	  retval->u.switch_info.discrim = marshal_populate(IDL_TYPE_UNION(tree).switch_type_spec, retval, &subpi);
	  retval->u.switch_info.discrim->name = "_d";
	  for(ntmp = IDL_TYPE_UNION(tree).switch_body; ntmp; ntmp = IDL_LIST(ntmp).next) {
	    OIDL_Marshal_Node * newnode
	      = marshal_populate(IDL_LIST(ntmp).data, retval, &subpi);
	    retval->u.switch_info.cases 
	      = g_slist_append(retval->u.switch_info.cases, newnode);
	  }
	  tree->data = NULL;
      }
    }
    break;

  case IDLN_MEMBER:
    {
      IDL_tree curitem, curnode;
      OIDL_Marshal_Node *tnode = NULL; /* Quiet gcc */

      retval = oidl_marshal_node_new(parent, MARSHAL_SET, NULL, pi);
      
      for(curitem = IDL_MEMBER(tree).dcls; curitem; curitem = IDL_LIST(curitem).next) {
	curnode = IDL_LIST(curitem).data;

	if(IDL_NODE_TYPE(curnode) == IDLN_IDENT) {
	  tnode = marshal_populate(IDL_MEMBER(tree).type_spec, retval, &subpi);
	} else if(IDL_NODE_TYPE(curnode) == IDLN_TYPE_ARRAY) {
	  tnode = marshal_populate(curnode, retval, &subpi);
	} else
	  g_error("A member that is not an ident nor an array?");

	tnode->name = orbit_idl_member_get_name(curnode);

	retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, tnode);
      }
    }
    retval->tree = tree;
    break;
  case IDLN_CASE_STMT:
    {
      IDL_tree ntmp;
      retval = oidl_marshal_node_new(parent, MARSHAL_CASE, "_u", pi);
      retval->u.case_info.contents = marshal_populate(IDL_CASE_STMT(tree).element_spec, retval, &subpi);
      for(ntmp = IDL_CASE_STMT(tree).labels; ntmp; ntmp = IDL_LIST(ntmp).next) {
	OIDL_Marshal_Node * newnode;

	newnode = marshal_populate(IDL_LIST(ntmp).data, retval, &subpi);

	retval->u.case_info.labels = g_slist_append(retval->u.case_info.labels, newnode);
      }

      retval->tree = tree;
    }
    break;
  case IDLN_IDENT:
    retval = marshal_populate(IDL_get_parent_node(tree, IDLN_ANY, NULL), parent, &subpi);
    retval->tree = tree;
    break;
  case IDLN_LIST:
    retval = marshal_populate(IDL_get_parent_node(tree, IDLN_ANY, NULL), parent, &subpi);
    break;
  case IDLN_TYPE_DCL:
    retval = marshal_populate(IDL_TYPE_DCL(tree).type_spec, parent, &subpi);
    break;
  case IDLN_PARAM_DCL:
    retval = marshal_populate(IDL_PARAM_DCL(tree).param_type_spec, parent, &subpi);
    g_assert(retval);
    g_assert(!retval->name);
    retval->name = IDL_IDENT(IDL_PARAM_DCL(tree).simple_declarator).str;
    break;
  case IDLN_TYPE_FIXED:
    retval = oidl_marshal_node_new(parent, MARSHAL_COMPLEX, NULL, pi);
    retval->u.complex_info.type = CX_CORBA_FIXED;
    retval->tree = tree;
    break;
  case IDLN_TYPE_ANY:
    retval = oidl_marshal_node_new(parent, MARSHAL_COMPLEX, NULL, pi);
    retval->u.complex_info.type = CX_CORBA_ANY;
    retval->tree = tree;
    break;
  case IDLN_TYPE_TYPECODE:
    retval = oidl_marshal_node_new(parent, MARSHAL_COMPLEX, NULL, pi);
    retval->u.complex_info.type = CX_CORBA_TYPECODE;
    retval->tree = tree;
    break;
  case IDLN_TYPE_OBJECT:
  case IDLN_INTERFACE:
    retval = oidl_marshal_node_new(parent, MARSHAL_COMPLEX, NULL, pi);
    retval->u.complex_info.type = CX_CORBA_OBJECT;
    retval->tree = tree;
    break;
  case IDLN_CHAR:
  case IDLN_WIDE_CHAR:
  case IDLN_STRING:
  case IDLN_WIDE_STRING:
  case IDLN_FIXED:
  case IDLN_FLOAT:
  case IDLN_BOOLEAN:
    retval = oidl_marshal_node_new(parent, MARSHAL_CONST, NULL, pi);
    retval->tree = tree;
    break;
  case IDLN_NATIVE:
    /* we dont have access to runinfo from here -- should warn if not PIDL */
    retval = oidl_marshal_node_new(parent, MARSHAL_COMPLEX, NULL, pi);
    retval->u.complex_info.type = CX_NATIVE;
    retval->tree = tree;
    break;
  default:
    g_warning("Not populating for %s", IDL_tree_type_names[IDL_NODE_TYPE(tree)]);
    break;
  }

 out:
  g_return_val_if_fail(retval, retval);

  return retval;
}

/**
    For each parameter, there is generally a parameter reference
    and the parameter value (roughly corresponding to a pointer
    and what is pointed to).  Some params are passed by value,
    in which case the reference (pointer) is implicit.

    STUB IN		Parameters treated read-only.
    			Client allocs reference (poss. implicit) and 
    			value.
    STUB OUT		Always by pass-by-reference; client defines reference,
			stub may need to alloc value. Client will free.
    SKEL IN		Alloc required; freed by skel.
    SKEL OUT		Freed by skel after marshalling.
**/

OIDL_Marshal_Node *
orbit_idl_marshal_populate_in(IDL_tree tree, gboolean is_skels, OIDL_Marshal_Context *ctxt)
{
  OIDL_Marshal_Node *retval;
  IDL_tree curitem, curparam, ts;
  OIDL_Populate_Info pi = {NULL};
  gboolean isSlice;

  g_assert(IDL_NODE_TYPE(tree) == IDLN_OP_DCL);

  if(!(IDL_OP_DCL(tree).parameter_dcls
       || IDL_OP_DCL(tree).context_expr)) return NULL;

  pi.ctxt = ctxt;
  pi.where = MW_Any;

  retval = oidl_marshal_node_new(NULL, MARSHAL_SET, NULL, &pi);

  retval->flags |= MN_TOPLEVEL;

  for(curitem = IDL_OP_DCL(tree).parameter_dcls; curitem; curitem = IDL_LIST(curitem).next) {
    OIDL_Marshal_Node *sub;

    curparam = IDL_LIST(curitem).data;

    if(IDL_PARAM_DCL(curparam).attr == IDL_PARAM_OUT)
      continue;

#if 0
    if(is_skels && !strcmp(IDL_IDENT(IDL_PARAM_DCL(curparam).simple_declarator).str, "p1"))
      G_BREAKPOINT();
#endif

    if(is_skels)
      {
	pi.where = MW_Null|MW_Heap;
	if(IDL_PARAM_DCL(curparam).attr == IDL_PARAM_IN)
	  pi.where |= MW_Alloca|MW_Auto|MW_Msg;
      }
    else
      pi.where = MW_Null;
    sub = marshal_populate(curparam, retval, &pi);

    ts = orbit_cbe_get_typespec(curparam);

    switch(IDL_PARAM_DCL(curparam).attr) {
    case IDL_PARAM_INOUT:
      sub->nptrs = oidl_param_info(curparam, DATA_INOUT, &isSlice);
      sub->flags |= MN_INOUT;
      break;
    case IDL_PARAM_IN:
      sub->nptrs = oidl_param_info(curparam, DATA_IN, &isSlice);
      break;
    default:
      g_error("Weird param direction for in pass.");
      break;
    }
    sub->marshal_error_exit = g_strdup_printf("goto %s_demarshal_error",
					      IDL_IDENT(IDL_PARAM_DCL(curparam).simple_declarator).str);

    if(isSlice)
      sub->flags |= MN_ISSLICE;

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, sub);
  }

  if(IDL_OP_DCL(tree).context_expr) {
    OIDL_Marshal_Node *mnode;
    int i;
    IDL_tree curitem;

    for(i = 0, curitem = IDL_OP_DCL(tree).context_expr; curitem; curitem = IDL_LIST(curitem).next, i++) /* */;

    mnode = oidl_marshal_node_new(retval, MARSHAL_COMPLEX, NULL, &pi);
    mnode->u.complex_info.type = CX_CORBA_CONTEXT;
    mnode->u.complex_info.context_item_count = i;
    mnode->marshal_error_exit = g_strdup("goto _context_demarshal_error");

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, mnode);
  }

  return retval;
}

OIDL_Marshal_Node *
orbit_idl_marshal_populate_out(IDL_tree tree, gboolean is_skels, OIDL_Marshal_Context *ctxt)
{
  OIDL_Marshal_Node *retval, *rvnode;
  IDL_tree curitem, curparam;
  OIDL_Populate_Info pi = {NULL};
  gboolean isSlice;

  g_assert(IDL_NODE_TYPE(tree) == IDLN_OP_DCL);

  pi.ctxt = ctxt;
  pi.where = MW_Any;
  retval = oidl_marshal_node_new(NULL, MARSHAL_SET, NULL, &pi);
  retval->flags |= MN_TOPLEVEL;

  if(IDL_OP_DCL(tree).op_type_spec) {
    if(is_skels)
      pi.where = MW_Null;
    else
      pi.where = MW_Heap|MW_Null;
    rvnode = marshal_populate(IDL_OP_DCL(tree).op_type_spec, retval, &pi);
    if(!rvnode) goto out1;

    rvnode->nptrs = oidl_param_info(IDL_OP_DCL(tree).op_type_spec, DATA_RETURN, &isSlice);
    if ( isSlice )
      {
    	rvnode->nptrs -= 1;
	rvnode->flags |= MN_ISSLICE;
      }
    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, rvnode);

    g_assert(! rvnode->name);

    rvnode->name = ORBIT_RETVAL_VAR_NAME;
    rvnode->marshal_error_exit = g_strdup_printf("goto %s_demarshal_error", ORBIT_RETVAL_VAR_NAME);
  }

 out1:

  for(curitem = IDL_OP_DCL(tree).parameter_dcls; curitem; curitem = IDL_LIST(curitem).next) {
    OIDL_Marshal_Node *sub;

    curparam = IDL_LIST(curitem).data;

    if(IDL_PARAM_DCL(curparam).attr == IDL_PARAM_IN)
      continue;

    if(is_skels)
      pi.where = MW_Null;
    else
      pi.where = MW_Heap|MW_Null;
    sub = marshal_populate(curparam, retval, &pi);

    switch(IDL_PARAM_DCL(curparam).attr) {
    case IDL_PARAM_INOUT:
      sub->flags |= MN_INOUT;
      sub->nptrs = oidl_param_info(curparam, DATA_INOUT, &isSlice);
      break;
    case IDL_PARAM_OUT:
      sub->nptrs = oidl_param_info(curparam, DATA_OUT, &isSlice);
      if ( isSlice )
	  sub->nptrs -= 1;
      break;
    default:
      g_error("Weird param direction for out pass.");
      break;
    }
    if (isSlice)
      sub->flags |= MN_ISSLICE;

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, sub);
    sub->marshal_error_exit = g_strdup_printf("goto %s_demarshal_error",
					      IDL_IDENT(IDL_PARAM_DCL(curparam).simple_declarator).str);
  }

  return retval;
}

OIDL_Marshal_Node *
orbit_idl_marshal_populate_except_marshal(IDL_tree tree, OIDL_Marshal_Context *ctxt)
{
  OIDL_Marshal_Node *retval;
  OIDL_Populate_Info pi = {NULL};

  pi.ctxt = ctxt;
  pi.where = MW_Null;
  retval = marshal_populate(tree, NULL, &pi);
  retval->name = "_ORBIT_exdata";
  retval->nptrs = 1;
  retval->flags |= MN_TOPLEVEL;

  return retval;
}

OIDL_Marshal_Node *
orbit_idl_marshal_populate_except_demarshal(IDL_tree tree, OIDL_Marshal_Context *ctxt)
{
  OIDL_Marshal_Node *retval;
  OIDL_Populate_Info pi = {NULL};

  pi.ctxt = ctxt;
  pi.where = MW_Heap|MW_Null;
  retval = marshal_populate(tree, NULL, &pi);
  retval->name = "_ORBIT_exdata";
  retval->nptrs = 1;
  retval->flags |= MN_TOPLEVEL;

  return retval;
}
