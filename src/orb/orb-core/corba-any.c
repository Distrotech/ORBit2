#include "config.h"
#include <orbit/orbit.h>
#include "orb-core-private.h"
#include <string.h>

size_t
ORBit_gather_alloc_info(CORBA_TypeCode tc)
{

  while (tc->kind == CORBA_tk_alias)
    tc = tc->subtypes[0];

  switch(tc->kind) {
  case CORBA_tk_long:
  case CORBA_tk_ulong:
  case CORBA_tk_enum:
    return sizeof(CORBA_long);
  case CORBA_tk_short:
  case CORBA_tk_ushort:
    return sizeof(CORBA_short);
  case CORBA_tk_float:
    return sizeof(CORBA_float);
  case CORBA_tk_double:
    return sizeof(CORBA_double);
  case CORBA_tk_boolean:
  case CORBA_tk_char:
  case CORBA_tk_octet:
    return sizeof(CORBA_octet);
  case CORBA_tk_any:
    return sizeof(CORBA_any);
  case CORBA_tk_TypeCode:
    return sizeof(CORBA_TypeCode);
  case CORBA_tk_Principal:
    return sizeof(CORBA_Principal);
  case CORBA_tk_objref:
    return sizeof(CORBA_Object);
  case CORBA_tk_except:
  case CORBA_tk_struct: {
    int i, sum;

    for(sum = i = 0; i < tc->sub_parts; i++) {
      sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, tc->subtypes[i]->c_align));
      sum += ORBit_gather_alloc_info(tc->subtypes[i]);
    }
    sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, tc->c_align));

    return sum;
    }
  case CORBA_tk_union: {
    int i, n, align, prevalign, prev, sum;
    sum = ORBit_gather_alloc_info(tc->discriminator);
    n = -1;
    align = 1;
    for(prev = prevalign = i = 0; i < tc->sub_parts; i++) {
      prevalign = align;
      align = tc->subtypes[i]->c_align;
      if(align > prevalign)
	n = i;

      prev = MAX(prev, ORBit_gather_alloc_info(tc->subtypes[i]));
    }
    if(n >= 0)
      sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, tc->subtypes[n]->c_align));
    sum += prev;
    sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, tc->c_align));
    return sum;
    }
  case CORBA_tk_wstring:
  case CORBA_tk_string:
    return sizeof(char *);
  case CORBA_tk_sequence:
    return sizeof(CORBA_sequence_CORBA_octet);
  case CORBA_tk_array:
    return ORBit_gather_alloc_info(tc->subtypes[0]) * tc->length;
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

/*
 * ORBit_marshal_value:
 * @buf: the send buffer.
 * @val: a pointer to the location of the value.
 * @tc:  the TypeCode indicating the type of the value.
 *
 * Marshals the value onto the buffer and changes @val to point 
 * to the location after the value.
 */
