#include "config.h"
#include <orbit/orbit.h>
#include "orb-core-private.h"
#include <string.h>

gint
ORBit_find_alignment(CORBA_TypeCode tc)
{
  gint retval = 1;
  int i;

  switch(tc->kind) {
  case CORBA_tk_union:
    retval = MAX(retval, ORBit_find_alignment(tc->discriminator));
  case CORBA_tk_except:
  case CORBA_tk_struct:
#if ALIGNOF_CORBA_STRUCT > 1
    retval = MAX(retval, ALIGNOF_CORBA_STRUCT);
#endif
    for(i = 0; i < tc->sub_parts; i++)
      retval = MAX(retval, ORBit_find_alignment(tc->subtypes[i]));
    return retval;
  case CORBA_tk_ulong:
  case CORBA_tk_long:
  case CORBA_tk_enum:
    return ALIGNOF_CORBA_LONG;
  case CORBA_tk_ushort:
  case CORBA_tk_short:
  case CORBA_tk_wchar:
    return ALIGNOF_CORBA_SHORT;
  case CORBA_tk_longlong:
  case CORBA_tk_ulonglong:
    return ALIGNOF_CORBA_LONG_LONG;
  case CORBA_tk_longdouble:
    return ALIGNOF_CORBA_LONG_DOUBLE;
  case CORBA_tk_float:
    return ALIGNOF_CORBA_FLOAT;
  case CORBA_tk_double:
    return ALIGNOF_CORBA_DOUBLE;
  case CORBA_tk_boolean:
  case CORBA_tk_char:
  case CORBA_tk_octet:
    return ALIGNOF_CORBA_CHAR;
  case CORBA_tk_string:
  case CORBA_tk_wstring:
  case CORBA_tk_TypeCode:
  case CORBA_tk_objref:
    return ALIGNOF_CORBA_POINTER;
  case CORBA_tk_sequence:
    return ALIGNOF_CORBA_SEQ;
  case CORBA_tk_any:
    return ALIGNOF_CORBA_ANY;
  case CORBA_tk_array:
  case CORBA_tk_alias:
    return ORBit_find_alignment(tc->subtypes[0]);
  case CORBA_tk_fixed:
    return MAX(ALIGNOF_CORBA_SHORT, ALIGNOF_CORBA_STRUCT);
  default:
    return 1;
  }
}

size_t
ORBit_gather_alloc_info(CORBA_TypeCode tc)
{
  int i, n, align=1, prevalign, sum, prev;
  size_t block_size;

  switch(tc->kind) {
  case CORBA_tk_long:
  case CORBA_tk_ulong:
  case CORBA_tk_enum:
    return sizeof(CORBA_long);
    break;
  case CORBA_tk_short:
  case CORBA_tk_ushort:
    return sizeof(CORBA_short);
    break;
  case CORBA_tk_float:
    return sizeof(CORBA_float);
    break;
  case CORBA_tk_double:
    return sizeof(CORBA_double);
    break;
  case CORBA_tk_boolean:
  case CORBA_tk_char:
  case CORBA_tk_octet:
    return sizeof(CORBA_octet);
    break;
  case CORBA_tk_any:
    return sizeof(CORBA_any);
    break;
  case CORBA_tk_TypeCode:
    return sizeof(CORBA_TypeCode);
    break;
  case CORBA_tk_Principal:
    return sizeof(CORBA_Principal);
    break;
  case CORBA_tk_objref:
    return sizeof(CORBA_Object);
    break;
  case CORBA_tk_except:
  case CORBA_tk_struct:
    sum = 0;
    for(i = 0; i < tc->sub_parts; i++) {
      sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, ORBit_find_alignment(tc->subtypes[i])));
      sum += ORBit_gather_alloc_info(tc->subtypes[i]);
    }
    sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, ORBit_find_alignment(tc)));
    return sum;
    break;
  case CORBA_tk_union:
    sum = ORBit_gather_alloc_info(tc->discriminator);
    n = -1;
    align = 1;
    for(prev = prevalign = i = 0; i < tc->sub_parts; i++) {
      prevalign = align;
      align = ORBit_find_alignment(tc->subtypes[i]);
      if(align > prevalign)
	n = i;

      prev = MAX(prev, ORBit_gather_alloc_info(tc->subtypes[i]));
    }
    if(n >= 0)
      sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, ORBit_find_alignment(tc->subtypes[n])));
    sum += prev;
    sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, ORBit_find_alignment(tc)));
    return sum;
    break;
  case CORBA_tk_wstring:
  case CORBA_tk_string:
    return sizeof(char *);
    break;
  case CORBA_tk_sequence:
    return sizeof(CORBA_sequence_CORBA_octet);
    break;
  case CORBA_tk_array:
    block_size = ORBit_gather_alloc_info(tc->subtypes[0]);
    return block_size * tc->length;
    break;
  case CORBA_tk_alias:
    return ORBit_gather_alloc_info(tc->subtypes[0]);
  case CORBA_tk_longlong:
  case CORBA_tk_ulonglong:
    return sizeof(CORBA_long_long);
  case CORBA_tk_longdouble:
    return sizeof(CORBA_long_double);
  case CORBA_tk_wchar:
    return sizeof(CORBA_wchar);
  case CORBA_tk_fixed:
    return sizeof(CORBA_fixed_d_s);
  default:
    return 0;
  }
}

