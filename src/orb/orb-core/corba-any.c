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

static glong ORBit_get_union_switch(CORBA_TypeCode tc, gconstpointer *val, 
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

void ORBit_marshal_arg(GIOPSendBuffer *buf,
                       gconstpointer val,
                       CORBA_TypeCode tc)
{
}

void ORBit_marshal_any(GIOPSendBuffer *buf, const CORBA_any *val)
{
}

gpointer
ORBit_demarshal_arg(GIOPRecvBuffer *buf,
		    CORBA_TypeCode tc,
		    gboolean dup_strings,
		    CORBA_ORB orb)
{
  return NULL;
}

gboolean
ORBit_demarshal_any(GIOPRecvBuffer *buf, CORBA_any *retval,
		    gboolean dup_strings,
		    CORBA_ORB orb)
{
  return FALSE;
}

gpointer
CORBA_any__freekids(gpointer mem, gpointer dat)
{
  CORBA_any *t;
  t = mem;
  if(t->_release)
    ORBit_free(t->_value); /* XXX fixme */
  return t + 1;
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
