#include "config.h"
#include "orbit-idl2.h"
#include "orbit-idl-c-backend.h"

typedef void (*OIDL_Pass_Func)(IDL_tree tree, gpointer data, gboolean is_demarshal);
typedef void (*OIDL_Node_Pass_Func)(OIDL_Marshal_Node *node);

static void oidl_pass_run_for_ops(IDL_tree tree, GFunc func, gboolean is_demarshal);

static void orbit_idl_collapse_sets(OIDL_Marshal_Node *node);
static void oidl_pass_make_updates(OIDL_Marshal_Node *node);
static void oidl_pass_tmpvars(IDL_tree tree, GFunc dummy, gboolean is_demarshal);
static void oidl_pass_set_coalescibility(OIDL_Marshal_Node *node);
static void oidl_pass_set_alignment(OIDL_Marshal_Node *node);
static gboolean oidl_pass_set_endian_dependant(OIDL_Marshal_Node *node);
static void oidl_pass_del_tail_update(OIDL_Marshal_Node *node); /* Must run after coalescibility */
static void oidl_pass_set_first_curptr_usage(OIDL_Marshal_Node *node);
static void oidl_pass_choose_where(OIDL_Marshal_Node *node);
static void oidl_pass_check_bounds(OIDL_Marshal_Node *node);
static void oidl_node_pass_tmpvars(OIDL_Marshal_Node *top);

static struct {
  const char *name;
  OIDL_Pass_Func func;
  OIDL_Node_Pass_Func data;
  enum { FOR_IN=1<<0, FOR_OUT=1<<1 } dirs;
} idl_passes[] = {
  {"Position updates", (OIDL_Pass_Func)oidl_pass_run_for_ops, oidl_pass_make_updates, FOR_OUT},
  {"Alignment calculation", (OIDL_Pass_Func)oidl_pass_run_for_ops, oidl_pass_set_alignment, FOR_IN|FOR_OUT},
  {"Endian dependancy", (OIDL_Pass_Func)oidl_pass_run_for_ops, (OIDL_Node_Pass_Func)oidl_pass_set_endian_dependant,
   FOR_IN|FOR_OUT},
  {"Coalescibility", (OIDL_Pass_Func)oidl_pass_run_for_ops, oidl_pass_set_coalescibility, FOR_IN|FOR_OUT},
  {"Extra update removal", (OIDL_Pass_Func)oidl_pass_run_for_ops, oidl_pass_del_tail_update, FOR_OUT},
  {"Variable assignment", (OIDL_Pass_Func)oidl_pass_tmpvars, oidl_node_pass_tmpvars, FOR_IN|FOR_OUT},
  {"First curptr usage", (OIDL_Pass_Func)oidl_pass_run_for_ops, oidl_pass_set_first_curptr_usage, FOR_IN|FOR_OUT},
  {"Memory allocation", (OIDL_Pass_Func)oidl_pass_run_for_ops, oidl_pass_choose_where, FOR_OUT},
  {"Bounds checking", (OIDL_Pass_Func)oidl_pass_run_for_ops, oidl_pass_check_bounds, FOR_OUT},
  {NULL, NULL}
};

void
orbit_idl_do_node_passes(OIDL_Marshal_Node *node, gboolean is_demarshal)
{
  gint flag = is_demarshal?FOR_OUT:FOR_IN;
  int i;

  for(i = 0; idl_passes[i].name; i++)
    {
      if(idl_passes[i].dirs & flag)
	idl_passes[i].data(node);
    }
}

void
orbit_idl_do_passes(IDL_tree tree, OIDL_Run_Info *rinfo)
{
  int i;

  for(i = 0; idl_passes[i].name; i++) {
    if(idl_passes[i].dirs & FOR_IN)
      idl_passes[i].func(tree, idl_passes[i].data, 0);
  }

  for(i = 0; idl_passes[i].name; i++) {
    if(idl_passes[i].dirs & FOR_OUT)
      idl_passes[i].func(tree, idl_passes[i].data, 1);
  }

  if(rinfo->debug_level > 2)
    oidl_marshal_tree_dump(tree, 0);
}

static void
orbit_idl_assign_tmpvar_names(OIDL_Marshal_Node *node, int *counter)
{
  if(!(node->flags & MN_NEED_TMPVAR))
    return;

  if(node->name)
    return;

  /* Special case TODO - if loop contents are coalescable, don't need to have a tmpvar for the loop var */

  node->name = g_strdup_printf("_ORBIT_tmpvar_%d", *counter);
  node->flags |= MN_NSROOT;

  (*counter)++;
}

static void
orbit_idl_tmpvars_assign(OIDL_Marshal_Node *top, int *counter)
{
  orbit_idl_node_foreach(top, (GFunc)orbit_idl_assign_tmpvar_names, counter);
}

static void
oidl_node_pass_tmpvars(OIDL_Marshal_Node *top)
{
  int ctr = 0;
  orbit_idl_tmpvars_assign(top, &ctr);
}