static void
ORBit_marshal_value(GIOPSendBuffer *buf,
		    gconstpointer *val,
		    CORBA_TypeCode tc,
		    ORBit_marshal_value_info *mi)
{
    CORBA_unsigned_long i, ulval;
    gconstpointer subval;
    ORBit_marshal_value_info submi;

    switch(tc->kind) {
    case CORBA_tk_wchar:
    case CORBA_tk_ushort:
    case CORBA_tk_short:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_SHORT);
	giop_send_buffer_align(buf, 2);
	giop_send_buffer_append(buf, *val, sizeof(CORBA_short));
	*val = ((guchar *)*val) + sizeof(CORBA_short);
	break;
    case CORBA_tk_enum:
    case CORBA_tk_long:
    case CORBA_tk_ulong:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG);
	giop_send_buffer_align(buf, 4);
	giop_send_buffer_append(buf, *val, sizeof(CORBA_long));
	*val = ((guchar *)*val) + sizeof(CORBA_long);
	break;
    case CORBA_tk_float:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_FLOAT);
	giop_send_buffer_align(buf, sizeof(CORBA_float));
	giop_send_buffer_append(buf, *val, sizeof(CORBA_float));
	*val = ((guchar *)*val) + sizeof(CORBA_float);
	break;
    case CORBA_tk_double:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_DOUBLE);
	giop_send_buffer_align(buf, sizeof(CORBA_double));
	giop_send_buffer_append(buf, *val, sizeof(CORBA_double));
	*val = ((guchar *)*val) + sizeof(CORBA_double);
	break;
    case CORBA_tk_boolean:
    case CORBA_tk_char:
    case CORBA_tk_octet:
	giop_send_buffer_append(buf, *val, sizeof(CORBA_octet));
	*val = ((guchar *)*val) + sizeof(CORBA_octet);
	break;
    case CORBA_tk_any:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_ANY);
	ORBit_marshal_any(buf, *val);
	*val = ((guchar *)*val) + sizeof(CORBA_any);
	break;
    case CORBA_tk_TypeCode:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	ORBit_encode_CORBA_TypeCode((CORBA_TypeCode)*val, buf);
	*val = ((guchar *)*val) + sizeof(CORBA_TypeCode);
	break;
    case CORBA_tk_Principal:
	*val = ALIGN_ADDRESS(*val,
			     MAX(MAX(ALIGNOF_CORBA_LONG, ALIGNOF_CORBA_STRUCT),
				 ALIGNOF_CORBA_POINTER));

	ulval = *(CORBA_unsigned_long *)(*val);
	giop_send_buffer_append(buf, *val, sizeof(CORBA_unsigned_long));

	giop_send_buffer_append(buf,
				*(char**)((char *)*val+sizeof(CORBA_unsigned_long)),
				ulval);
	*val = ((guchar *)*val) + sizeof(CORBA_Principal);
	break;
    case CORBA_tk_objref:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	ORBit_marshal_object(buf, *(CORBA_Object*)*val);
	*val = ((guchar *)*val) + sizeof(CORBA_Object);
	break;
    case CORBA_tk_except:
    case CORBA_tk_struct:
	*val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc));
	for(i = 0; i < tc->sub_parts; i++)
	    ORBit_marshal_value(buf, val, tc->subtypes[i], mi);
	break;
    case CORBA_tk_union:
	{
	    gconstpointer	discrim, body;
	    CORBA_TypeCode 	subtc;
	    int			al = 1, sz = 0;

	    discrim = *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_STRUCT);
	    ORBit_marshal_value(buf, val, tc->discriminator, mi);
	    subtc = ORBit_get_union_tag(tc, &discrim, FALSE);
	    for (i=0; i < tc->sub_parts; i++) {
	    	al = MAX(al, ORBit_find_alignment(tc->subtypes[i]));
	    	sz = MAX(sz, ORBit_gather_alloc_info(tc->subtypes[i]));
	    }
	    body = *val = ALIGN_ADDRESS(*val, al);
	    ORBit_marshal_value(buf, &body, subtc, mi);
	    /* WATCHOUT: end of subtc may not be end of union */
	    *val = ((guchar*)*val) + sz;
	}
	break;
    case CORBA_tk_wstring:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	ulval = CORBA_wstring_len(*(CORBA_wchar **)*val) + 1;
	giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
	giop_send_buffer_append_indirect(buf,
					 &ulval,
					 sizeof(CORBA_unsigned_long));
	giop_send_buffer_append(buf, *(char **)*val, ulval);
	*val = ((guchar *)*val) + sizeof(char *);
	break;
    case CORBA_tk_string:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	ulval = strlen(*(char **)*val) + 1;
	giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
	giop_send_buffer_append_indirect(buf,
					 &ulval,
					 sizeof(CORBA_unsigned_long));
	giop_send_buffer_append(buf, *(char **)*val, ulval);
	
	*val = ((guchar *)*val) + sizeof(char *);
	break;
    case CORBA_tk_sequence:
	{
	    const CORBA_sequence_CORBA_octet *sval;
	    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_SEQ);
	    sval = *val;
	    giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
	    giop_send_buffer_append(buf, &sval->_length,
				    sizeof(sval->_length));

	    subval = sval->_buffer;

	    for(i = 0; i < sval->_length; i++)
		ORBit_marshal_value(buf, &subval, tc->subtypes[0], mi);
	    
	    *val = ((guchar *)*val) + sizeof(CORBA_sequence_CORBA_octet);
	}
	break;
    case CORBA_tk_array:
	submi.alias_element_type = tc->subtypes[0];
	for(i = 0; i < tc->length; i++) {
	  ORBit_marshal_value(buf, val, submi.alias_element_type, &submi);
	  *val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc->subtypes[0]));
	}
	break;
    case CORBA_tk_alias:
	submi.alias_element_type = tc->subtypes[0];
	ORBit_marshal_value(buf, val, submi.alias_element_type, &submi);
	break;
    case CORBA_tk_longlong:
    case CORBA_tk_ulonglong:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_LONG);
	giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
	giop_send_buffer_append(buf, *val, sizeof(CORBA_long_long));
	return /* *val + sizeof(CORBA_long_long)*/;
	break;
    case CORBA_tk_longdouble:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_DOUBLE);
	giop_send_buffer_align(buf, sizeof(CORBA_long_double));
	giop_send_buffer_append(buf, *val, sizeof(CORBA_long_double));
	return /* *val + sizeof(CORBA_long_double)*/;
	break;
    case CORBA_tk_fixed:
	/* XXX todo */
	g_error("CORBA_fixed NYI");
	
	break;
    case CORBA_tk_null:
    case CORBA_tk_void:
	break;
    default:
	g_error("Can't encode unknown type %d", tc->kind);
    }
}