void
ORBit_marshal_value (GIOPSendBuffer *buf,
		     gconstpointer  *val,
		     CORBA_TypeCode  tc)
{
	CORBA_unsigned_long i, ulval;
	gconstpointer       subval;

	while (tc->kind == CORBA_tk_alias)
		tc = tc->subtypes[0];

	switch(tc->kind) {
	case CORBA_tk_wchar:
	case CORBA_tk_ushort:
	case CORBA_tk_short:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_SHORT);
		giop_send_buffer_append_aligned (buf, *val, sizeof (CORBA_short));
		*val = ((guchar *)*val) + sizeof (CORBA_short);
		break;
	case CORBA_tk_enum:
	case CORBA_tk_long:
	case CORBA_tk_ulong:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_LONG);
		giop_send_buffer_append_aligned (buf, *val, sizeof (CORBA_long));
		*val = ((guchar *)*val) + sizeof (CORBA_long);
		break;
	case CORBA_tk_float:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_FLOAT);
		giop_send_buffer_append_aligned (buf, *val, sizeof (CORBA_float));
		*val = ((guchar *)*val) + sizeof (CORBA_float);
		break;
	case CORBA_tk_double:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_DOUBLE);
		giop_send_buffer_append_aligned (buf, *val, sizeof (CORBA_double));
		*val = ((guchar *)*val) + sizeof (CORBA_double);
		break;
	case CORBA_tk_boolean:
	case CORBA_tk_char:
	case CORBA_tk_octet:
		giop_send_buffer_append (buf, *val, sizeof (CORBA_octet));
		*val = ((guchar *)*val) + sizeof (CORBA_octet);
		break;
	case CORBA_tk_any:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_ANY);
		ORBit_marshal_any (buf, *val);
		*val = ((guchar *)*val) + sizeof (CORBA_any);
		break;
	case CORBA_tk_Principal:
		*val = ALIGN_ADDRESS (*val, MAX(MAX(ORBIT_ALIGNOF_CORBA_LONG,
						    ORBIT_ALIGNOF_CORBA_STRUCT),
						    ORBIT_ALIGNOF_CORBA_POINTER));

		ulval = *(CORBA_unsigned_long *)(*val);
		giop_send_buffer_append (buf, *val, sizeof (CORBA_unsigned_long));

		giop_send_buffer_append (buf,
					*(char **)((char *)*val + sizeof (CORBA_unsigned_long)),
					ulval);
		*val = ((guchar *)*val) + sizeof (CORBA_Principal);
		break;
	case CORBA_tk_objref:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_POINTER);
		ORBit_marshal_object (buf, *(CORBA_Object *)*val);
		*val = ((guchar *)*val) + sizeof (CORBA_Object);
		break;
	case CORBA_tk_TypeCode:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_POINTER);
		ORBit_encode_CORBA_TypeCode (*(CORBA_TypeCode *)*val, buf);
		*val = ((guchar *)*val) + sizeof (CORBA_TypeCode);
		break;
	case CORBA_tk_except:
	case CORBA_tk_struct:
		*val = ALIGN_ADDRESS (*val, tc->c_align);
		for(i = 0; i < tc->sub_parts; i++)
			ORBit_marshal_value (buf, val, tc->subtypes[i]);
		break;
	case CORBA_tk_union: {
		gconstpointer	discrim, body;
		CORBA_TypeCode 	subtc;
		int             al = 1, sz = 0;

		discrim = *val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_STRUCT);
		ORBit_marshal_value (buf, val, tc->discriminator);
		subtc = ORBit_get_union_tag (tc, &discrim, FALSE);
		for (i=0; i < tc->sub_parts; i++) {
			al = MAX (al, tc->subtypes[i]->c_align);
			sz = MAX (sz, ORBit_gather_alloc_info (tc->subtypes[i]));
		}
		body = *val = ALIGN_ADDRESS (*val, al);
		ORBit_marshal_value (buf, &body, subtc);
		/* FIXME:
		 * WATCHOUT: end of subtc may not be end of union
		 */
		*val = ((guchar *)*val) + sz;
		}
		break;
	case CORBA_tk_wstring:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_POINTER);
		ulval = CORBA_wstring_len (*(CORBA_wchar **)*val) + 1;
		giop_send_buffer_append_aligned (buf, &ulval,
						 sizeof (CORBA_unsigned_long));
		giop_send_buffer_append (buf, *(char **)*val, ulval);
		*val = ((guchar *)*val) + sizeof (char *);
		break;
	case CORBA_tk_string:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_POINTER);
		ulval = strlen (*(char **)*val) + 1;
		giop_send_buffer_append_aligned (buf, &ulval,
						 sizeof (CORBA_unsigned_long));
		giop_send_buffer_append (buf, *(char **)*val, ulval);
	
		*val = ((guchar *)*val) + sizeof (char *);
		break;
	case CORBA_tk_sequence: {
		const CORBA_sequence_CORBA_octet *sval;
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_SEQ);
		sval = *val;
		giop_send_buffer_align (buf, sizeof (CORBA_unsigned_long));
		giop_send_buffer_append (buf, &sval->_length,
					 sizeof (CORBA_unsigned_long));

		subval = sval->_buffer;

		switch (tc->subtypes[0]->kind) {
		case CORBA_tk_boolean:
		case CORBA_tk_char:
		case CORBA_tk_octet:
			giop_send_buffer_append (buf, subval, sval->_length);
			break;
		default:
			for(i = 0; i < sval->_length; i++)
				ORBit_marshal_value (buf, &subval, tc->subtypes[0]);
			break;
		}
		*val = ((guchar *)*val) + sizeof (CORBA_sequence_CORBA_octet);
		}
		break;
	case CORBA_tk_array:
		switch (tc->subtypes[0]->kind) {
		case CORBA_tk_boolean:
		case CORBA_tk_char:
		case CORBA_tk_octet:
			giop_send_buffer_append (buf, val, tc->length);
			break;
		default: {
			int align = tc->subtypes[0]->c_align;

	  		for(i = 0; i < tc->length; i++) {
				ORBit_marshal_value (buf, val, tc->subtypes[0]);
	
				*val = ALIGN_ADDRESS (*val, align);
			}
			break;
			}
		}
		break;
	case CORBA_tk_longlong:
	case CORBA_tk_ulonglong:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_LONG_LONG);
		giop_send_buffer_append_aligned (buf, *val, sizeof (CORBA_long_long));
		*val = ((guchar *)*val) + sizeof (CORBA_long_long);
		break;
	case CORBA_tk_longdouble:
		*val = ALIGN_ADDRESS (*val, ORBIT_ALIGNOF_CORBA_LONG_DOUBLE);
		giop_send_buffer_append_aligned (buf, *val, sizeof (CORBA_long_double));
		*val = ((guchar *)*val) + sizeof (CORBA_long_double);
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
		break;
	}
}