static void
oidl_pass_tmpvars(IDL_tree tree, GFunc dummy, gboolean is_demarshal)
{
  IDL_tree node;

  if(!tree) return;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_LIST:
    for(node = tree; node; node = IDL_LIST(node).next) {
      oidl_pass_tmpvars(IDL_LIST(node).data, dummy, is_demarshal);
    }
    break;
  case IDLN_MODULE:
    oidl_pass_tmpvars(IDL_MODULE(tree).definition_list, dummy, is_demarshal);
    break;
  case IDLN_INTERFACE:
    oidl_pass_tmpvars(IDL_INTERFACE(tree).body, dummy, is_demarshal);
    break;
  case IDLN_EXCEPT_DCL:
    {
      OIDL_Except_Info *ei = tree->data;
      int ctr = 0;

      if(is_demarshal)
	orbit_idl_tmpvars_assign(ei->demarshal, &ctr);
      else
	orbit_idl_tmpvars_assign(ei->marshal, &ctr);
    }
    break;
  case IDLN_OP_DCL:
    {
      OIDL_Op_Info *oi;

      oi = tree->data;

      if(is_demarshal) {
	if(oi->out_stubs)
	  orbit_idl_tmpvars_assign(oi->out_stubs, &oi->counter);
	if(oi->in_skels)
	  orbit_idl_tmpvars_assign(oi->in_skels, &oi->counter);
      } else {
	if(oi->in_stubs)
	  orbit_idl_tmpvars_assign(oi->in_stubs, &oi->counter);
	if(oi->out_skels)
	  orbit_idl_tmpvars_assign(oi->out_skels, &oi->counter);
      }
    }
    break;
  case IDLN_ATTR_DCL:
    {
      IDL_tree curnode, attr_name;

      for(curnode = IDL_ATTR_DCL(tree).simple_declarations; curnode; curnode = IDL_LIST(curnode).next) {
	attr_name = IDL_LIST(curnode).data;

	oidl_pass_tmpvars(((OIDL_Attr_Info *)attr_name->data)->op1, dummy, is_demarshal);
	if(((OIDL_Attr_Info *)attr_name->data)->op2)
	  oidl_pass_tmpvars(((OIDL_Attr_Info *)attr_name->data)->op2, dummy, is_demarshal);
      }
    }
    break;
  default:
    break;
  }
}

static void
oidl_pass_run_for_ops(IDL_tree tree, GFunc func, gboolean is_demarshal)
{
  IDL_tree node;

  if(!tree) return;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_LIST:
    for(node = tree; node; node = IDL_LIST(node).next)
      oidl_pass_run_for_ops(IDL_LIST(node).data, func, is_demarshal);
    break;
  case IDLN_MODULE:
    oidl_pass_run_for_ops(IDL_MODULE(tree).definition_list, func, is_demarshal);
    break;
  case IDLN_INTERFACE:
    oidl_pass_run_for_ops(IDL_INTERFACE(tree).body, func, is_demarshal);
    break;
  case IDLN_EXCEPT_DCL:
    {
      OIDL_Except_Info *ei = tree->data;
      if(is_demarshal)
	func(ei->demarshal, NULL);
      else
	func(ei->marshal, NULL);
    }
    break;
  case IDLN_OP_DCL:
    {
      OIDL_Op_Info *oi = (OIDL_Op_Info *)tree->data;

      if(is_demarshal) {
	if(oi->out_stubs)
	  func(oi->out_stubs, NULL);
	if(oi->in_skels)
	  func(oi->in_skels, NULL);
      } else {
	if(oi->in_stubs)
	  func(oi->in_stubs, NULL);
	if(oi->out_skels)
	  func(oi->out_skels, NULL);
      }
    }
    break;
  case IDLN_ATTR_DCL:
    {
      IDL_tree curnode;

      for(curnode = IDL_ATTR_DCL(tree).simple_declarations; curnode; curnode = IDL_LIST(curnode).next) {
	IDL_tree attr_name = IDL_LIST(curnode).data;
	OIDL_Attr_Info *ai = attr_name->data;
	oidl_pass_run_for_ops(ai->op1, func, is_demarshal);
	if(ai->op2)
	  oidl_pass_run_for_ops(ai->op2, func, is_demarshal);
      }
    }
    break;
  default:
    break;
  }  
}

static guint8
oidl_get_tree_alignment(IDL_tree tree)
{
  guint8 itmp = 0; /* Quiet gcc */

  g_assert(tree);

  /* find arch alignments */
  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_TYPE_INTEGER:
    switch(IDL_TYPE_INTEGER(tree).f_type) {
    case IDL_INTEGER_TYPE_SHORT:
      itmp = ALIGNOF_CORBA_SHORT;
      break;
    case IDL_INTEGER_TYPE_LONG:
      itmp = ALIGNOF_CORBA_LONG;
      break;
    case IDL_INTEGER_TYPE_LONGLONG:
      itmp = ALIGNOF_CORBA_LONG_LONG;
      break;
    default:
      g_error("Weird integer type");
      break;
    }
    break;
  case IDLN_TYPE_ENUM:
    itmp = ALIGNOF_CORBA_LONG;
    break;
  case IDLN_TYPE_FLOAT:
    switch(IDL_TYPE_FLOAT(tree).f_type) {
    case IDL_FLOAT_TYPE_FLOAT:
      itmp = ALIGNOF_CORBA_FLOAT;
      break;
    case IDL_FLOAT_TYPE_DOUBLE:
      itmp = ALIGNOF_CORBA_DOUBLE;
      break;
    case IDL_FLOAT_TYPE_LONGDOUBLE:
      itmp = ALIGNOF_CORBA_LONG_DOUBLE;
      break;
    default:
      g_error("Weird float type");
      break;
    }
    break;
  case IDLN_TYPE_OCTET:
  case IDLN_TYPE_CHAR:
    itmp = ALIGNOF_CORBA_CHAR;
    break;
  case IDLN_TYPE_BOOLEAN:
    itmp = ALIGNOF_CORBA_BOOLEAN;
    break;
  case IDLN_IDENT:
    itmp = oidl_get_tree_alignment(orbit_cbe_get_typespec(tree));
    break;
  default:
    g_error("Don't know how to get alignment of a %s datum", IDL_tree_type_names[IDL_NODE_TYPE(tree)]);
    break;
  }

  return itmp;
}