static glong
ORBit_get_union_switch(CORBA_TypeCode tc, gconstpointer *val, 
		       gboolean update)
{
  glong retval = 0; /* Quiet gcc */

  switch(tc->kind) {
  case CORBA_tk_ulong:
  case CORBA_tk_long:
  case CORBA_tk_enum:
    retval = *(CORBA_long *)*val;
    if(update) *val = ((guchar *)*val) + sizeof(CORBA_long);
    break;
  case CORBA_tk_ushort:
  case CORBA_tk_short:
    retval = *(CORBA_short *)*val;
    if(update) *val = ((guchar *)*val) + sizeof(CORBA_short);
    break;
  case CORBA_tk_char:
  case CORBA_tk_boolean:
  case CORBA_tk_octet:
    retval = *(CORBA_octet *)*val;
    if(update) *val = ((guchar *)*val) + sizeof(CORBA_char);
    break;
  case CORBA_tk_alias:
    return ORBit_get_union_switch(tc->subtypes[0], val, update);
    break;
  default:
    g_error("Wow, some nut has passed us a weird type[%d] as a union discriminator!", tc->kind);
  }

  return retval;
}

/* This function (and the one above it) exist for the
   sole purpose of finding out which CORBA_TypeCode a union discriminator value
   indicates.

   If {update} is TRUE, {*val} will be advanced by the native size
   of the descriminator type.

   Hairy stuff.
*/
CORBA_TypeCode
ORBit_get_union_tag(CORBA_TypeCode union_tc, gconstpointer *val, 
		    gboolean update)
{
  glong discrim_val, case_val;
  int i;
  CORBA_TypeCode retval = CORBA_OBJECT_NIL;

  discrim_val = ORBit_get_union_switch(union_tc->discriminator, val, update);

  for(i = 0; i < union_tc->sub_parts; i++) {
    if(i == union_tc->default_index)
      continue;

    case_val = ORBit_get_union_switch(union_tc->sublabels[i]._type,
				      (gconstpointer*)&union_tc->sublabels[i]._value, FALSE);
    if(case_val == discrim_val) {
      retval = union_tc->subtypes[i];
      break;
    }
  }

  if(retval)
    return retval;
  else if(union_tc->default_index >= 0)
    return union_tc->subtypes[union_tc->default_index];
  else {
    return TC_null;
  }
}

