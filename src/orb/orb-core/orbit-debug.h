#ifndef __ORBIT_DEBUG_H__
#define __ORBIT_DEBUG_H__

/*
 * Flip this switch to dump GIOP messages
 * as they are sent and received.
 */
#undef DEBUG

/*
 * Flip this switch to get a nice trace
 * of method invocations.
 */
#undef TRACE_DEBUG

/*
 * Flip this switch if you want tyeplib
 * related warnings.
 */
#undef TYPE_DEBUG



#ifndef DEBUG

static inline void dprintf (const char *format, ...) { };
#define dump_arg(a,b)
#define do_giop_dump_send(a)
#define do_giop_dump_recv(a)

#else /* DEBUG */

#define dprintf(format...) fprintf(stderr, format)
#define do_giop_dump_send(a) giop_dump_send(a)
#define do_giop_dump_recv(a) giop_dump_recv(a)

static inline void
dump_arg (const ORBit_IArg *a, CORBA_TypeCode tc)
{
	fprintf (stderr, " '%s' : kind - %d, %c%c",
		 a->name, tc->kind, 
		 a->flags & (ORBit_I_ARG_IN | ORBit_I_ARG_INOUT) ? 'i' : ' ',
		 a->flags & (ORBit_I_ARG_OUT | ORBit_I_ARG_INOUT) ? 'o' : ' ');
}

#endif /* DEBUG */



#ifndef TRACE_DEBUG

static inline void tprintf (const char *format, ...) { };
#define tprintf_trace_value(a,b)

#else /* TRACE_DEBUG */

void     ORBit_trace_objref   (const CORBA_Object   obj);
void     ORBit_trace_any      (const CORBA_any     *any);
void     ORBit_trace_typecode (const CORBA_TypeCode tc);
void     ORBit_trace_value    (gconstpointer       *val,
                               CORBA_TypeCode       tc);

#define tprintf(format...) fprintf(stderr, format)
#define tprintf_trace_value(a,b) \
		ORBit_trace_value ((gconstpointer *)(a), (b))

#endif /* TRACE_DEBUG */

#endif /* __ORBIT_DEBUG_H__ */