static void
oidl_pass_set_alignment_datum(OIDL_Marshal_Node *node)
{
  if(node->tree) {
    guint8 itmp;

    itmp = oidl_get_tree_alignment(node->tree);
    node->arch_head_align = node->arch_tail_align = itmp; /* I don't think there's any cases where these aren't equal */
  } else
    node->arch_head_align = node->arch_tail_align = node->u.datum_info.datum_size;

  node->iiop_head_align = node->iiop_tail_align = node->u.datum_info.datum_size;
}

static void
oidl_pass_set_alignment(OIDL_Marshal_Node *node)
{
  if(!node) return;

  if(node->use_count)
    return;

  node->use_count++;

  switch(node->type) {
  case MARSHAL_DATUM:
    oidl_pass_set_alignment_datum(node);
    break;
  case MARSHAL_SET:
    {
      GSList *ltmp;
      guint8 itmp;
      OIDL_Marshal_Node *sub;

      g_slist_foreach(node->u.set_info.subnodes, (GFunc)oidl_pass_set_alignment, NULL);

      itmp = ALIGNOF_CORBA_STRUCT;
      for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp)) {
	sub = ltmp->data;
	itmp = MAX(MAX(itmp, sub->arch_head_align), sub->arch_tail_align);
      }
      node->arch_head_align = node->arch_tail_align = itmp;

      ltmp = node->u.set_info.subnodes;
      if(ltmp) {
	sub = ltmp->data;
	node->iiop_head_align = sub->iiop_head_align;

	ltmp = g_slist_last(node->u.set_info.subnodes);
	sub = ltmp->data;
	node->iiop_tail_align = sub->iiop_tail_align;
      } else
	node->iiop_head_align = node->iiop_tail_align = 1;
    }
    break;
  case MARSHAL_SWITCH:
    {
      GSList *ltmp;
      oidl_pass_set_alignment(node->u.switch_info.discrim);
      g_slist_foreach(node->u.switch_info.cases, (GFunc)oidl_pass_set_alignment, NULL);
      node->arch_head_align = MAX(node->u.switch_info.discrim->arch_head_align, ALIGNOF_CORBA_STRUCT);
      node->arch_tail_align = MAX(node->u.switch_info.discrim->arch_tail_align, ALIGNOF_CORBA_STRUCT);
      for(ltmp = node->u.switch_info.cases, node->iiop_tail_align = ((OIDL_Marshal_Node *)ltmp->data)->iiop_tail_align;
	  ltmp; ltmp = g_slist_next(ltmp)) {
	OIDL_Marshal_Node *sub;

	sub = ltmp->data;
	node->arch_head_align = MAX(node->arch_head_align, sub->arch_head_align);
	node->arch_tail_align = MAX(node->arch_tail_align, sub->arch_tail_align);
	node->iiop_tail_align = MIN(node->iiop_tail_align, sub->iiop_tail_align);
      }
      node->iiop_head_align = node->u.switch_info.discrim->iiop_head_align;
    }
    break;
  case MARSHAL_CASE:
    oidl_pass_set_alignment(node->u.case_info.contents);
    node->arch_head_align = node->u.case_info.contents->arch_head_align;
    node->arch_tail_align = node->u.case_info.contents->arch_tail_align;
    node->iiop_head_align = node->u.case_info.contents->iiop_head_align;
    node->iiop_tail_align = node->u.case_info.contents->iiop_tail_align;
    break;
  case MARSHAL_CONST:
    node->arch_head_align = node->arch_tail_align = node->iiop_head_align = node->iiop_tail_align = 1;
    break;
  case MARSHAL_COMPLEX:
    /* Used to set node->iiop_head_align to the size of the first element in the complex thingie,
       but that just caused extra alignment instructions to be generated, so I set it to 1 now and
       require the complex marshal/demarshal function to internally ensure its alignment requirements are met */
    switch(node->u.complex_info.type) {
    case CX_CORBA_FIXED:
      node->arch_head_align = MAX(ALIGNOF_CORBA_STRUCT, ALIGNOF_CORBA_SHORT);
      node->arch_tail_align = MAX(ALIGNOF_CORBA_STRUCT, ALIGNOF_CORBA_SHORT);
      node->iiop_head_align = 1; /* sizeof(CORBA_short); */
      node->iiop_tail_align = 1;
      break;
    case CX_CORBA_OBJECT:
      node->arch_head_align = node->arch_tail_align = sizeof(gpointer);
      node->iiop_head_align = 1; /* sizeof(CORBA_unsigned_long); */ /* Leading type_id string */
      node->iiop_tail_align = 1;
      break;
    case CX_CORBA_ANY:
      node->arch_head_align = node->arch_tail_align = MAX(ALIGNOF_CORBA_STRUCT, ALIGNOF_CORBA_POINTER);
      node->iiop_head_align = 1; /* sizeof(CORBA_unsigned_long); */ /* TCKind enum */
      node->iiop_tail_align = 1;
      break;
    case CX_CORBA_TYPECODE:
      node->arch_head_align = node->arch_tail_align = MAX(ALIGNOF_CORBA_STRUCT,
							  MAX(ALIGNOF_CORBA_POINTER, ALIGNOF_CORBA_LONG));
      node->iiop_head_align = 1; /* sizeof(CORBA_unsigned_long); */ /* TCKind enum */
      node->iiop_tail_align = 1;
      break;
    case CX_CORBA_CONTEXT:
      node->arch_head_align = node->arch_tail_align = sizeof(CORBA_unsigned_long);
      node->iiop_head_align = 1; /* sizeof(CORBA_unsigned_long); */ /* sequence<string> length */
      node->iiop_tail_align = 1;
      break;
    case CX_NATIVE:
      break;
    case CX_MARSHAL_METHOD:
      node->arch_head_align = node->arch_tail_align = 1;
      node->iiop_head_align = 1;
      node->iiop_tail_align = 1;
      break;
    }
    break;
  case MARSHAL_LOOP:
    oidl_pass_set_alignment(node->u.loop_info.loop_var);
    oidl_pass_set_alignment(node->u.loop_info.length_var);
    oidl_pass_set_alignment(node->u.loop_info.contents);
    node->iiop_head_align = node->u.loop_info.loop_var->iiop_head_align;
    node->iiop_tail_align = node->u.loop_info.contents->iiop_tail_align;
    break;
  default:
    g_error("Alignment of type %d not known", node->type);
    break;
  }

  node->use_count--;
}