void
ORBit_marshal_arg(GIOPSendBuffer *buf,
		  gconstpointer val,
		  CORBA_TypeCode tc)
{
  ORBit_marshal_value_info mi;

  ORBit_marshal_value(buf, &val, tc, &mi);
}

void
ORBit_marshal_any(GIOPSendBuffer *buf, const CORBA_any *val)
{
  ORBit_marshal_value_info mi;

  gconstpointer mval = val->_value;

  ORBit_encode_CORBA_TypeCode(val->_type, buf);

  ORBit_marshal_value(buf, &mval, val->_type, &mi);
}

gboolean
ORBit_demarshal_value(CORBA_TypeCode tc,
		      gpointer *val,
		      GIOPRecvBuffer *buf,
		      gboolean dup_strings,
		      CORBA_ORB orb)
{
  CORBA_long i;

  switch(tc->kind) {
  case CORBA_tk_short:
  case CORBA_tk_ushort:
  case CORBA_tk_wchar:
    {
      CORBA_unsigned_short *ptr;
      *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_SHORT);
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_short));
      if((buf->cur + sizeof(CORBA_short)) > buf->end)
	return TRUE;
      ptr = *val;
      *ptr = *(CORBA_unsigned_short *)buf->cur;
      if(giop_msg_conversion_needed(buf))
	*ptr = GUINT16_SWAP_LE_BE(*ptr);
      buf->cur += sizeof(CORBA_short);
      *val = ((guchar *)*val) + sizeof(CORBA_short);
    }
    break;
  case CORBA_tk_long:
  case CORBA_tk_ulong:
  case CORBA_tk_enum:
    {
      CORBA_unsigned_long *ptr;
      *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG);
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long));
      if((buf->cur + sizeof(CORBA_long)) > buf->end)
	return TRUE;
      ptr = *val;
      *ptr = *(CORBA_unsigned_long *)buf->cur;
      if(giop_msg_conversion_needed(buf))
	*ptr = GUINT32_SWAP_LE_BE(*ptr);
      buf->cur += sizeof(CORBA_long);
      *val = ((guchar *)*val) + sizeof(CORBA_long);
    }
    break;
  case CORBA_tk_longlong:
  case CORBA_tk_ulonglong:
    {
      CORBA_unsigned_long_long *ptr;
      *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_LONG);
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long_long));
      if((buf->cur + sizeof(CORBA_long_long)) > buf->end)
	return TRUE;
      ptr = *val;
      *ptr = *(CORBA_unsigned_long_long *)buf->cur;
      if(giop_msg_conversion_needed(buf))
	*ptr = GUINT64_SWAP_LE_BE(*ptr);
      buf->cur += sizeof(CORBA_long_long);
      *val = ((guchar *)*val) + sizeof(CORBA_long_long);
    }
    break;
  case CORBA_tk_longdouble:
    {
      CORBA_long_double *ptr;
      *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_DOUBLE);
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long_double));
      if((buf->cur + sizeof(CORBA_long_double)) > buf->end)
	return TRUE;
      ptr = *val;
      if(giop_msg_conversion_needed(buf))
	giop_byteswap(ptr, buf->cur, sizeof(CORBA_long_double));
      else
	*ptr = *(CORBA_long_double *)buf->cur;
      buf->cur += sizeof(CORBA_long_double);
      *val = ((guchar *)*val) + sizeof(CORBA_long_double);
    }
    break;
  case CORBA_tk_float:
    {
      CORBA_float *ptr;
      *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_FLOAT);
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_float));
      if((buf->cur + sizeof(CORBA_float)) > buf->end)
	return TRUE;
      ptr = *val;
      if(giop_msg_conversion_needed(buf))
	giop_byteswap(ptr, buf->cur, sizeof(CORBA_float));
      else
	*ptr = *(CORBA_float *)buf->cur;
      buf->cur += sizeof(CORBA_float);

      *val = ((guchar *)*val) + sizeof(CORBA_float);
    }
    break;
  case CORBA_tk_double:
    {
      CORBA_double *ptr;
      *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_DOUBLE);
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_double));
      if((buf->cur + sizeof(CORBA_double)) > buf->end)
	return TRUE;
      ptr = *val;
      if(giop_msg_conversion_needed(buf))
	giop_byteswap(ptr, buf->cur, sizeof(CORBA_double));
      else
	*ptr = *(CORBA_double *)buf->cur;
      buf->cur += sizeof(CORBA_double);

      *val = ((guchar *)*val) + sizeof(CORBA_double);
    }
    break;
  case CORBA_tk_boolean:
  case CORBA_tk_char:
  case CORBA_tk_octet:
    {
      CORBA_octet *ptr;
      if((buf->cur + sizeof(CORBA_octet)) > buf->end)
	return TRUE;
      ptr = *val;
      *ptr = buf->cur;
      buf->cur++;
      
      *val = ((guchar *)*val) + sizeof(CORBA_octet);
    }
    break;
  case CORBA_tk_any:
    {
      CORBA_any *decoded;

      *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_ANY);
      decoded = *val;
      decoded->_release = CORBA_FALSE;
      if(ORBit_demarshal_any(buf, decoded, dup_strings, orb))
	return TRUE;
      *val = ((guchar *)*val) + sizeof(CORBA_any);
    }
    break;
  case CORBA_tk_TypeCode:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
    if(ORBit_decode_CORBA_TypeCode(*val, buf))
      return TRUE;
    ORBit_RootObject_duplicate(*(CORBA_TypeCode *)*val);
    *val = ((guchar *)*val) + sizeof(CORBA_TypeCode);
    break;
  case CORBA_tk_Principal:
    {
      CORBA_Principal *p;

      *val = ALIGN_ADDRESS(*val, MAX(ALIGNOF_CORBA_STRUCT,
				     MAX(ALIGNOF_CORBA_LONG, ALIGNOF_CORBA_POINTER)));

      p = *val;
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long));
      p->_release = 1;
      if((buf->cur + sizeof(CORBA_unsigned_long)) > buf->end)
	return TRUE;
      if(giop_msg_conversion_needed(buf))
	p->_length = GUINT32_SWAP_LE_BE(*(CORBA_unsigned_long *)buf->cur);
      else
	p->_length = *(CORBA_unsigned_long *)buf->cur;
      buf->cur += sizeof(CORBA_unsigned_long);
      if((buf->cur + p->_length) > buf->end
	 || (buf->cur + p->_length) < buf->cur)
	return TRUE;
      p->_buffer = ORBit_alloc_simple(p->_length);
      memcpy(p->_buffer, buf->cur, p->_length);
      buf->cur += p->_length;
      *val = ((guchar *)*val) + sizeof(CORBA_sequence_CORBA_octet);
    }
    break;
  case CORBA_tk_objref:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
    if(ORBit_demarshal_object((CORBA_Object *)*val, buf, orb))
      return TRUE;
    *val = ((guchar *)*val) + sizeof(CORBA_Object);
    break;
  case CORBA_tk_except:
  case CORBA_tk_struct:
    *val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc));
    for(i = 0; i < tc->sub_parts; i++)
      {
	if(ORBit_demarshal_value(tc->subtypes[i], val, buf, dup_strings, orb))
	  return TRUE;
      }
    break;
  case CORBA_tk_union:
    {
      gpointer		discrim, body;
      CORBA_TypeCode	subtc;
      int			sz = 0, al = 1;
      discrim = *val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc));
      if(ORBit_demarshal_value(tc->discriminator, val, buf, dup_strings, orb))
	return TRUE;
      subtc = ORBit_get_union_tag(tc, (gconstpointer*)&discrim, FALSE);
      for(i = 0; i < tc->sub_parts; i++) {
	al = MAX(al, ORBit_find_alignment(tc->subtypes[i]));
	sz = MAX(sz, ORBit_gather_alloc_info(tc->subtypes[i]));
      }
      body = *val = ALIGN_ADDRESS(*val, al);
      if(ORBit_demarshal_value(subtc, &body, buf, dup_strings, orb))
	return TRUE;
      /* WATCHOUT: end subtc body may not be end of union */
      *val = ((guchar *)*val) + sz;
    }
    break;
  case CORBA_tk_string:
  case CORBA_tk_wstring:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
    buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long));
    if((buf->cur + sizeof(CORBA_long)) > buf->end)
      return TRUE;
    i = *(CORBA_unsigned_long *)buf->cur;
    buf->cur += sizeof(CORBA_unsigned_long);
    if((buf->cur + i) > buf->end
       || (buf->cur + i) < buf->cur)
      return TRUE;
    if(dup_strings) {
      *(char **)*val = CORBA_string_dup(buf->cur);
    } else {
      *(((ORBit_MemHow*)(buf->cur))-1) = ORBIT_MEMHOW_NONE;
      *(char **)*val = buf->cur;
    }
    *val = ((guchar *)*val) + sizeof(CORBA_char *);
    buf->cur += i;
    break;
  case CORBA_tk_sequence:
    {
      CORBA_sequence_CORBA_octet *p;
      gpointer subval;

      *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_SEQ);
      p = *val;
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long));
      if((buf->cur + sizeof(CORBA_long)*2) > buf->end)
	return TRUE;
      buf->cur += sizeof(CORBA_long); /* skip maximum */
      if(giop_msg_conversion_needed(buf))
	p->_length = GUINT32_SWAP_LE_BE(*(CORBA_unsigned_long *)buf->cur);
      else
	p->_length = *(CORBA_unsigned_long *)buf->cur;
      buf->cur += sizeof(CORBA_long);
      if(tc->subtypes[0]->kind == CORBA_tk_octet
	 || tc->subtypes[0]->kind == CORBA_tk_boolean
	 || tc->subtypes[0]->kind == CORBA_tk_char) {
	/* This special-casing could be taken further to apply to
	   all atoms... */
	if((buf->cur + p->_length) > buf->end
	   || (buf->cur + p->_length) < buf->cur)
	  return TRUE;
	p->_buffer = ORBit_alloc_simple(p->_length);
	memcpy(p->_buffer, buf->cur, p->_length);
	buf->cur = ((guchar *)buf->cur) + p->_length;
      } else {
	p->_buffer = ORBit_alloc_tcval(tc->subtypes[0],
				       p->_length);
	subval = p->_buffer;

	for(i = 0; i < p->_length; i++)
	  if(ORBit_demarshal_value(tc->subtypes[0], &subval,
				   buf, dup_strings, orb))
	    return TRUE;
      }

      *val = ((guchar *)*val) + sizeof(CORBA_sequence_CORBA_octet);
    }
    break;
  case CORBA_tk_array:
    for(i = 0; i < tc->length; i++)
      if(ORBit_demarshal_value(tc->subtypes[0], val, buf, dup_strings, orb))
	return TRUE;
    break;
  case CORBA_tk_alias:
    if(ORBit_demarshal_value(tc->subtypes[0], val, buf, dup_strings, orb))
      return TRUE;
    break;
  case CORBA_tk_fixed:
    g_error("CORBA_fixed NYI");
    break;
  default:
    break;
  }

  return TRUE;
}


