#include "config.h"
#include "orbit-idl2.h"

typedef void (*OIDL_Pass_Func)(IDL_tree tree, gpointer data);

static void oidl_pass_make_allocs(IDL_tree tree);
static void oidl_pass_make_updates(IDL_tree tree);
static void oidl_pass_tmpvars(IDL_tree tree);
static void oidl_pass_run_for_ops(IDL_tree tree, GFunc func);
static void oidl_pass_set_coalescibility(OIDL_Marshal_Node *node);
static void oidl_pass_set_alignment(OIDL_Marshal_Node *node);

static struct {
  const char *name;
  OIDL_Pass_Func func;
  gpointer data;
} idl_passes[] = {
  {"Allocation", (OIDL_Pass_Func)oidl_pass_make_allocs, NULL},
  {"Position updates", (OIDL_Pass_Func)oidl_pass_make_updates, NULL},
  {"Variable assignment", (OIDL_Pass_Func)oidl_pass_tmpvars, NULL},
  {"Set collapsing", (OIDL_Pass_Func)oidl_pass_run_for_ops, orbit_idl_collapse_sets},
  {"Alignment calculation", (OIDL_Pass_Func)oidl_pass_run_for_ops, oidl_pass_set_alignment},
  {"Coalescibility", (OIDL_Pass_Func)oidl_pass_run_for_ops, oidl_pass_set_coalescibility},
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

static void
oidl_pass_set_alignment_datum(OIDL_Marshal_Node *node)
{
  guint8 itmp;

  if(node->tree) {
    /* find arch alignments */
    switch(IDL_NODE_TYPE(node->tree)) {
    case IDLN_TYPE_INTEGER:
      switch(IDL_TYPE_INTEGER(node->tree).f_type) {
      case IDL_INTEGER_TYPE_SHORT:
#if ALIGNOF_CORBA_SHORT != ALIGNOF_CORBA_UNSIGNED_SHORT
#error "unsigned alignment is different from signed"
#endif
	itmp = ALIGNOF_CORBA_SHORT;
	break;
      case IDL_INTEGER_TYPE_LONG:
#if ALIGNOF_CORBA_LONG != ALIGNOF_CORBA_UNSIGNED_LONG
#error "unsigned alignment is different from signed"
#endif
	itmp = ALIGNOF_CORBA_LONG;
	break;
      case IDL_INTEGER_TYPE_LONGLONG:
#if ALIGNOF_CORBA_LONG_LONG != ALIGNOF_CORBA_UNSIGNED_LONG_LONG
#error "unsigned alignment is different from signed"
#endif
	itmp = ALIGNOF_CORBA_LONG_LONG;
	break;
      default:
	g_error("Weird integer type");
	break;
      }
      break;
    case IDLN_TYPE_FLOAT:
      switch(IDL_TYPE_FLOAT(node->tree).f_type) {
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
    default:
      g_error("Don't know how to get alignment of a %s datum", IDL_tree_type_names[IDL_NODE_TYPE(node->tree)]);
      break;
    }

    node->arch_head_align = node->arch_tail_align = itmp; /* I don't think there's any cases where these aren't equal */
  } else
    node->arch_head_align = node->arch_tail_align = node->u.datum_info.datum_size;

  node->iiop_head_align = node->iiop_tail_align = node->u.datum_info.datum_size;
}

static void
oidl_pass_set_alignment(OIDL_Marshal_Node *node)
{
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
	node->iiop_head_align = node->iiop_tail_align = 1; /* Blah */
    }
    break;
  case MARSHAL_SWITCH:
    oidl_pass_set_alignment(node->u.switch_info.discrim);
    oidl_pass_set_alignment(node->u.switch_info.contents);
    g_error("Union alignment sucketh currently. Fix unions.");
    break;
  case MARSHAL_UPDATE:
    oidl_pass_set_alignment(node->u.update_info.amount);
    node->arch_head_align = node->u.update_info.amount->arch_head_align;
    node->arch_tail_align = node->u.update_info.amount->arch_tail_align;
    node->iiop_head_align = node->u.update_info.amount->iiop_head_align;
    node->iiop_tail_align = node->u.update_info.amount->iiop_tail_align;
    break;
  case MARSHAL_CONST:
    node->arch_head_align = node->arch_tail_align = node->iiop_head_align = node->iiop_tail_align = 1;
    break;
  case MARSHAL_COMPLEX:
    g_error("Complex alignment NYI");
    break;
  case MARSHAL_LOOP:
    oidl_pass_set_alignment(node->u.loop_info.loop_var);
    oidl_pass_set_alignment(node->u.loop_info.length_var);
    oidl_pass_set_alignment(node->u.loop_info.contents);
    break;
  default:
    g_error("Alignment of type %d not known", node->type);
    break;
  }
}

static void
oidl_pass_set_coalescibility(OIDL_Marshal_Node *node)
{
  gboolean elements_ok;
  OIDL_Marshal_Node *sub;

  switch(node->type) {
  case MARSHAL_DATUM:
  case MARSHAL_CONST: /* ??? */
    node->flags |= MN_COALESCABLE;
    break;
  case MARSHAL_LOOP:
    oidl_pass_set_coalescibility(node->u.loop_info.loop_var);
    oidl_pass_set_coalescibility(node->u.loop_info.length_var);
    oidl_pass_set_coalescibility(node->u.loop_info.contents);
    if(node->flags & (MN_ISSEQ|MN_ISSTRING)) break;
    elements_ok = TRUE;
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
      elements_ok = FALSE;

      for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp)) {
	sub = ltmp->data;
	oidl_pass_set_coalescibility(sub);
	if(sub->flags & MN_NOMARSHAL) continue;

	elements_ok = elements_ok || (sub->flags & MN_COALESCABLE);
      }

      if(!elements_ok) break;

      elements_ok = TRUE;
      /* Now figure out alignment stuff */
      for(ltmp = node->u.set_info.subnodes; ltmp && elements_ok; ltmp = g_slist_next(ltmp)) {
	sub = ltmp->data;
	if(sub->flags & MN_NOMARSHAL) continue;

	elements_ok = elements_ok
	  && (sub->iiop_head_align == sub->arch_head_align)
	  && (sub->iiop_tail_align == sub->iiop_tail_align);
      }
      if(!elements_ok) break;

      node->flags |= MN_COALESCABLE;
    }
    break;
  case MARSHAL_SWITCH:
    oidl_pass_set_coalescibility(node->u.switch_info.discrim);
    oidl_pass_set_coalescibility(node->u.switch_info.contents);
    /* Not coalescible, even if children are. */
    break;
  case MARSHAL_UPDATE:
    oidl_pass_set_coalescibility(node->u.update_info.amount);
    if((node->u.update_info.amount->flags & MN_COALESCABLE)
       && !(node->u.update_info.amount->flags & MN_NOMARSHAL))
      node->flags |= MN_COALESCABLE;
    break;
  case MARSHAL_COMPLEX:
  default:
    break;
  }
}