/*
 * there be dragons here.
 *
 * Possible optimizations for a structure with datums sized [2, 1, 1, 2] exist, but I don't know how to do this yet.
 * You can also optimize [1, 1, 2] in certain cases.
 */
/* Right now we don't really use prev_alignment for anything, but we calculate it because it will probably be needed
   to do the above cases and more */
static gint
oidl_pass_set_coalescibility_2(OIDL_Marshal_Node *node, gint prev_alignment)
{
  gboolean elements_ok;
  OIDL_Marshal_Node *sub;
  gint retval = 0; /* Quiet gcc */
  gboolean check_marshal;

  if(!node) return prev_alignment;
  if(node->use_count)
    return prev_alignment;
  node->use_count++;

  switch(node->type) {
  case MARSHAL_DATUM:
  case MARSHAL_CONST: /* ??? */
    node->flags |= MN_COALESCABLE;
    retval = node->iiop_tail_align;
    break;
  case MARSHAL_LOOP:
    retval = prev_alignment;
    if(!(node->u.loop_info.loop_var->flags & MN_NOMARSHAL))
      retval = oidl_pass_set_coalescibility_2(node->u.loop_info.loop_var, prev_alignment);
    if(!(node->u.loop_info.length_var->flags & MN_NOMARSHAL))
      retval = oidl_pass_set_coalescibility_2(node->u.loop_info.length_var, retval);
    if(!(node->u.loop_info.contents->flags & MN_NOMARSHAL))
      retval = oidl_pass_set_coalescibility_2(node->u.loop_info.contents, retval);

    if(node->flags & (MN_ISSEQ|MN_ISSTRING)) break;
    elements_ok = TRUE;
    check_marshal = FALSE;
    if(!(node->u.loop_info.loop_var->flags & MN_NOMARSHAL))
      elements_ok = elements_ok && (node->u.loop_info.loop_var->flags & MN_COALESCABLE);
    if(!(node->u.loop_info.length_var->flags & MN_NOMARSHAL))
      elements_ok = elements_ok && (node->u.loop_info.length_var->flags & MN_COALESCABLE);
    if(!(node->u.loop_info.contents->flags & MN_NOMARSHAL))
      elements_ok = elements_ok && (node->u.loop_info.contents->flags & MN_COALESCABLE);
    if(elements_ok)
      node->flags |= MN_COALESCABLE;
    break;
  case MARSHAL_SET:
    {
      GSList *ltmp;
      int tv = 0; /* Quiet gcc */

      elements_ok = TRUE;

      retval = prev_alignment;
      for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp)) {

	sub = ltmp->data;
	tv = oidl_pass_set_coalescibility_2(sub, retval);
	if(sub->flags & MN_NOMARSHAL) continue;

	retval = tv;

	elements_ok = elements_ok && (sub->flags & MN_COALESCABLE);
      }

      if(!elements_ok) break;

      elements_ok = TRUE;
      check_marshal = FALSE;
      /* Now figure out whether alignment issues will allow coalescing */
      for(ltmp = node->u.set_info.subnodes; ltmp && elements_ok; ltmp = g_slist_next(ltmp)) {
	sub = ltmp->data;
	if(sub->flags & MN_NOMARSHAL) continue;

	elements_ok = elements_ok
	  && (sub->iiop_head_align >= sub->arch_head_align)
	  && (sub->iiop_tail_align >= sub->iiop_tail_align)
	  && (!check_marshal || (tv >= sub->iiop_head_align));

	if(!check_marshal)
	  tv = sub->iiop_tail_align;
	else
	  tv = MIN(sub->iiop_tail_align, tv);

	check_marshal = TRUE;
      }

      if(elements_ok)
	node->flags |= MN_COALESCABLE;
    }
    break;
  case MARSHAL_SWITCH:
    oidl_pass_set_coalescibility_2(node->u.switch_info.discrim, prev_alignment);
    g_slist_foreach(node->u.switch_info.cases, (GFunc)oidl_pass_set_coalescibility_2,
		    GUINT_TO_POINTER((int)node->u.switch_info.discrim->iiop_tail_align));
    /* Not usually coalescible, even if children are. */
    if(g_slist_length(node->u.switch_info.cases) < 2) { /* Further improvement possible - this should also kick in if
							   all the cases use types of the same size. */
      OIDL_Marshal_Node *sub;

      sub = node->u.switch_info.cases->data;
      if((sub->flags & MN_COALESCABLE)
	 && (node->u.switch_info.discrim->flags & MN_COALESCABLE)
	 && (node->u.switch_info.discrim->iiop_tail_align == node->u.switch_info.discrim->arch_tail_align)
	 && (sub->iiop_head_align == sub->arch_head_align))
	node->flags |= MN_COALESCABLE;
    }
    break;
  case MARSHAL_CASE:
    g_slist_foreach(node->u.case_info.labels, (GFunc)oidl_pass_set_coalescibility_2, GUINT_TO_POINTER(prev_alignment));
    retval = oidl_pass_set_coalescibility_2(node->u.case_info.contents, prev_alignment);
    break;
  case MARSHAL_COMPLEX:
    retval = 1;
    break;
  default:
    g_warning("default coalesce for %d", node->type);
    retval = prev_alignment;
    break;
  }

  node->use_count--;
  return retval;
}