gpointer
ORBit_demarshal_arg(GIOPRecvBuffer *buf,
		    CORBA_TypeCode tc,
		    gboolean dup_strings,
		    CORBA_ORB orb)
{
  gpointer retval, val;

  retval = val = ORBit_alloc_tcval(tc, 1);

  ORBit_demarshal_value(tc, &val, buf, dup_strings, orb);

  return retval;
}

gboolean
ORBit_demarshal_any(GIOPRecvBuffer *buf, CORBA_any *retval,
		    gboolean dup_strings,
		    CORBA_ORB orb)
{
  gpointer val;

  CORBA_any_set_release(retval, CORBA_TRUE);

  if(ORBit_decode_CORBA_TypeCode(&retval->_type, buf))
    return TRUE;
  ORBit_RootObject_duplicate(retval->_type);

  val = retval->_value = ORBit_alloc_tcval(retval->_type, 1);
  if(ORBit_demarshal_value(retval->_type, &val, buf, dup_strings, orb))
    return TRUE;
  return FALSE;
}

gpointer
CORBA_any__freekids(gpointer mem, gpointer dat)
{
  CORBA_any *t;
  t = mem;
  if(t->_release)
    CORBA_free(t->_value);
  return t + 1;
}

CORBA_any *
CORBA_any__alloc(void)
{
  CORBA_any *retval = ORBit_alloc(sizeof(CORBA_any), 1, 
				  &CORBA_any__freekids);
  memset(retval, 0, sizeof(CORBA_any)); /* Make things easier on stubs */
  return retval;
}

