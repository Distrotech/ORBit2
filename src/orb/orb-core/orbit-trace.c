#include "config.h"
#include <orbit/orbit.h>
#include "orb-core-private.h"
#include <string.h>

#define tprintf(format...) fprintf(stderr, format)

void
ORBit_trace_objref (const CORBA_Object obj)
{
	tprintf ("[%p]", obj);
}

void
ORBit_trace_any (const CORBA_any *any)
{
	tprintf ("{ an any }");
}

void
ORBit_trace_typecode (const CORBA_TypeCode tc)
{
	tprintf ("{ tc %d }", tc->kind);
}

void
ORBit_trace_value (gconstpointer *val,
		   CORBA_TypeCode tc,
		   ORBit_marshal_value_info *mi)
{
	CORBA_unsigned_long i;
	ORBit_marshal_value_info submi;

	*val = ALIGN_ADDRESS (*val, ORBit_find_alignment (tc));

	switch (tc->kind) {

	case CORBA_tk_alias:
		submi.alias_element_type = tc->subtypes[0];
		ORBit_trace_value (val, submi.alias_element_type, &submi);
		break;

#define _ORBIT_HANDLE_TYPE(ctk,typ,ctpye,wirebits,wirebytes,format) \
	case CORBA_tk_##ctk: \
		tprintf (format, *(CORBA_##typ *) *val); \
		break;

/* CORBA_tk_, CORBA_ type, C-stack type, wire size (bits), wire size (bytes), print format */
	_ORBIT_HANDLE_TYPE (short, short, int, 16, 2, "%d");
	_ORBIT_HANDLE_TYPE (long, long, int, 32, 4, "0x%x");
	/* FIXME: for an enum, would be nice to dump the string name ! */
	_ORBIT_HANDLE_TYPE (enum, long, int, 32, 4, "%d");
	_ORBIT_HANDLE_TYPE (ushort, unsigned_short, unsigned int, 16, 2, "%u");
	_ORBIT_HANDLE_TYPE (ulong, unsigned_long, unsigned int, 32, 4, "0x%x");
	_ORBIT_HANDLE_TYPE (boolean, boolean, int, 8, 1, "%d");
	_ORBIT_HANDLE_TYPE (char, char, int, 8, 1, "'%c'");
	_ORBIT_HANDLE_TYPE (wchar, wchar, int, 16, 2, "'%lc'");
	_ORBIT_HANDLE_TYPE (octet, octet, int, 8, 1, "0x%x");
	_ORBIT_HANDLE_TYPE (float, float, double, 32, 4, "%f");
	_ORBIT_HANDLE_TYPE (double, double, double, 64, 8, "%g");

	case CORBA_tk_any:
		ORBit_trace_any (*val);
		break;

	case CORBA_tk_objref:
		ORBit_trace_objref (*(CORBA_Object*)*val);
		break;

	case CORBA_tk_TypeCode:
		ORBit_trace_typecode (*(CORBA_TypeCode *)*val);
		break;

	case CORBA_tk_except:
	case CORBA_tk_struct:
		tprintf ("{ ");
		for (i = 0; i < tc->sub_parts; i++) {
			ORBit_trace_value (val, tc->subtypes [i], mi);
			if (i < tc->sub_parts - 1)
				tprintf (", ");
		}
		tprintf (" }");
		break;

	case CORBA_tk_union: {
		CORBA_TypeCode subtc;
		int            al = 0, sz = 0;
		gconstpointer  body, discrim;

		tprintf ("{ d=");

		ORBit_trace_value (val, tc->discriminator, mi);
		tprintf (" v=");

		discrim = *val;
		subtc = ORBit_get_union_tag (tc, &discrim, FALSE);
		for (i = 0; i < tc->sub_parts; i++) {
			al = MAX (al, ORBit_find_alignment (tc->subtypes [i]));
			sz = MAX (sz, ORBit_gather_alloc_info (tc->subtypes [i]));
		}
		body = *val = ALIGN_ADDRESS (*val, al);
		ORBit_trace_value (&body, subtc, mi);
		tprintf (" }");
		break;
	}
	case CORBA_tk_wstring:
		tprintf ("[wstring]");
		break;
	case CORBA_tk_string:
		tprintf ("'%s'", *(char **)*val);
		break;

	case CORBA_tk_sequence: {
		const CORBA_sequence_CORBA_octet *sval = *val;
		gconstpointer subval = sval->_buffer;

		tprintf ("seq[%d]={ ", sval->_length);

		for(i = 0; i < sval->_length; i++) {
			ORBit_trace_value (&subval, tc->subtypes[0], mi);
			if (i < sval->_length - 1)
				tprintf (", ");
		}
		tprintf (" }");
		break;
	}
	case CORBA_tk_array: {
		submi.alias_element_type = tc->subtypes[0];

		tprintf ("array[%d]={ ", tc->length);
		for(i = 0; i < tc->length; i++) {
			ORBit_trace_value (val, submi.alias_element_type, &submi);
			if (i < tc->length - 1)
				tprintf (", ");
		}
		tprintf (" }");
		break;
	}
	case CORBA_tk_longlong:
	case CORBA_tk_ulonglong:
	case CORBA_tk_longdouble:
	case CORBA_tk_fixed:
		tprintf ("wierd");
		break;
	case CORBA_tk_null:
		tprintf ("[null]");
		break;
	case CORBA_tk_void:
		tprintf ("[void]");
		break;
	default:
		g_error("Can't encode unknown type %d", tc->kind);
	}

	*val = ((guchar *)*val) + ORBit_gather_alloc_info (tc);
}