static void
oidl_pass_set_coalescibility(OIDL_Marshal_Node *node)
{
  oidl_pass_set_coalescibility_2(node, 1);
}

static gboolean
oidl_pass_set_endian_dependant(OIDL_Marshal_Node *node)
{
  GSList *ltmp;
  gboolean btmp = FALSE;

  if(!node) return FALSE;

  if(node->use_count || (node->flags & MN_NOMARSHAL))
    return FALSE;

  node->use_count++;

  switch(node->type) {
  case MARSHAL_DATUM:
    if(node->u.datum_info.datum_size > 1)
      node->flags |= MN_ENDIAN_DEPENDANT;
    break;
  case MARSHAL_SET:
    for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp))
      btmp = oidl_pass_set_endian_dependant(ltmp->data) || btmp;
    if(btmp)
      node->flags |= MN_ENDIAN_DEPENDANT;
    break;
  case MARSHAL_LOOP:
    btmp = oidl_pass_set_endian_dependant(node->u.loop_info.loop_var) || btmp;
    btmp = oidl_pass_set_endian_dependant(node->u.loop_info.length_var) || btmp;
    btmp = oidl_pass_set_endian_dependant(node->u.loop_info.contents) || btmp;
    if(btmp)
      node->flags |= MN_ENDIAN_DEPENDANT;
    break;
  case MARSHAL_SWITCH:
    {
      GSList *ltmp;
      btmp = oidl_pass_set_endian_dependant(node->u.switch_info.discrim);

      for(ltmp = node->u.switch_info.cases; ltmp; ltmp = g_slist_next(ltmp))
	btmp = oidl_pass_set_endian_dependant(ltmp->data) && btmp;

      if(btmp)
	node->flags |= MN_ENDIAN_DEPENDANT;
    }    
    break;
  case MARSHAL_CASE:
    if(oidl_pass_set_endian_dependant(node->u.case_info.contents))
      node->flags |= MN_ENDIAN_DEPENDANT;
    break;
  default:
    break;
  }

  node->use_count--;

  return (node->flags & MN_ENDIAN_DEPENDANT)?TRUE:FALSE;
}

static void
orbit_idl_collapse_sets(OIDL_Marshal_Node *node)
{
  GSList *ltmp;
  OIDL_Marshal_Node *ntmp;

  if(!node) return;

#define SET_SIZE(node) g_slist_length(((OIDL_Marshal_Node *)node)->u.set_info.subnodes)

#define COLLAPSE_NODE(anode) \
if(!(anode)->name \
   && (anode)->type == MARSHAL_SET \
   && SET_SIZE((anode)) == 1) { \
(anode) = (anode)->u.set_info.subnodes->data; \
(anode)->up = node; \
g_message("Successful collapse of %s", (anode)->name); \
}

  switch(node->type) {
  case MARSHAL_SET:
    for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp)) {
      ntmp = ltmp->data;
      orbit_idl_collapse_sets(ntmp);
      if(!ntmp->name
	 && ntmp->type == MARSHAL_SET
	 && SET_SIZE(ntmp) == 1) {
	ltmp->data = ntmp->u.set_info.subnodes->data;
	ntmp = ltmp->data;
	ntmp->up = node;
	g_message("Successful collapse of %s", ntmp->name);
      }
    }
    break;
  case MARSHAL_LOOP:
    orbit_idl_collapse_sets(node->u.loop_info.loop_var);
    COLLAPSE_NODE(node->u.loop_info.loop_var);
    orbit_idl_collapse_sets(node->u.loop_info.length_var);
    COLLAPSE_NODE(node->u.loop_info.length_var);
    orbit_idl_collapse_sets(node->u.loop_info.contents);
    COLLAPSE_NODE(node->u.loop_info.contents);
    break;
  case MARSHAL_SWITCH:
    orbit_idl_collapse_sets(node->u.switch_info.discrim);
    COLLAPSE_NODE(node->u.switch_info.discrim);
    g_slist_foreach(node->u.switch_info.cases, (GFunc)orbit_idl_collapse_sets, NULL);
    break;
  case MARSHAL_CASE:
    orbit_idl_collapse_sets(node->u.case_info.contents);
    COLLAPSE_NODE(node->u.case_info.contents);
    break;
  default:
    break;
  }
}

