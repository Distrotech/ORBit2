#include "config.h"

#include <string.h>

#include "orbit-idl-c-backend.h"

static void c_demarshal_generate(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_datum(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_loop(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_switch(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_complex(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_set(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_validate_curptr(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_alignfor(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_generate_alloc(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_generate_check(OIDL_Marshal_Length *len, OIDL_C_Marshal_Info *cmi);


void
c_demarshalling_generate(OIDL_Marshal_Node *node, OIDL_C_Info *ci, gboolean in_skels, gboolean subfunc)
{
  OIDL_C_Marshal_Info cmi;

  cmi.ci = ci;
  cmi.last_tail_align = 1;
  cmi.endian_swap_pass = TRUE;
  cmi.in_skels = in_skels?1:0;
  cmi.subfunc = subfunc;

  cmi.marshal_error_exit = "goto _ORBIT_demarshal_error";
  cmi.curptr_in_local = FALSE;
  cmi.alloc_on_stack = in_skels;
  if(in_skels)
    cmi.orb_name = "ORBIT_SERVANT_TO_ORB(_ORBIT_servant)";
  else
    cmi.orb_name = "_ORBIT_recv_buffer->connection->orb_data";

  if(node->flags & MN_ENDIAN_DEPENDANT)
    {
      gboolean start_in_local;
      OIDL_Marshal_Length *len;

      len = node->pre;
      c_demarshal_validate_curptr(node, &cmi);
      start_in_local = cmi.curptr_in_local;
      c_demarshal_alignfor(node, &cmi);
      cmi.last_tail_align = node->iiop_head_align; /* We already did the alignment outside of the 'if' */
      if(len)
	{
	  c_demarshal_generate_check(len, &cmi);
	  node->pre = NULL;
	}

      fprintf(ci->fh, "if(giop_msg_conversion_needed(_ORBIT_recv_buffer)) {\n");
      c_demarshal_generate(node, &cmi);
      fprintf(ci->fh, "} else {\n");
      cmi.last_tail_align = node->iiop_head_align; /* We already did the alignment outside of the 'if' */
      cmi.endian_swap_pass = FALSE;
      cmi.curptr_in_local = start_in_local;
      c_demarshal_generate(node, &cmi);
      fprintf(ci->fh, "}\n");
      node->pre = len;
    }
  else
    {
      cmi.last_tail_align = 1;
      cmi.endian_swap_pass = FALSE;
      cmi.curptr_in_local = FALSE;
      c_demarshal_generate(node, &cmi);
    }
}

static void
c_demarshal_generate_heap_alloc(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp, *ctmp2;

  if(!(node->flags & MN_ISSTRING)) /* Strings get handled specially */
    {
      g_assert(node->nptrs > 0);

      ctmp2 = orbit_cbe_get_typespec_str(node->tree);
      node->nptrs--;
      ctmp = oidl_marshal_node_valuestr(node);
      node->nptrs++;
      fprintf(cmi->ci->fh, "%s = %s__alloc();\n", ctmp, ctmp2);
    }

}

static void
c_demarshal_generate_alloca_alloc(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp, *ctmp2;

  if(!(node->flags & MN_ISSTRING))
    {
      g_assert(node->nptrs > 0);

      ctmp2 = orbit_cbe_get_typespec_str(node->tree);
      node->nptrs--;
      ctmp = oidl_marshal_node_valuestr(node);
      node->nptrs++;
      fprintf(cmi->ci->fh, "%s = g_alloca(sizeof(%s));\n", ctmp, ctmp2);
    }
}

static void
c_demarshal_generate_alloc(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  if(!node->name)
    return;

  switch(node->where)
    {
    case MW_Heap:
      c_demarshal_generate_heap_alloc(node, cmi);
      break;
    case MW_Alloca:
      c_demarshal_generate_alloca_alloc(node, cmi);
      break;
    default:
      break;
    }
}

static void
c_demarshal_generate_check(OIDL_Marshal_Length *len, OIDL_C_Marshal_Info *cmi)
{
  char *curptr_name;
  char len_str[1024];
  gboolean is_const = TRUE;

  if(!len)
    return;

  if(len->mult_expr)
    {
      char *fqn = oidl_marshal_node_valuestr(len->mult_expr);

      is_const = FALSE;
      g_snprintf(len_str, sizeof(len_str), "%d + %s*%s", len->len, len->mult_const, fqn);
      g_free(fqn);
    }
  else
    g_snprintf(len_str, sizeof(len_str), "%d", len->len);
  curptr_name = cmi->curptr_in_local?"_ORBIT_curptr":"_ORBIT_recv_buffer->cur";
  fprintf(cmi->ci->fh, "{\nregister const long _ORBIT_checklen = %s;\n", len_str);
  if(is_const)
    fprintf(cmi->ci->fh, "if((%s + %s) > _ORBIT_buf_end)\n",
	    curptr_name, "_ORBIT_checklen");
  else
    fprintf(cmi->ci->fh, "if((%s + %s) < %s || (%s + %s) > _ORBIT_buf_end)\n", curptr_name, "_ORBIT_checklen", curptr_name,
	    curptr_name, "_ORBIT_checklen");
  fprintf(cmi->ci->fh, "%s;\n", cmi->marshal_error_exit);
  fprintf(cmi->ci->fh, "}\n");
}

static void
c_demarshal_generate(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  if(!node) return;

  if(node->flags & MN_NOMARSHAL) return;
  if(node->use_count) return;

  node->use_count++;

  if(!cmi->curptr_in_local && node->flags & MN_NEED_CURPTR_LOCAL)
    c_demarshal_validate_curptr(node, cmi); /* Help out c_demarshal_generate_check, which will prefer to use the local var */

  c_demarshal_generate_check(node->pre, cmi);

  /* This needs to be done _after_ generate_check because we don't
     want to free the data for a parameter if it cannot possibly be
     demarshalled - the marshal_error_exit label points to stuff that
     knows how to free the demarshalled version of that parameter */
  if(node->marshal_error_exit)
    cmi->marshal_error_exit = node->marshal_error_exit;

  if(node->flags & MN_TOPLEVEL)
    c_demarshal_generate_alloc(node, cmi);

  c_demarshal_validate_curptr(node, cmi);

  switch(node->type) {
  case MARSHAL_DATUM:
    c_demarshal_datum(node, cmi);
    break;
  case MARSHAL_LOOP:
    c_demarshal_loop(node, cmi);
    break;
  case MARSHAL_SWITCH:
    c_demarshal_switch(node, cmi);
    break;
  case MARSHAL_COMPLEX:
    c_demarshal_complex(node, cmi);
    break;
  case MARSHAL_SET:
    c_demarshal_set(node, cmi);
    break;
  default:
    g_assert_not_reached();
    break;
  }

  c_demarshal_generate_check(node->post, cmi);

  node->use_count--;
}

static void
c_demarshal_update_curptr(OIDL_Marshal_Node *node, char *sizestr, OIDL_C_Marshal_Info *cmi)
{
  if((node->flags & MN_DEMARSHAL_UPDATE_AFTER)
     || (node->flags & MN_LOOPED))
    fprintf(cmi->ci->fh, "_ORBIT_curptr += %s;\n", sizestr);
}

static void
c_demarshal_validate_curptr(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  gboolean desired_curptr_in_local_state;

  if(node->flags & MN_NEED_CURPTR_LOCAL)
    desired_curptr_in_local_state = TRUE;
  else if(node->flags & MN_NEED_CURPTR_RECVBUF)
    desired_curptr_in_local_state = FALSE;
  else
    return;

  if(desired_curptr_in_local_state != cmi->curptr_in_local)
    {
      if(desired_curptr_in_local_state)
	fprintf(cmi->ci->fh, "_ORBIT_curptr = _ORBIT_recv_buffer->cur;\n");
      else
	fprintf(cmi->ci->fh, "_ORBIT_recv_buffer->cur = _ORBIT_curptr;\n");

      cmi->curptr_in_local = desired_curptr_in_local_state;
    }
}

static void
c_demarshal_alignfor(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  /* do we need to generate an alignment space? */
  if(node->iiop_head_align > cmi->last_tail_align)
    {
      fprintf(cmi->ci->fh, "%s = ALIGN_ADDRESS(_ORBIT_curptr, %d);\n",
	      cmi->curptr_in_local?"_ORBIT_curptr":"_ORBIT_recv_buffer->cur",
	      node->iiop_head_align);
    }

  cmi->last_tail_align = node->iiop_tail_align;
}

static void
c_demarshal_datum(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp;

  ctmp = oidl_marshal_node_valuestr(node);

  c_demarshal_alignfor(node, cmi);

  if(cmi->endian_swap_pass
     && (node->flags & MN_ENDIAN_DEPENDANT)) {
    int n;

    n = node->u.datum_info.datum_size * 8;

    if(node->u.datum_info.needs_bswap)
      fprintf(cmi->ci->fh, "giop_byteswap((guchar *)&(%s), _ORBIT_curptr, %d);\n", ctmp, node->u.datum_info.datum_size);
    else {
      fprintf(cmi->ci->fh, "(*((guint%d *)&(%s))) = ", n, ctmp);
      fprintf(cmi->ci->fh, "GUINT%d_SWAP_LE_BE(*((guint%d *)_ORBIT_curptr));\n",
	      n, n);
    }
  } else {
    fprintf(cmi->ci->fh, "%s = *((", ctmp);
    orbit_cbe_write_node_typespec(cmi->ci->fh, node);
    fprintf(cmi->ci->fh, "*)_ORBIT_curptr);\n");
  }

  {
    char buf[32];
    g_snprintf(buf, sizeof(buf), "%d", node->u.datum_info.datum_size);
    c_demarshal_update_curptr(node, buf, cmi);
  }

  g_free(ctmp);
}

/**
    LOOP is the (de)marshalling equivalent of IDL sequence, array, string,
    and wstring.  In all cases, there is a single underlying type ("subtype").
    Here, we are generating code which demarshals from a variable
    of a given type, which consists of elements of the type's subtype.
    There are 3 issues:
    - Where is the memory allocated?
    - Does the variable data need to be munged, either for endianness,
      format or alignment reasons?
    - Can the elements be agregated?
    All of these questions interact with each other.
**/
static void
c_demarshal_loop(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp, *ctmp_len, *ctmp_loop, *ctmp_contents;
  gboolean loopvar_set = FALSE;

  ctmp = oidl_marshal_node_valuestr(node);
  ctmp_loop = oidl_marshal_node_valuestr(node->u.loop_info.loop_var);
  ctmp_len = oidl_marshal_node_valuestr(node->u.loop_info.length_var);
  ctmp_contents = oidl_marshal_node_valuestr(node->u.loop_info.contents);

  c_demarshal_generate(node->u.loop_info.length_var, cmi);
  if(node->flags & MN_WIDESTRING)
    fprintf(cmi->ci->fh, "%s /= sizeof(CORBA_wchar);\n", ctmp_len);

  if(cmi->alloc_on_stack && (node->u.loop_info.contents->where == MW_Alloca)) {
    fprintf(cmi->ci->fh, "%s%s = g_alloca(sizeof(%s) * (%s%s));\n", ctmp, (node->flags & MN_ISSEQ)?"._buffer":"",
	    ctmp_contents, ctmp_len, (node->flags & MN_WIDESTRING)?"+1":"");
    if(node->flags & MN_ISSEQ)
      fprintf(cmi->ci->fh, "%s._release = CORBA_FALSE;\n", ctmp);
  } else if(node->u.loop_info.contents->where == MW_Msg
	    || node->where == MW_Null) {
    /* No allocation necessary */
  } else if(node->u.loop_info.contents->where == MW_Heap) {
    char *tname, *tcname;
    
    tname = orbit_cbe_get_typespec_str(node->tree);
    tcname = orbit_cbe_get_typespec_str(node->u.loop_info.contents->tree);

    if(node->flags & MN_ISSEQ) {
      IDL_tree seq = orbit_cbe_get_typespec(node->tree);
      if(orbit_cbe_type_is_builtin(node->u.loop_info.contents->tree))
	fprintf(cmi->ci->fh, "%s._buffer = CORBA_sequence_%s_allocbuf(%s);\n", ctmp, tcname+strlen("CORBA_"), ctmp_len);
      else {
	if(IDL_TYPE_SEQUENCE(seq).positive_int_const) {
	  /* bounded */
	  fprintf(cmi->ci->fh,"%s._maximum = ",ctmp);
	  orbit_cbe_write_const(cmi->ci->fh, IDL_TYPE_SEQUENCE(seq).positive_int_const);
	  fprintf(cmi->ci->fh,";\n");
	  fprintf(cmi->ci->fh, "%s._buffer = CORBA_sequence_%s_allocbuf(%s._maximum);\n", ctmp, tcname, ctmp);
	} else {
	  /* unbounded */
	  fprintf(cmi->ci->fh, "%s._maximum = %s._length;\n",ctmp,ctmp);
	  fprintf(cmi->ci->fh, "%s._buffer = CORBA_sequence_%s_allocbuf(%s);\n", ctmp, tcname, ctmp_len);
	}
      }
      fprintf(cmi->ci->fh, "%s._release = CORBA_TRUE;\n", ctmp);
    } else if(node->flags & MN_ISSTRING)
      fprintf(cmi->ci->fh, "%s = CORBA_%sstring_alloc(%s%s);\n", ctmp, (node->flags & MN_WIDESTRING)?"w":"",
	      ctmp_len,
	      (node->flags & MN_WIDESTRING)?"+1":"");
    g_free(tcname);
    g_free(tname);
  } else
    g_error("Don't know how to allocate node %p", node);

  if((!cmi->endian_swap_pass || !(node->u.loop_info.contents->flags & MN_ENDIAN_DEPENDANT))
     && (node->u.loop_info.contents->flags & MN_COALESCABLE)) {
    GString *tmpstr, *tmpstr2;

    c_demarshal_alignfor(node->u.loop_info.contents, cmi);

    tmpstr = g_string_new(NULL);
    tmpstr2 = g_string_new(NULL);
    g_string_sprintf(tmpstr, "sizeof(%s) * %s", ctmp_contents, ctmp_len);
    /* XXX badhack - what if 'node' is a pointer thingie? Need to find out whether to append '._buffer' or '->_buffer' */
    g_string_sprintf(tmpstr2, "%s%s", ctmp, (node->flags & MN_ISSEQ)?"._buffer":"");

    if(cmi->alloc_on_stack && (node->u.loop_info.contents->where & MW_Msg))
      {
	fprintf(cmi->ci->fh, "*(((gulong*)_ORBIT_curptr)-1) = ORBIT_MEMHOW_NONE;\n");
	fprintf(cmi->ci->fh, "%s = (", tmpstr2->str);
	orbit_cbe_write_typespec(cmi->ci->fh, node->u.loop_info.contents->tree);
	fprintf(cmi->ci->fh, "*)_ORBIT_curptr;\n");
      }
    else
      fprintf(cmi->ci->fh, "memcpy(%s, _ORBIT_curptr, %s);\n", tmpstr2->str, tmpstr->str);

    c_demarshal_update_curptr(node, tmpstr->str, cmi);

    g_string_free(tmpstr2, TRUE);
    g_string_free(tmpstr, TRUE);
  } else {
    cmi->last_tail_align = MIN(cmi->last_tail_align, node->u.loop_info.contents->iiop_tail_align);
    c_demarshal_validate_curptr(node->u.loop_info.contents, cmi); /* We must explicitly do this here,
								     otherwise bad things will happen -
								     it will keep updating local from recvbuf or vice versa,
								     inside the loop, and that is Very Bad(tm) */
    fprintf(cmi->ci->fh, "for(%s = 0; %s < %s; %s++) {\n", ctmp_loop, ctmp_loop, ctmp_len, ctmp_loop);
    /* XXX: what does the next line (loop_var) do? Anything useful? */
    /* c_demarshal_generate(node->u.loop_info.loop_var, cmi); */
    g_assert(node->u.loop_info.loop_var->flags & MN_NOMARSHAL);
    /* this next line does the element-by-element work! */
    c_demarshal_generate(node->u.loop_info.contents, cmi);
    fprintf(cmi->ci->fh, "}\n\n");
    loopvar_set = TRUE;
  }
  if(node->flags & MN_WIDESTRING)
    {
      /* NUL-terminate it */
      if(!loopvar_set)
	fprintf(cmi->ci->fh, "%s = %s;\n", ctmp_loop, ctmp_len);
      fprintf(cmi->ci->fh, "%s = 0;\n", ctmp_contents);
    }

  g_free(ctmp_contents);
  g_free(ctmp_len);
  g_free(ctmp_loop);
  g_free(ctmp);
}

static void
c_demarshal_switch(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp;
  GSList *ltmp;
  guint8 last_tail_align;
  gboolean need_default;

  c_demarshal_generate(node->u.switch_info.discrim, cmi);

  last_tail_align = cmi->last_tail_align;

  ctmp = oidl_marshal_node_valuestr(node->u.switch_info.discrim);
  fprintf(cmi->ci->fh, "switch(%s) {\n", ctmp);
  g_free(ctmp);

  need_default = TRUE;
  for(ltmp = node->u.switch_info.cases; ltmp; ltmp = g_slist_next(ltmp)) {
    GSList *ltmp2;
    OIDL_Marshal_Node *sub;

    cmi->last_tail_align = last_tail_align;

    sub = ltmp->data;
    g_assert(sub->type == MARSHAL_CASE);
    if(sub->u.case_info.labels) {
      for(ltmp2 = sub->u.case_info.labels; ltmp2; ltmp2 = g_slist_next(ltmp2)) {
	if(ltmp2->data) {
	  fprintf(cmi->ci->fh, "case ");
	  orbit_cbe_write_const_node(cmi->ci->fh, ltmp2->data);
	  fprintf(cmi->ci->fh, ":\n");
	} else {
	  fprintf(cmi->ci->fh, "default:\n");
	  need_default = FALSE;
	}
      }
    } else {
      fprintf(cmi->ci->fh, "default:\n");
      need_default = FALSE;
    }
    c_demarshal_generate(sub->u.case_info.contents, cmi);
    fprintf(cmi->ci->fh, "break;\n");
  }
  if(need_default) {
    fprintf(cmi->ci->fh, "default:\nbreak;\n");
  }
  fprintf(cmi->ci->fh, "}\n");

  cmi->last_tail_align = node->iiop_tail_align;
}

static void
c_demarshal_complex(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp;
  const char *do_dup;

  ctmp = oidl_marshal_node_valuestr(node);

  if(cmi->subfunc)
    do_dup = "do_dup";
  else
    do_dup = (cmi->alloc_on_stack && (node->where & (MW_Alloca|MW_Msg)))?"CORBA_FALSE":"CORBA_TRUE";

  switch(node->u.complex_info.type) {
  case CX_CORBA_FIXED:
    g_error("Don't know how to demarshal a CORBA_fixed yet.");
    break;
  case CX_CORBA_ANY:
    fprintf(cmi->ci->fh, "if(ORBit_demarshal_any(_ORBIT_recv_buffer, &(%s), %s, %s))\n%s;\n",
	    ctmp, do_dup, cmi->orb_name, cmi->marshal_error_exit);
    break;
  case CX_CORBA_OBJECT:
    fprintf(cmi->ci->fh, "if(ORBit_demarshal_object(&(%s), _ORBIT_recv_buffer, %s))\n%s;\n",
	    ctmp, cmi->orb_name, cmi->marshal_error_exit);
    break;
  case CX_CORBA_TYPECODE:
    fprintf(cmi->ci->fh, "if(ORBit_decode_CORBA_TypeCode(&%s, _ORBIT_recv_buffer))\n%s;\n", ctmp,
	    cmi->marshal_error_exit);
    break;
  case CX_CORBA_CONTEXT:
    if(cmi->in_skels)
      fprintf(cmi->ci->fh, "if(ORBit_Context_demarshal(NULL, &_ctx, _ORBIT_recv_buffer))\n%s;\n",
	      cmi->marshal_error_exit);
    break;
  case CX_NATIVE:
    g_error("Don't know how to demarshal a NATIVE yet.");
    break;
  case CX_MARSHAL_METHOD:
    {
      OIDL_Type_Marshal_Info *tmi;
      char *ctmp, *ctmp2;

      tmi = oidl_marshal_context_find(cmi->ci->ctxt, node->tree);

      ctmp = oidl_marshal_node_valuestr(node);
      ctmp2 = orbit_cbe_get_typespec_str(node->tree);

      switch(tmi->mtype)
	{
	case MARSHAL_FUNC:
	  fprintf(cmi->ci->fh, "if(%s_demarshal(_ORBIT_recv_buffer, %s(%s), %s, ev))\n%s;\n", ctmp2,
		  (node->flags & MN_ISSLICE)?"":"&",
		  ctmp, do_dup,
		  cmi->marshal_error_exit);
	  break;
	case MARSHAL_ANY:
	  fprintf(cmi->ci->fh, "{ gpointer _valref = &(%s);\n", ctmp);
	  fprintf(cmi->ci->fh, "if(ORBit_TypeCode_demarshal_value(TC_%s, &_valref, _ORBIT_recv_buffer, %s, %s))\n%s;\n",
		  ctmp2, do_dup, cmi->orb_name, cmi->marshal_error_exit);
	  fprintf(cmi->ci->fh, "}\n");
	  break;
	default:
	  g_assert_not_reached();
	  break;
	}
    }    
    break;
  default:
    g_assert_not_reached();
    break;
  }

  g_free(ctmp);
}

static void
c_demarshal_set(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  if((!cmi->endian_swap_pass || !(node->flags & MN_ENDIAN_DEPENDANT))
     && node->name
     && (node->flags & MN_COALESCABLE))
    {
      char *ctmp, *ctmp2;

      ctmp = oidl_marshal_node_valuestr(node);

      c_demarshal_alignfor(node, cmi);

      fprintf(cmi->ci->fh, "memcpy(&(%s), _ORBIT_curptr, sizeof(%s));\n", ctmp, ctmp);
      ctmp2 = g_strdup_printf("sizeof(%s)", ctmp);

      c_demarshal_update_curptr(node, ctmp2, cmi);

      g_free(ctmp2);
      g_free(ctmp);
    }
  else
    {
      /* No need to do this if it is coalescable - obviously will be no need to do it then */
      g_slist_foreach(node->u.set_info.subnodes, (GFunc)c_demarshal_generate_alloc, cmi);

      g_slist_foreach(node->u.set_info.subnodes, (GFunc)c_demarshal_generate, cmi);
    }
}