static glong
ORBit_get_union_switch(CORBA_TypeCode tc, gconstpointer *val, 
		       gboolean update)
{
  glong retval = 0; /* Quiet gcc */

  while (tc->kind == CORBA_tk_alias)
    tc = tc->subtypes[0];

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
ORBit_get_union_tag(CORBA_TypeCode union_tc,
		    gconstpointer *val, 
		    gboolean       update)
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
  ORBit_marshal_value(buf, &val, tc);
}

void
ORBit_marshal_any(GIOPSendBuffer *buf, const CORBA_any *val)
{
  gconstpointer mval = val->_value;

  ORBit_encode_CORBA_TypeCode(val->_type, buf);

  ORBit_marshal_value(buf, &mval, val->_type);
}

gboolean
ORBit_demarshal_value(CORBA_TypeCode tc,
		      gpointer *val,
		      GIOPRecvBuffer *buf,
		      gboolean dup_strings,
		      CORBA_ORB orb)
{
  CORBA_long i;

  while (tc->kind == CORBA_tk_alias)
    tc = tc->subtypes[0];

  switch(tc->kind) {
  case CORBA_tk_short:
  case CORBA_tk_ushort:
  case CORBA_tk_wchar:
    {
      CORBA_unsigned_short *ptr;
      *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_SHORT);
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
      *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_LONG);
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
      *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_LONG_LONG);
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
      *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_LONG_DOUBLE);
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long_double));
      if((buf->cur + sizeof(CORBA_long_double)) > buf->end)
	return TRUE;
      ptr = *val;
      if(giop_msg_conversion_needed(buf))
	giop_byteswap((guchar *)ptr, buf->cur, sizeof(CORBA_long_double));
      else
	*ptr = *(CORBA_long_double *)buf->cur;
      buf->cur += sizeof(CORBA_long_double);
      *val = ((guchar *)*val) + sizeof(CORBA_long_double);
    }
    break;
  case CORBA_tk_float:
    {
      CORBA_float *ptr;
      *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_FLOAT);
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_float));
      if((buf->cur + sizeof(CORBA_float)) > buf->end)
	return TRUE;
      ptr = *val;
      if(giop_msg_conversion_needed(buf))
	giop_byteswap((guchar *)ptr, buf->cur, sizeof(CORBA_float));
      else
	*ptr = *(CORBA_float *)buf->cur;
      buf->cur += sizeof(CORBA_float);

      *val = ((guchar *)*val) + sizeof(CORBA_float);
    }
    break;
  case CORBA_tk_double:
    {
      CORBA_double *ptr;
      *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_DOUBLE);
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_double));
      if((buf->cur + sizeof(CORBA_double)) > buf->end)
	return TRUE;
      ptr = *val;
      if(giop_msg_conversion_needed(buf))
	giop_byteswap((guchar *)ptr, buf->cur, sizeof(CORBA_double));
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
      *ptr = *buf->cur;
      buf->cur++;
      
      *val = ((guchar *)*val) + sizeof(CORBA_octet);
    }
    break;
  case CORBA_tk_any:
    {
      CORBA_any *decoded;

      *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_ANY);
      decoded = *val;
      decoded->_release = CORBA_FALSE;
      if(ORBit_demarshal_any(buf, decoded, dup_strings, orb))
	return TRUE;
      *val = ((guchar *)*val) + sizeof(CORBA_any);
    }
    break;
  case CORBA_tk_Principal:
    {
      CORBA_Principal *p;

      *val = ALIGN_ADDRESS(*val, MAX(ORBIT_ALIGNOF_CORBA_STRUCT,
				     MAX(ORBIT_ALIGNOF_CORBA_LONG, ORBIT_ALIGNOF_CORBA_POINTER)));

      p = *val;
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long));
      p->_release = TRUE;
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
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_POINTER);
    if(ORBit_demarshal_object((CORBA_Object *)*val, buf, orb))
      return TRUE;
    *val = ((guchar *)*val) + sizeof(CORBA_Object);
    break;
  case CORBA_tk_TypeCode:
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_POINTER);
    if(ORBit_decode_CORBA_TypeCode(*val, buf))
      return TRUE;
    ORBit_RootObject_duplicate(*(CORBA_TypeCode *)*val);
    *val = ((guchar *)*val) + sizeof(CORBA_TypeCode);
    break;
  case CORBA_tk_except:
  case CORBA_tk_struct:
    *val = ALIGN_ADDRESS(*val, tc->c_align);
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
      discrim = *val = ALIGN_ADDRESS(*val, tc->c_align);
      if(ORBit_demarshal_value(tc->discriminator, val, buf, dup_strings, orb))
	return TRUE;
      subtc = ORBit_get_union_tag(tc, (gconstpointer*)&discrim, FALSE);
      for(i = 0; i < tc->sub_parts; i++) {
	al = MAX(al, tc->subtypes[i]->c_align);
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
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_POINTER);
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

      *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_SEQ);
      p = *val;
      p->_release = TRUE;
      buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long));
      if((buf->cur + sizeof(CORBA_long)) > buf->end)
	return TRUE;
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
  case CORBA_tk_fixed:
  default:
    return TRUE;
    break;
  }

  return FALSE;
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
  if(t->_type)
    ORBit_RootObject_release_T ((ORBit_RootObject)t->_type);
  if(t->_release)
    ORBit_free_T(t->_value);
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
ORBit_copy_value_core(gconstpointer *val, gpointer *newval, CORBA_TypeCode tc)
{
  CORBA_long i;
  gconstpointer pval1; 
  gpointer pval2;

  while (tc->kind == CORBA_tk_alias)
    tc = tc->subtypes[0];

  switch(tc->kind) {
  case CORBA_tk_wchar:
  case CORBA_tk_short:
  case CORBA_tk_ushort:
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_SHORT);
    *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_SHORT);
    *(CORBA_short *)*newval = *(CORBA_short *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_short);
    *newval = ((guchar *)*newval) + sizeof(CORBA_short);
    break;
  case CORBA_tk_enum:
  case CORBA_tk_long:
  case CORBA_tk_ulong:
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_LONG);
    *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_LONG);
    *(CORBA_long *)*newval = *(CORBA_long *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_long);
    *newval = ((guchar *)*newval) + sizeof(CORBA_long);
    break;
  case CORBA_tk_longlong:
  case CORBA_tk_ulonglong:
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_LONG_LONG);
    *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_LONG_LONG);
    *(CORBA_long_long *)*newval = *(CORBA_long_long *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_long_long);
    *newval = ((guchar *)*newval) + sizeof(CORBA_long_long);
    break;
  case CORBA_tk_longdouble:
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_LONG_DOUBLE);
    *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_LONG_DOUBLE);
    *(CORBA_long_double *)*newval = *(CORBA_long_double *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_long_double);
    *newval = ((guchar *)*newval) + sizeof(CORBA_long_double);
    break;
  case CORBA_tk_float:
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_FLOAT);
    *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_FLOAT);
    *(CORBA_long *)*newval = *(CORBA_long *)*val;
    *val = ((guchar *)*val) + sizeof(CORBA_float);
    *newval = ((guchar *)*newval) + sizeof(CORBA_float);
    break;
  case CORBA_tk_double:
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_DOUBLE);
    *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_DOUBLE);
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
      *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_ANY);
      *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_ANY);
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
			 MAX(MAX(ORBIT_ALIGNOF_CORBA_LONG,
				 ORBIT_ALIGNOF_CORBA_STRUCT),
			     ORBIT_ALIGNOF_CORBA_POINTER));
    *newval = ALIGN_ADDRESS(*newval,
			    MAX(MAX(ORBIT_ALIGNOF_CORBA_LONG,
				    ORBIT_ALIGNOF_CORBA_STRUCT),
				ORBIT_ALIGNOF_CORBA_POINTER));
    *(CORBA_Principal *)*newval = *(CORBA_Principal *)*val;
    ((CORBA_Principal *)*newval)->_buffer =
      CORBA_sequence_CORBA_octet_allocbuf(((CORBA_Principal *)*newval)->_length);
    ((CORBA_Principal *)*newval)->_release = CORBA_TRUE;
    memcpy(((CORBA_Principal *)*newval)->_buffer,
	   ((CORBA_Principal *)*val)->_buffer,
	   ((CORBA_Principal *)*val)->_length);
    *val = ((guchar *)*val) + sizeof(CORBA_Principal);
    *newval = ((guchar *)*newval) + sizeof(CORBA_Principal);
    break;
  case CORBA_tk_TypeCode:
  case CORBA_tk_objref:
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_POINTER);
    *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_POINTER);
    *(CORBA_Object *)*newval = CORBA_Object_duplicate(*(CORBA_Object *)*val,
						      NULL);
    *val = ((guchar *)*val) + sizeof(CORBA_Object);
    *newval = ((guchar *)*newval) + sizeof(CORBA_Object);
    break;
  case CORBA_tk_struct:
  case CORBA_tk_except:
    *val = ALIGN_ADDRESS(*val, tc->c_align);
    *newval = ALIGN_ADDRESS(*newval, tc->c_align);
    for(i = 0; i < tc->sub_parts; i++) {
      ORBit_copy_value_core(val, newval, tc->subtypes[i]);
    }
    break;
  case CORBA_tk_union:
    {
      CORBA_TypeCode utc = ORBit_get_union_tag(tc, (gconstpointer *)val, FALSE);
      gint	union_align = tc->c_align;
      size_t	union_size = ORBit_gather_alloc_info(tc);

      /* need to advance val,newval by size of union, not just
       * current tagged field within it */
      pval1 = *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_STRUCT);
      pval2 = *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_STRUCT);
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
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_POINTER);
    *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_POINTER);
	
    *(CORBA_char **)*newval = CORBA_string_dup(*(CORBA_char **)*val);
    *val = ((guchar *)*val) + sizeof(CORBA_char *);
    *newval = ((guchar *)*newval) + sizeof(CORBA_char *);
    break;
  case CORBA_tk_sequence:
    *val = ALIGN_ADDRESS(*val, ORBIT_ALIGNOF_CORBA_SEQ);
    *newval = ALIGN_ADDRESS(*newval, ORBIT_ALIGNOF_CORBA_SEQ);
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
CORBA_any__copy(CORBA_any *out, const CORBA_any *in)
{
  out->_type = ORBit_RootObject_duplicate(in->_type);
  out->_value = ORBit_copy_value(in->_value, in->_type);
  out->_release = CORBA_TRUE;
}