static void
oidl_pass_make_updates(OIDL_Marshal_Node *node)
{
  OIDL_Marshal_Node *sub;

  if(!node) return;
  if(node->use_count)
    return;

  if(node->flags & MN_NOMARSHAL) return;

  node->use_count++;

  switch(node->type) {
  case MARSHAL_DATUM:
    node->flags |= MN_DEMARSHAL_UPDATE_AFTER;
    break;
  case MARSHAL_LOOP:
    oidl_pass_make_updates(node->u.loop_info.loop_var);
    oidl_pass_make_updates(node->u.loop_info.length_var);
    oidl_pass_make_updates(node->u.loop_info.contents);
    node->flags |= MN_DEMARSHAL_UPDATE_AFTER;
    break;
  case MARSHAL_SET:
    {
      GSList *ltmp;

      for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp)) {
	sub = ltmp->data;
	oidl_pass_make_updates(sub);
      }

      node->flags |= MN_DEMARSHAL_UPDATE_AFTER;
    }
    break;
  case MARSHAL_SWITCH:
    oidl_pass_make_updates(node->u.switch_info.discrim);
    g_slist_foreach(node->u.switch_info.cases, (GFunc)oidl_pass_make_updates, NULL);
    break;
  case MARSHAL_CASE:
    g_slist_foreach(node->u.case_info.labels, (GFunc)oidl_pass_make_updates, NULL);
    oidl_pass_make_updates(node->u.case_info.contents);
    break;
  case MARSHAL_COMPLEX:
    break;
  default:
    break;
  }
  node->use_count--;
}

static void
oidl_pass_del_tail_update(OIDL_Marshal_Node *node)
{
  if(!node) return;

  if(node->use_count)
    return;

  if(node->flags & MN_NOMARSHAL) return;

  node->use_count++;

  switch(node->type) {
  case MARSHAL_DATUM:
    node->flags &= ~MN_DEMARSHAL_UPDATE_AFTER;
    break;
  case MARSHAL_LOOP:
    if(node->flags & MN_COALESCABLE)
      node->flags &= ~MN_DEMARSHAL_UPDATE_AFTER;
    break;
  case MARSHAL_SET:
    {
      GSList *ltmp;

      if(node->flags & MN_COALESCABLE)
	node->flags &= ~MN_DEMARSHAL_UPDATE_AFTER;
      ltmp = g_slist_last(node->u.set_info.subnodes);
      if(ltmp)
	oidl_pass_del_tail_update(ltmp->data);
    }
    break;
  case MARSHAL_SWITCH:
    g_slist_foreach(node->u.switch_info.cases, (GFunc)oidl_pass_del_tail_update, NULL);
    break;
  case MARSHAL_CASE:
    oidl_pass_del_tail_update(node->u.case_info.contents);
    break;
  case MARSHAL_COMPLEX:
  default:
    break;
  }

  node->use_count--;
}

static gint
oidl_node_pass_set_first_curptr_usage(OIDL_Marshal_Node *node)
{
  gint retval = 2, itmp;
  GSList *ltmp;

  if(!node)
    goto out;

  if(node->use_count)
    goto out;

  if(node->flags & MN_NOMARSHAL)
    goto out;

  node->use_count++;

  switch(node->type) {
  case MARSHAL_COMPLEX:
    retval = 0;
    break;
  case MARSHAL_DATUM:
    retval = 1;
    break;
  case MARSHAL_LOOP:
    g_assert(node->u.loop_info.loop_var->flags & MN_NOMARSHAL);
    retval = oidl_node_pass_set_first_curptr_usage(node->u.loop_info.length_var);
    itmp = oidl_node_pass_set_first_curptr_usage(node->u.loop_info.contents);
    if(retval == 2)
      retval = itmp;
    break;
  case MARSHAL_SET:
    for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = ltmp->next)
      {
	itmp = oidl_node_pass_set_first_curptr_usage(ltmp->data);
	if(retval == 2)
	  retval = itmp;
      }
    break;
  case MARSHAL_SWITCH:
    retval = oidl_node_pass_set_first_curptr_usage(node->u.switch_info.discrim);
    for(ltmp = node->u.switch_info.cases; ltmp; ltmp = g_slist_next(ltmp))
      itmp = oidl_node_pass_set_first_curptr_usage(ltmp->data);
    break;
  default:
    g_assert_not_reached();
    break;
  }

  node->use_count--;

  switch(retval)
    {
    case 0:
      node->flags |= MN_NEED_CURPTR_RECVBUF;
      break;
    case 1:
      node->flags |= MN_NEED_CURPTR_LOCAL;
      break;
    default:
      break;
    }

 out:
  return retval;
}

