#include "config.h"

#include "orbit-idl-c-backend.h"

static void c_marshal_datum(OIDL_Marshal_Node *node, OIDL_C_Info *ci);
static void c_marshal_switch(OIDL_Marshal_Node *node, OIDL_C_Info *ci);
static void c_marshal_loop(OIDL_Marshal_Node *node, OIDL_C_Info *ci);
static void c_marshal_complex(OIDL_Marshal_Node *node, OIDL_C_Info *ci);
static void c_marshal_set(OIDL_Marshal_Node *node, OIDL_C_Info *ci);

void
c_marshal_generate(OIDL_Marshal_Node *node, OIDL_C_Info *ci)
{
  g_return_if_fail(node);
  if(node->flags & MN_NOMARSHAL) return;

  switch(node->type) {
  case MARSHAL_LOOP:
    c_marshal_loop(node, ci);
    break;
  case MARSHAL_SWITCH:
    c_marshal_switch(node, ci);
    break;
  case MARSHAL_DATUM:
    c_marshal_datum(node, ci);
    break;
  case MARSHAL_COMPLEX:
    c_marshal_complex(node, ci);
    break;
  case MARSHAL_SET:
    c_marshal_set(node, ci);
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

#define AP(itemstr, sizestr) c_marshal_append(ci->fh, itemstr, sizestr, FALSE, FALSE)
#define APA(itemstr, sizestr) c_marshal_append(ci->fh, itemstr, sizestr, TRUE, FALSE)

#define API(itemstr, sizestr) c_marshal_append(ci->fh, itemstr, sizestr, FALSE, TRUE)
#define APIA(itemstr, sizestr) c_marshal_append(ci->fh, itemstr, sizestr, TRUE, TRUE)

static void
c_marshal_datum(OIDL_Marshal_Node *node, OIDL_C_Info *ci)
{
  GString *tmpstr;
  char *ctmp;

  tmpstr = g_string_new(NULL);

  ctmp = oidl_marshal_node_fqn(node);
  g_string_sprintf(tmpstr, "sizeof(%s)", ctmp);

  if(node->flags & MN_LOOPED)
    APIA(ctmp, tmpstr->str);
  else
    APA(ctmp, tmpstr->str);

  g_free(ctmp);
  g_string_free(tmpstr, TRUE);
}

static void
c_marshal_switch(OIDL_Marshal_Node *node, OIDL_C_Info *ci)
{
}

static void
c_marshal_loop(OIDL_Marshal_Node *node, OIDL_C_Info *ci)
{
  char *ctmp, *ctmp_len, *ctmp_loop, *ctmp_contents;

  ctmp = oidl_marshal_node_fqn(node);
  ctmp_loop = oidl_marshal_node_fqn(node->u.loop_info.loop_var);
  ctmp_len = oidl_marshal_node_fqn(node->u.loop_info.length_var);
  ctmp_contents = oidl_marshal_node_fqn(node->u.loop_info.contents);

  if(node->flags & MN_ISSTRING)
    fprintf(ci->fh, "%s = strlen(%s) + 1;\n", ctmp_len, ctmp);

  c_marshal_generate(node->u.loop_info.length_var, ci);

  if(oidl_node_aggregatable_p(node->u.loop_info.contents)) {
    GString *tmpstr, *tmpstr2;

    tmpstr = g_string_new(NULL);
    tmpstr2 = g_string_new(NULL);
    g_string_sprintf(tmpstr, "sizeof(%s) * %s", ctmp_contents, ctmp_len);

    /* XXX badhack - what if 'node' is a pointer thingie? Need to find out whether to append '._buffer' or '->_buffer' */
    g_string_sprintf(tmpstr2, "%s%s", ctmp, (node->flags & MN_ISSEQ)?"._buffer":"");

    if(node->flags & MN_LOOPED) /* We use our LOOPED flag, not the contents */
      APIA(tmpstr2->str, tmpstr->str);
    else
      APA(tmpstr2->str, tmpstr->str);

    g_string_free(tmpstr2, TRUE);
    g_string_free(tmpstr, TRUE);
  } else {
    fprintf(ci->fh, "for(%s = 0; %s < %s; %s++) {\n", ctmp_loop, ctmp_loop, ctmp_len, ctmp_loop);
    c_marshal_generate(node->u.loop_info.loop_var, ci);
    c_marshal_generate(node->u.loop_info.contents, ci);
    fprintf(ci->fh, "}\n\n");
  }

  g_free(ctmp_contents);
  g_free(ctmp_len);
  g_free(ctmp_loop);
  g_free(ctmp);
}

static void
c_marshal_complex(OIDL_Marshal_Node *node, OIDL_C_Info *ci)
{
}

static void
c_marshal_set(OIDL_Marshal_Node *node, OIDL_C_Info *ci)
{
  GSList *ltmp;

  for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp))
    c_marshal_generate(ltmp->data, ci);
}
