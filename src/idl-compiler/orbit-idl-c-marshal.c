#include "config.h"

#include "orbit-idl-c-backend.h"

#define NEEDS_INDIRECT(node) (((node)->flags & MN_LOOPED) && ((node)->flags & MN_NEED_TMPVAR))

static void c_marshal_generate(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_datum(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_switch(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_loop(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_complex(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_set(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_alignfor(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);

void
c_marshalling_generate(OIDL_Marshal_Node *node, OIDL_C_Info *ci)
{
  OIDL_C_Marshal_Info cmi;

  cmi.ci = ci;
  cmi.last_tail_align = 1;

  c_marshal_generate(node, &cmi);
}

static void
c_marshal_generate(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  g_return_if_fail(node);
  if(node->flags & MN_NOMARSHAL) return;

  switch(node->type) {
  case MARSHAL_LOOP:
    c_marshal_loop(node, cmi);
    break;
  case MARSHAL_SWITCH:
    c_marshal_switch(node, cmi);
    break;
  case MARSHAL_DATUM:
    c_marshal_datum(node, cmi);
    break;
  case MARSHAL_COMPLEX:
    c_marshal_complex(node, cmi);
    break;
  case MARSHAL_SET:
    c_marshal_set(node, cmi);
    break;
  case MARSHAL_CONST:
  case MARSHAL_UPDATE:
  case MARSHAL_ALLOCATE:
  default:
    g_error("We're supposed to marshal a %d node?", node->type);
    break;
  }
}

static void
c_marshal_append(FILE *of, char *itemstr, char *sizestr, gboolean aligned, gboolean indirect)
{
  if(indirect)
    fprintf(of, "{ gpointer t; t = alloca(%s); memcpy(t, %s, %s);\n", sizestr, itemstr, sizestr);

  fprintf(of, "giop_%s_buffer_append_mem%s%s(GIOP_%s_BUFFER(_ORBIT_send_buffer), %s, %s);\n",
	  indirect?"send":"message",
	  indirect?"_indirect":"",
	  aligned?"_a":"",
	  indirect?"SEND":"MESSAGE",
	  itemstr, sizestr);

  if(indirect)
    fprintf(of, "}\n");
}

#define AP(itemstr, sizestr) c_marshal_append(cmi->ci->fh, itemstr, sizestr, FALSE, FALSE)
#define APA(itemstr, sizestr) c_marshal_append(cmi->ci->fh, itemstr, sizestr, TRUE, FALSE)

#define API(itemstr, sizestr) c_marshal_append(cmi->ci->fh, itemstr, sizestr, FALSE, TRUE)
#define APIA(itemstr, sizestr) c_marshal_append(cmi->ci->fh, itemstr, sizestr, TRUE, TRUE)

static void
c_marshal_alignfor(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  /* do we need to generate an alignment space? */
  if(node->iiop_head_align > cmi->last_tail_align) {
    fprintf(cmi->ci->fh, "giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(_ORBIT_send_buffer), %d);\n",
	    node->iiop_head_align);
  }
  cmi->last_tail_align = node->iiop_tail_align;
}

static void
c_marshal_datum(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  GString *tmpstr;
  char *ctmp;

  c_marshal_alignfor(node, cmi);
  tmpstr = g_string_new(NULL);

  ctmp = oidl_marshal_node_fqn(node);
  g_string_sprintf(tmpstr, "sizeof(%s)", ctmp);

  if(NEEDS_INDIRECT(node))
    API(ctmp, tmpstr->str);
  else
    AP(ctmp, tmpstr->str);

  g_free(ctmp);
  g_string_free(tmpstr, TRUE);
}

static void
c_marshal_switch(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp;
  GSList *ltmp;
  guint8 last_tail_align;

  c_marshal_generate(node->u.switch_info.discrim, cmi);

  last_tail_align = cmi->last_tail_align;

  ctmp = oidl_marshal_node_fqn(node->u.switch_info.discrim);
  fprintf(cmi->ci->fh, "switch(%s) {\n", ctmp);
  g_free(ctmp);

  for(ltmp = node->u.switch_info.cases; ltmp; ltmp = g_slist_next(ltmp)) {
    GSList *ltmp2;
    OIDL_Marshal_Node *sub;

    cmi->last_tail_align = last_tail_align;

    sub = ltmp->data;
    g_assert(sub->type == MARSHAL_CASE);
    for(ltmp2 = sub->u.case_info.labels; ltmp2; ltmp2 = g_slist_next(ltmp2)) {
      fprintf(cmi->ci->fh, "case ");
      orbit_cbe_write_const_node(cmi->ci->fh, ltmp2->data);
      fprintf(cmi->ci->fh, ":\n");
    }
    c_marshal_generate(sub->u.case_info.contents, cmi);
    fprintf(cmi->ci->fh, "break;\n");
  }
  fprintf(cmi->ci->fh, "}\n");

  cmi->last_tail_align = node->iiop_tail_align;
}

static void
c_marshal_loop(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp, *ctmp_len, *ctmp_loop, *ctmp_contents;

  ctmp = oidl_marshal_node_fqn(node);
  ctmp_loop = oidl_marshal_node_fqn(node->u.loop_info.loop_var);
  ctmp_len = oidl_marshal_node_fqn(node->u.loop_info.length_var);
  ctmp_contents = oidl_marshal_node_fqn(node->u.loop_info.contents);

  if(node->flags & MN_ISSTRING)
    fprintf(cmi->ci->fh, "%s = strlen(%s) + 1;\n", ctmp_len, ctmp);

  c_marshal_generate(node->u.loop_info.length_var, cmi);

  if(node->u.loop_info.contents->flags & MN_COALESCABLE) {
    GString *tmpstr, *tmpstr2;

    tmpstr = g_string_new(NULL);
    tmpstr2 = g_string_new(NULL);
    g_string_sprintf(tmpstr, "sizeof(%s) * %s", ctmp_contents, ctmp_len);

    /* XXX badhack - what if 'node' is a pointer thingie? Need to find out whether to append '._buffer' or '->_buffer' */
    g_string_sprintf(tmpstr2, "%s%s", ctmp, (node->flags & MN_ISSEQ)?"._buffer":"");

    c_marshal_alignfor(node->u.loop_info.contents, cmi);

    if(NEEDS_INDIRECT(node->u.loop_info.contents)) /* We use our LOOPED flag, not the contents */
      API(tmpstr2->str, tmpstr->str);
    else
      AP(tmpstr2->str, tmpstr->str);

    g_string_free(tmpstr2, TRUE);
    g_string_free(tmpstr, TRUE);
  } else {
    fprintf(cmi->ci->fh, "for(%s = 0; %s < %s; %s++) {\n", ctmp_loop, ctmp_loop, ctmp_len, ctmp_loop);
    c_marshal_generate(node->u.loop_info.loop_var, cmi);
    c_marshal_generate(node->u.loop_info.contents, cmi);
    fprintf(cmi->ci->fh, "}\n\n");
  }

  g_free(ctmp_contents);
  g_free(ctmp_len);
  g_free(ctmp_loop);
  g_free(ctmp);
}

static void
c_marshal_complex(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  g_error("Complex marshalling NYI");
}

static void
c_marshal_set(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  if(node->flags & MN_COALESCABLE) {
    char *ctmp;
    GString *tmpstr = g_string_new(NULL);

    ctmp = oidl_marshal_node_fqn(node);
    g_string_sprintf(tmpstr, "sizeof(%s)", ctmp);
    if(NEEDS_INDIRECT(node))
      API(ctmp, tmpstr->str);
    else
      AP(ctmp, tmpstr->str);

  } else {
    GSList *ltmp;

    for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp))
      c_marshal_generate(ltmp->data, cmi);
  }
}