static void
oidl_pass_set_first_curptr_usage(OIDL_Marshal_Node *node)
{
  oidl_node_pass_set_first_curptr_usage(node);
}

static void
oidl_pass_choose_where_sub_held(OIDL_Marshal_Node *node)
{
  if(node->flags & MN_NOMARSHAL)
    return;

  switch(node->type)
    {
    case MARSHAL_LOOP:
      if(((node->flags & (MN_ISSTRING|MN_COALESCABLE|MN_WIDESTRING)) == (MN_ISSTRING|MN_COALESCABLE))
	 && (node->where & MW_Msg))
	{
	  node->where = MW_Msg;
	  return;
	}
      if(((node->flags & (MN_ISSTRING|MN_COALESCABLE|MN_WIDESTRING)) == (MN_ISSTRING|MN_COALESCABLE|MN_WIDESTRING))
	 && (node->where & MW_Alloca))
	{
	  node->where = MW_Alloca;
	  return;
	}

    default:
      node->where = MW_Null;
      break;
    }
}

static void
oidl_node_pass_choose_where(OIDL_Marshal_Node *node)
{
  OIDL_Marshal_Node *sub;
  GSList *ltmp;

  if(!node) return;
  if(node->use_count)
    return;

  if(node->flags & MN_NOMARSHAL)
    {
      node->where = MW_Auto;
      return;
    }

  node->use_count++;

  switch(node->type) {
  case MARSHAL_LOOP:
    oidl_node_pass_choose_where(node->u.loop_info.loop_var);
    oidl_node_pass_choose_where(node->u.loop_info.length_var);
    oidl_node_pass_choose_where(node->u.loop_info.contents);
    oidl_pass_choose_where_sub_held(node->u.loop_info.loop_var);
    oidl_pass_choose_where_sub_held(node->u.loop_info.length_var);
    if((node->u.loop_info.contents->flags & MN_COALESCABLE)
       && (node->u.loop_info.contents->where & MW_Msg)
       && !(node->flags & MN_WIDESTRING))
      node->u.loop_info.contents->where = MW_Msg;
    else if(!(node->flags & (MN_ISSEQ|MN_ISSTRING))
	    && (node->u.loop_info.contents->where & MW_Null))
      node->u.loop_info.contents->where = MW_Null;
    else if((node->u.loop_info.contents->where & MW_Alloca))
      node->u.loop_info.contents->where = MW_Alloca;
    else
      node->u.loop_info.contents->where = MW_Heap;
    break;
  case MARSHAL_SET:
    {
      for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp)) {
	sub = ltmp->data;
	oidl_node_pass_choose_where(sub);
	if(node->flags & MN_TOPLEVEL)
	  {
	    switch(sub->type)
	      {
	      case MARSHAL_SWITCH:
	      case MARSHAL_SET:
		if((sub->flags & MN_COALESCABLE)
		   && (sub->where & MW_Msg))
		  sub->where = MW_Msg;
		else if(sub->where & MW_Auto)
		  sub->where = MW_Auto;
		else if(sub->flags & MN_PARAM_INOUT)
		  sub->where = MW_Null;
		else
		  sub->where = MW_Heap;
		break;
	      case MARSHAL_LOOP:
		if((sub->where & MW_Msg)
		   && ((sub->flags & (MN_ISSEQ|MN_COALESCABLE|MN_WIDESTRING)) == MN_COALESCABLE))
		  {
		    /* We can only use MW_Msg for strings and coalescable arrays */
		    sub->where = MW_Msg;
		  }
		else if(sub->where & MW_Auto)
		  sub->where = MW_Auto;
		else if((sub->flags & MN_PARAM_INOUT)
			&& (sub->flags & MN_ISSEQ))
		  sub->where = MW_Null;
		else
		  sub->where = MW_Heap;
		break;
	      case MARSHAL_COMPLEX:
	      case MARSHAL_DATUM:
		if((sub->where & MW_Auto)
		   && !(sub->flags & MN_PARAM_INOUT))
		  sub->where = MW_Auto;
		else
		  sub->where = MW_Null;
		break;
	      default:
		g_assert_not_reached();
		break;
	      }
	  }
	else
	  oidl_pass_choose_where_sub_held(sub);
      }
    }
    break;
  case MARSHAL_SWITCH:
    oidl_node_pass_choose_where(node->u.switch_info.discrim);
    oidl_pass_choose_where_sub_held(node->u.switch_info.discrim);
    g_slist_foreach(node->u.switch_info.cases, (GFunc)oidl_node_pass_choose_where, NULL);
    g_slist_foreach(node->u.switch_info.cases, (GFunc)oidl_pass_choose_where_sub_held, NULL);
    break;
  case MARSHAL_CASE:
    g_slist_foreach(node->u.case_info.labels, (GFunc)oidl_node_pass_choose_where, NULL);
    oidl_node_pass_choose_where(node->u.case_info.contents);
    oidl_pass_choose_where_sub_held(node->u.case_info.contents);
    break;
  case MARSHAL_COMPLEX:
    if(node->u.complex_info.type == CX_MARSHAL_METHOD)
      {
	if(node->where & MW_Auto)
	  node->where = MW_Auto;
	else if(node->where & MW_Alloca)
	  node->where = MW_Alloca;
	else if(node->where & MW_Heap)
	  node->where = MW_Heap;
      }
    break;
  case MARSHAL_DATUM:
  case MARSHAL_CONST:
  default:
    break;
  }
  node->use_count--;
}