void
CORBA_any_set_release(CORBA_any *val, CORBA_boolean setme)
{
  val->_release = setme;
}

void
ORBit_copy_value_core(gconstpointer *val, gpointer *newval, CORBA_TypeCode tc)
{
  CORBA_long i;
  gconstpointer pval1; 
  gpointer pval2;

  switch(tc->kind) {
  case CORBA_tk_wchar:
  case CORBA_tk_short:
  case CORBA_tk_ushort:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_SHORT);
    *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_SHORT);
    *(CORBA_short *)*newval = *(CORBA_short *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_short);
    *newval = ((guchar *)*newval) + sizeof(CORBA_short);
    break;
  case CORBA_tk_enum:
  case CORBA_tk_long:
  case CORBA_tk_ulong:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG);
    *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_LONG);
    *(CORBA_long *)*newval = *(CORBA_long *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_long);
    *newval = ((guchar *)*newval) + sizeof(CORBA_long);
    break;
  case CORBA_tk_longlong:
  case CORBA_tk_ulonglong:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_LONG);
    *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_LONG_LONG);
    *(CORBA_long_long *)*newval = *(CORBA_long_long *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_long_long);
    *newval = ((guchar *)*newval) + sizeof(CORBA_long_long);
    break;
  case CORBA_tk_longdouble:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_DOUBLE);
    *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_LONG_DOUBLE);
    *(CORBA_long_double *)*newval = *(CORBA_long_double *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_long_double);
    *newval = ((guchar *)*newval) + sizeof(CORBA_long_double);
    break;
  case CORBA_tk_float:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_FLOAT);
    *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_FLOAT);
    *(CORBA_long *)*newval = *(CORBA_long *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_float);
    *newval = ((guchar *)*newval) + sizeof(CORBA_float);
    break;
  case CORBA_tk_double:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_DOUBLE);
    *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_DOUBLE);
    *(CORBA_double *)*newval = *(CORBA_double *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_double);
    *newval = ((guchar *)*newval) + sizeof(CORBA_double);
    break;
  case CORBA_tk_boolean:
  case CORBA_tk_char:
  case CORBA_tk_octet:
    *(CORBA_octet *)*newval = *(CORBA_octet *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_octet);
    *newval = ((guchar *)*newval) + sizeof(CORBA_octet);
    break;
  case CORBA_tk_any:
    {
      const CORBA_any *oldany;
      CORBA_any *newany;
      *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_ANY);
      *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_ANY);
      oldany = *val;
      newany = *newval;
      newany->_type = ORBit_RootObject_duplicate(oldany->_type);
      /* XXX are we supposed to do above even if oldany->_release
	 == FALSE? */
      newany->_value = ORBit_copy_value(oldany->_value, oldany->_type);
      newany->_release = CORBA_TRUE;
      *val = ((guchar *)*val) + sizeof(CORBA_any);
      *newval = ((guchar *)*newval) + sizeof(CORBA_any);
    }
    break;
  case CORBA_tk_Principal:
    *val = ALIGN_ADDRESS(*val,
			 MAX(MAX(ALIGNOF_CORBA_LONG,
				 ALIGNOF_CORBA_STRUCT),
			     ALIGNOF_CORBA_POINTER));
    *newval = ALIGN_ADDRESS(*newval,
			    MAX(MAX(ALIGNOF_CORBA_LONG,
				    ALIGNOF_CORBA_STRUCT),
				ALIGNOF_CORBA_POINTER));
    *(CORBA_Principal *)*newval = *(CORBA_Principal *)*val;
    ((CORBA_Principal *)*newval)->_buffer =
      CORBA_sequence_CORBA_octet_allocbuf(((CORBA_Principal *)*newval)->_length);
    memcpy(((CORBA_Principal *)*newval)->_buffer,
	   ((CORBA_Principal *)*val)->_buffer,
	   ((CORBA_Principal *)*val)->_length);
    *val = ((guchar *)*val) + sizeof(CORBA_Principal);
    *newval = ((guchar *)*newval) + sizeof(CORBA_Principal);
    break;
  case CORBA_tk_TypeCode:
  case CORBA_tk_objref:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
    *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_POINTER);
    *(CORBA_Object *)*newval = CORBA_Object_duplicate(*(CORBA_Object *)*val,
						      NULL);
    *val = ((guchar *)*val) + sizeof(CORBA_Object);
    *newval = ((guchar *)*newval) + sizeof(CORBA_Object);
    break;
  case CORBA_tk_struct:
  case CORBA_tk_except:
    *val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc));
    *newval = ALIGN_ADDRESS(*newval, ORBit_find_alignment(tc));
    for(i = 0; i < tc->sub_parts; i++) {
      ORBit_copy_value_core(val, newval, tc->subtypes[i]);
    }
    break;
  case CORBA_tk_union:
    {
      CORBA_TypeCode utc = ORBit_get_union_tag(tc, val, FALSE);
      gint	union_align = ORBit_find_alignment(tc);
      size_t	union_size = ORBit_gather_alloc_info(tc);

      /* need to advance val,newval by size of union, not just
       * current tagged field within it */
      pval1 = *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_STRUCT);
      pval2 = *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_STRUCT);
      ORBit_copy_value_core(&pval1, &pval2, tc->discriminator);
      pval1 = ALIGN_ADDRESS(pval1, union_align);
      pval2 = ALIGN_ADDRESS(pval2, union_align);
      ORBit_copy_value_core(&pval1, &pval2, utc);
      *val = ((guchar *)*val) + union_size;
      *newval = ((guchar *)*newval) + union_size;
    }
    break;
  case CORBA_tk_wstring:
  case CORBA_tk_string:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
    *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_POINTER);
	
    *(CORBA_char **)*newval = CORBA_string_dup(*(CORBA_char **)*val);
    *val = ((guchar *)*val) + sizeof(CORBA_char *);
    *newval = ((guchar *)*newval) + sizeof(CORBA_char *);
    break;
  case CORBA_tk_sequence:
    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_SEQ);
    *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_SEQ);
    ((CORBA_Principal *)*newval)->_release = CORBA_TRUE;
    ((CORBA_Principal *)*newval)->_length =
      ((CORBA_Principal *)*newval)->_maximum =
      ((CORBA_Principal *)*val)->_length;
    ((CORBA_Principal *)*newval)->_buffer = pval2 =
      ORBit_alloc_tcval(tc->subtypes[0],
			((CORBA_Principal *)*val)->_length);
    pval1 = ((CORBA_Principal *)*val)->_buffer;
	
    for(i = 0; i < ((CORBA_Principal *)*newval)->_length; i++) {
      ORBit_copy_value_core(&pval1, &pval2, tc->subtypes[0]);
    }
    *val = ((guchar *)*val) + sizeof(CORBA_sequence_CORBA_octet);
    *newval = ((guchar *)*newval) + sizeof(CORBA_sequence_CORBA_octet);
    break;
  case CORBA_tk_array:
    for(i = 0; i < tc->length; i++) {
      ORBit_copy_value_core(val, newval, tc->subtypes[0]);
    }
    break;
  case CORBA_tk_alias:
    ORBit_copy_value_core(val, newval, tc->subtypes[0]);
    break;
  case CORBA_tk_fixed:
    g_error("CORBA_fixed NYI!");
    break;
  case CORBA_tk_void:
  case CORBA_tk_null:
    *val = NULL;
    break;
  default:
    g_error("Can't handle copy of value kind %d", tc->kind);
  }
}

gpointer
ORBit_copy_value(gconstpointer value, CORBA_TypeCode tc)
{
  gpointer retval, newval;

  retval = newval = ORBit_alloc_tcval(tc, 1);
  ORBit_copy_value_core(&value, &newval, tc);

  return retval;
}

void
CORBA_any__copy(CORBA_any *out, CORBA_any *in)
{
  out->_type = ORBit_RootObject_duplicate(in->_type);
  out->_value = ORBit_copy_value(in->_value, in->_type);
  out->_release = CORBA_TRUE;
}