#define ALIGN_COMPARE(a,b,tk,type,align)	\
	case CORBA_tk_##tk:			\
		*a = ALIGN_ADDRESS (*a, align);	\
		*b = ALIGN_ADDRESS (*b, align);	\
		ret = *(CORBA_##type *) *a == *(CORBA_##type *) *b;	\
		*a = ((guchar *) *a) + sizeof (CORBA_##type);		\
		*b = ((guchar *) *b) + sizeof (CORBA_##type);		\
		return ret

#define ORBIT_ALIGNOF_CORBA_UNION	(MAX (MAX (ORBIT_ALIGNOF_CORBA_LONG,		\
					   ORBIT_ALIGNOF_CORBA_STRUCT),	\
				      ORBIT_ALIGNOF_CORBA_POINTER))

CORBA_boolean
ORBit_value_equivalent (gpointer *a, gpointer *b,
			CORBA_TypeCode tc,
			CORBA_Environment *ev)
{
	gboolean ret;
	int i;

	while (tc->kind == CORBA_tk_alias)
		tc = tc->subtypes[0];

	switch (tc->kind) {
	case CORBA_tk_null:
	case CORBA_tk_void:
		return TRUE;

		ALIGN_COMPARE(a, b, short,  short, ORBIT_ALIGNOF_CORBA_SHORT);
		ALIGN_COMPARE(a, b, ushort, short, ORBIT_ALIGNOF_CORBA_SHORT);
		ALIGN_COMPARE(a, b, wchar,  short, ORBIT_ALIGNOF_CORBA_SHORT);

		ALIGN_COMPARE(a, b, enum, long,  ORBIT_ALIGNOF_CORBA_LONG);
		ALIGN_COMPARE(a, b, long, long,  ORBIT_ALIGNOF_CORBA_LONG);
		ALIGN_COMPARE(a, b, ulong, long, ORBIT_ALIGNOF_CORBA_LONG);

		ALIGN_COMPARE(a, b, longlong, long_long,  ORBIT_ALIGNOF_CORBA_LONG_LONG);
		ALIGN_COMPARE(a, b, ulonglong, long_long, ORBIT_ALIGNOF_CORBA_LONG_LONG);

		ALIGN_COMPARE(a, b, longdouble, long_double, ORBIT_ALIGNOF_CORBA_LONG_DOUBLE);

		ALIGN_COMPARE(a, b, float, float,   ORBIT_ALIGNOF_CORBA_FLOAT);
		ALIGN_COMPARE(a, b, double, double, ORBIT_ALIGNOF_CORBA_DOUBLE);

		ALIGN_COMPARE(a, b, boolean, octet, ORBIT_ALIGNOF_CORBA_OCTET);
		ALIGN_COMPARE(a, b, char,    octet, ORBIT_ALIGNOF_CORBA_OCTET);
		ALIGN_COMPARE(a, b, octet,   octet, ORBIT_ALIGNOF_CORBA_OCTET);

	case CORBA_tk_string:
		*a = ALIGN_ADDRESS (*a, ORBIT_ALIGNOF_CORBA_POINTER);
		*b = ALIGN_ADDRESS (*b, ORBIT_ALIGNOF_CORBA_POINTER);
		ret = !strcmp (*(char **)*a, *(char **)*b);
		*a = ((guchar *) *a) + sizeof (CORBA_char *);
		*b = ((guchar *) *b) + sizeof (CORBA_char *);
		return ret;

	case CORBA_tk_wstring:
		g_warning ("wstring totaly broken");
  		return FALSE;

	case CORBA_tk_TypeCode:
	case CORBA_tk_objref:
		*a = ALIGN_ADDRESS (*a, ORBIT_ALIGNOF_CORBA_POINTER);
		*b = ALIGN_ADDRESS (*b, ORBIT_ALIGNOF_CORBA_POINTER);
		ret = CORBA_Object_is_equivalent (*a, *b, ev);
		*a = ((guchar *) *a) + sizeof (CORBA_Object);
		*b = ((guchar *) *b) + sizeof (CORBA_Object);
		return ret;

	case CORBA_tk_any: {
		CORBA_any *any_a, *any_b;

		*a = ALIGN_ADDRESS (*a, ORBIT_ALIGNOF_CORBA_POINTER);
		*b = ALIGN_ADDRESS (*b, ORBIT_ALIGNOF_CORBA_POINTER);

		any_a = *((CORBA_any **) *a);
		any_b = *((CORBA_any **) *b);

		ret = ORBit_any_equivalent (any_a, any_b, ev);

		*a = ((guchar *) *a) + sizeof (CORBA_any *);
		*b = ((guchar *) *b) + sizeof (CORBA_any *);

		return ret;
	}

	case CORBA_tk_struct:
	case CORBA_tk_except: {
		int i;

		*a = ALIGN_ADDRESS (*a, tc->c_align);
		*b = ALIGN_ADDRESS (*b, tc->c_align);

		for (i = 0; i < tc->sub_parts; i++)
			if (!ORBit_value_equivalent (a, b, tc->subtypes [i], ev))
				return FALSE;

		return TRUE;
	}

	case CORBA_tk_sequence: {
		CORBA_Principal *ap, *bp;
		gpointer a_val, b_val;

		*a = ALIGN_ADDRESS (*a, ORBIT_ALIGNOF_CORBA_UNION);
		*b = ALIGN_ADDRESS (*b, ORBIT_ALIGNOF_CORBA_UNION);
			
		ap = (CORBA_Principal *) *a;
		bp = (CORBA_Principal *) *b;

		if (ap->_length != bp->_length)
			return FALSE;

		a_val = ap->_buffer;
		b_val = bp->_buffer;

		for (i = 0; i < ap->_length; i++) {
			if (!ORBit_value_equivalent (&a_val, &b_val, tc->subtypes [0], ev))
				return FALSE;
		}
		*a = ((guchar *) *a) + sizeof (CORBA_sequence_CORBA_octet);
		*b = ((guchar *) *b) + sizeof (CORBA_sequence_CORBA_octet);
		return TRUE;
	}

	case CORBA_tk_union: {
		gint   union_align = tc->c_align;
		size_t union_size = ORBit_gather_alloc_info (tc);

		CORBA_TypeCode utc_a = ORBit_get_union_tag (
			tc, (gconstpointer *)a, FALSE);
		CORBA_TypeCode utc_b = ORBit_get_union_tag (
			tc, (gconstpointer *)b, FALSE);

		if (!CORBA_TypeCode_equal (utc_a, utc_b, ev))
			return FALSE;

		*a = ALIGN_ADDRESS (*a, ORBIT_ALIGNOF_CORBA_STRUCT);
		*b = ALIGN_ADDRESS (*b, ORBIT_ALIGNOF_CORBA_STRUCT);

		if (!ORBit_value_equivalent (a, b, tc->discriminator, ev))
			return FALSE;

		*a = ALIGN_ADDRESS (*a, union_align);
		*b = ALIGN_ADDRESS (*b, union_align);

		if (!ORBit_value_equivalent (a, b, utc_a, ev))
			return FALSE;

		*a = ((guchar *) *a) + union_size;
		*b = ((guchar *) *b) + union_size;
		return TRUE;
	}

	case CORBA_tk_array:
		for (i = 0; i < tc->length; i++) {
			if (!ORBit_value_equivalent (a, b, tc->subtypes [0], ev))
				return FALSE;
		}
		return TRUE;

	default:
		g_warning ("ORBit_value_equivalent unimplemented");
		return FALSE;
	};
}

/*
 * Compares the typecodes of each any
 */
CORBA_boolean
ORBit_any_equivalent(CORBA_any *obj, CORBA_any *any, CORBA_Environment *ev)
{
	gpointer a, b;

	/* Is this correct ? */
	if (obj == NULL &&
	    any == NULL)
		return TRUE;

	if (!obj->_type || !any->_type) {
		CORBA_exception_set_system (
			ev, ex_CORBA_BAD_PARAM, CORBA_COMPLETED_NO);
		return FALSE;
	}

	if (!CORBA_TypeCode_equal (obj->_type, any->_type, ev))
		return FALSE;

	if (ev->_major != CORBA_NO_EXCEPTION)
		return FALSE;
	
	a = obj->_value;
	b = any->_value;

	return ORBit_value_equivalent (&a, &b, any->_type, ev);
}