static void
oidl_pass_choose_where(OIDL_Marshal_Node *node)
{
  oidl_node_pass_choose_where(node);
  if(node->name)
    {
      /* handle the very top level node when it matters */
      if(node->where & MW_Heap)
	node->where = MW_Heap;
      else
	node->where = MW_Null;
    }
}

typedef struct {
  OIDL_Marshal_Length curlen;
  OIDL_Marshal_Node *run_start; /* where we are going to do the length check when we figure out how long it is */
  gboolean is_post;
  int tail_align;
} OIDL_Check_Bounds_Info;

static void
oidl_bounds_check_push(OIDL_Marshal_Node *node, OIDL_Check_Bounds_Info *info)
{
  if(info->curlen.len > 0
     || info->curlen.mult_expr)
    {
      OIDL_Marshal_Length *ptr;

      ptr = g_new(OIDL_Marshal_Length, 1);
      *ptr = info->curlen;

      if(info->is_post)
	info->run_start->post = ptr;
      else
	info->run_start->pre = ptr;
      info->curlen.mult_const = NULL;
      info->curlen.mult_expr = NULL;
      info->curlen.len = 0;
      info->is_post = FALSE;
    }

  info->run_start = node;
}

static int oidl_pass_check_bounds_2(OIDL_Marshal_Node *node, gint tail_align);

static void
oidl_node_pass_check_bounds(OIDL_Marshal_Node *node, OIDL_Check_Bounds_Info *info)
{
  int sub_tail_align;
  GSList *ltmp;

  if(node->flags & MN_NOMARSHAL)
    return;

  if(!info->run_start)
    info->run_start = node;

  if(node->iiop_head_align > info->tail_align)
    oidl_bounds_check_push(node, info);

  switch(node->type)
    {
    case MARSHAL_DATUM:
      info->curlen.len += node->u.datum_info.datum_size;
      break;
    case MARSHAL_LOOP:
      if(node->u.loop_info.contents->flags & MN_COALESCABLE)
	{
	  char *ctmp_contents;

	  oidl_node_pass_check_bounds(node->u.loop_info.length_var, info);

	  oidl_bounds_check_push(node->u.loop_info.length_var, info);
	  info->is_post = TRUE;

	  info->curlen.mult_expr = node->u.loop_info.length_var;

	  ctmp_contents = oidl_marshal_node_valuestr(node->u.loop_info.contents);
	  info->curlen.mult_const = g_strdup_printf("sizeof(%s)", ctmp_contents);
	  g_free(ctmp_contents);

	  oidl_bounds_check_push(NULL, info);
	  info->tail_align = node->u.loop_info.contents->iiop_tail_align;
	}
      else
	{
	  oidl_node_pass_check_bounds(node->u.loop_info.length_var, info);
	  oidl_bounds_check_push(NULL, info);
	  info->tail_align = oidl_pass_check_bounds_2(node->u.loop_info.contents,
						      MIN(info->tail_align, node->u.loop_info.contents->iiop_tail_align));
	}
      break;
    case MARSHAL_SWITCH:
      oidl_node_pass_check_bounds(node->u.switch_info.discrim, info);
      oidl_bounds_check_push(NULL, info);
      sub_tail_align = 9999;
      for(ltmp = node->u.switch_info.cases; ltmp; ltmp = ltmp->next)
	{
	  int n;
	  n = oidl_pass_check_bounds_2(ltmp->data, info->tail_align);
	  sub_tail_align = MIN(n, sub_tail_align);
	}
      info->tail_align = sub_tail_align;
      break;
    case MARSHAL_CASE:
      oidl_node_pass_check_bounds(node->u.case_info.contents, info);
      break;
    case MARSHAL_COMPLEX:
      oidl_bounds_check_push(NULL /* We don't want the check to go BEFORE this node... */, info);
      info->tail_align = 1;
      break;
    case MARSHAL_SET:
      g_slist_foreach(node->u.set_info.subnodes, (GFunc)oidl_node_pass_check_bounds, info);
      break;
    case MARSHAL_CONST:
      break;
    default:
      g_assert_not_reached();
      break;
    }
}

static int
oidl_pass_check_bounds_2(OIDL_Marshal_Node *node, gint tail_align)
{
  OIDL_Check_Bounds_Info info = {{0,NULL}, NULL, FALSE};

  info.tail_align = tail_align;
  info.run_start = node;
  oidl_node_pass_check_bounds(node, &info);
  oidl_bounds_check_push(node, &info);
  return info.tail_align;
}

static void
oidl_pass_check_bounds(OIDL_Marshal_Node *node)
{
  oidl_pass_check_bounds_2(node, 1);
}
