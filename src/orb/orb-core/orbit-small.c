/*
 * Warning - before reading this file, and while hacking
 * it, it is advisable to examine:
 *
 *    docs/internals/interface-indirection.gnumeric
 *
 * FIXME: We need some global I/F -> m_data lookup action
 * FIXME: We need to map interface inheritance.
 * FIXME: Add #ifdef ORBIT_PURIFY support.
 *
 * FIXME: 'Obvious' optimizations
 *  Here:
 *    * 2 demarshalers - 1 straight, 1 endianness switching.
 *    * do more alloca's for basic things
 *    * more IDL compiler help for allocation and indirection
 *    decisions - these are closely tied
 *    * No alias types in the structures ... nice :-)
 *  Elsewhere:
 *    * store object profiles in GIOP format for fast marshaling
 *    * make locking more chunky _T everything.
 */

#include "config.h"
#include <orbit/orbit.h>
#include <orbit/GIOP/giop.h>
#include <stdio.h>
#include <string.h>
#include <gmodule.h>
#include <glib/garray.h>
#include "../poa/orbit-poa-export.h"
#include "orb-core-private.h"

#undef DEBUG
#undef TYPE_DEBUG
#undef TRACE_DEBUG

gpointer
ORBit_small_alloc (CORBA_TypeCode tc)
{
	size_t			element_size;
	ORBit_MemPrefix_TypeCode	*pre;
	gpointer 			mem;

	if ((element_size = ORBit_gather_alloc_info(tc)) == 0)
		return NULL;
	
	mem = ORBit_alloc_core (element_size,
				ORBIT_MEMHOW_TYPECODE | 1,
				sizeof(*pre), (gpointer)&pre,
				/*align*/0);

	memset (mem, 0, element_size);

	ORBIT_MEM_MAGICSET(pre->magic);

	pre->tc = ORBit_RootObject_duplicate(tc);
	
	return mem;
}

gpointer
ORBit_small_allocbuf (CORBA_TypeCode tc, CORBA_unsigned_long length)
{
	/* see above */
	return ORBit_alloc_tcval (tc->subtypes [0], length);
}

void
ORBit_small_freekids (CORBA_TypeCode tc, gpointer p, gpointer d)
{
	/* see above */
	ORBit_freekids_via_TypeCode (tc, p);
}


#ifndef DEBUG

static inline void dprintf (const char *format, ...) { };
#define dump_arg(a,b)
#define do_giop_dump_send(a)
#define do_giop_dump_recv(a)

#else /* DEBUG */

#define dprintf(format...) fprintf(stderr, format)
#define do_giop_dump_send(a) giop_dump_send(a)
#define do_giop_dump_recv(a) giop_dump_recv(a)

static void
dump_arg (const ORBit_IArg *a, CORBA_TypeCode tc)
{
	fprintf (stderr, " '%s' : kind - %d, %c%c",
		 a->name, tc->kind, a->flags & (ORBit_I_ARG_IN | ORBit_I_ARG_INOUT) ? 'i' : ' ',
		 a->flags & (ORBit_I_ARG_OUT | ORBit_I_ARG_INOUT) ? 'o' : ' ');
}

#endif /* DEBUG */

#ifndef TRACE_DEBUG
static inline void tprintf (const char *format, ...) { };
#define tprintf_trace_value(a,b,c)
#else
#define tprintf(format...) fprintf(stderr, format)
#define tprintf_trace_value(a,b,c) \
	ORBit_trace_value ((gconstpointer *)(a), (b), (c))
#endif

static void
ORBit_handle_exception_array (GIOPRecvBuffer     *rb,
			      CORBA_Environment  *ev,
			      const ORBit_ITypes *types,
			      CORBA_ORB           orb)
{
	CORBA_SystemException *new;
	CORBA_unsigned_long len, completion_status, reply_status;
	CORBA_char *my_repoid;

	g_return_if_fail (rb->msg.header.message_type == GIOP_REPLY);

	CORBA_exception_free (ev);

	rb->cur = ALIGN_ADDRESS (rb->cur, sizeof (len));
	if ((rb->cur + 4) > rb->end)
		goto errout;

	len = *(CORBA_unsigned_long *)rb->cur;
	rb->cur += 4;
	if (giop_msg_conversion_needed (rb))
		len = GUINT32_SWAP_LE_BE (len);

	if (len) {
		my_repoid = rb->cur;
		rb->cur += len;
	} else
		my_repoid = NULL;

	reply_status = giop_recv_buffer_reply_status (rb);

	dprintf ("Received exception %d: '%s'\n",
		 reply_status, my_repoid ? my_repoid : "<Null>");

	if (reply_status == CORBA_SYSTEM_EXCEPTION) {
		CORBA_unsigned_long minor;

		dprintf ("system exception\n");
		
		ev->_major = CORBA_SYSTEM_EXCEPTION;

		rb->cur = ALIGN_ADDRESS (rb->cur, sizeof (minor));
		if ((rb->cur + sizeof (minor)) > rb->end)
			goto errout;
		minor = *(CORBA_unsigned_long *) rb->cur;
		rb->cur += 4;
		if (giop_msg_conversion_needed (rb))
			minor = GUINT32_SWAP_LE_BE (minor);

		rb->cur = ALIGN_ADDRESS (rb->cur, sizeof (completion_status));
		if ((rb->cur + sizeof (completion_status)) > rb->end)
			goto errout;
		completion_status = *(CORBA_unsigned_long *) rb->cur;
		rb->cur += 4;
		if (giop_msg_conversion_needed (rb))
			completion_status = GUINT32_SWAP_LE_BE (completion_status);

		new = CORBA_SystemException__alloc ();
		new->minor = minor;
		new->completed = completion_status;
			
		/* FIXME: check what should the repo ID be? */
		CORBA_exception_set (ev, CORBA_SYSTEM_EXCEPTION,
				     my_repoid, new);

		dprintf ("system exception de-marshaled\n");
		return;

	} else if (reply_status == CORBA_USER_EXCEPTION) {
		int i;

		dprintf ("user exception\n");

		for (i = 0; i < types->_length; i++) {
			if (!strcmp (types->_buffer[i]->repo_id, my_repoid))
				break;
		}

		if (!types || types->_length == 0) {
			/* weirdness; they raised an exception that we don't
			   know about */
			CORBA_exception_set_system (
				ev, ex_CORBA_MARSHAL,
				CORBA_COMPLETED_MAYBE);
		} else {
			gpointer data;
			
			dprintf ("de-marshal user exception\n");

			data = ORBit_demarshal_arg (
				rb, types->_buffer [i], TRUE, orb);

			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     types->_buffer [i]->repo_id, data);
		}
		return;
	}
	return;
  
 errout:
	/* ignore LOCATION_FORWARD here, that gets handled in the stub */
	CORBA_exception_set_system (ev, ex_CORBA_MARSHAL,
				    CORBA_COMPLETED_MAYBE);
}

static gboolean
ORBit_small_send_user_exception (GIOPSendBuffer     *send_buffer,
				 CORBA_Environment  *ev,
				 const ORBit_ITypes *types)
{
	int i;

	for (i = 0; i < types->_length; i++) {
		if(!strcmp (types->_buffer[i]->repo_id, ev->_id))
			break;
	}

	if (i >= types->_length) {
		g_warning ("Some clown returned undeclared "
			   "exception '%s' ", ev->_id);

		CORBA_exception_free (ev);
		CORBA_exception_set_system (
			ev, ex_CORBA_UNKNOWN,
			CORBA_COMPLETED_MAYBE);

		giop_send_buffer_unuse (send_buffer);

		return FALSE;
	} else {
		CORBA_unsigned_long len = strlen (ev->_id) + 1;

		giop_send_buffer_append_aligned (send_buffer, &len, sizeof (len));
		giop_send_buffer_append (send_buffer, ev->_id, len);

		dprintf ("Returning exception of type '%s'\n", ev->_id);

		ORBit_marshal_arg (send_buffer, ev->_any._value,
				   types->_buffer[i]);

		return TRUE;
	}
}

static void
ORBit_small_marshal_context (GIOPSendBuffer *send_buffer,
			     ORBit_IMethod  *m_data,
			     CORBA_Context   ctx)
{
	if (m_data->contexts._length > 0) {
		int i;
		/* Horrible inefficiency to get round the 'lete
		   efficiency of the current impl */
		ORBit_ContextMarshalItem *mlist;

		tprintf (" context { [impl. me] } ");

		mlist = alloca (sizeof (ORBit_ContextMarshalItem) *
				m_data->contexts._length);

		for (i = 0; i < m_data->contexts._length; i++) {
			mlist [i].str = m_data->contexts._buffer [i];
			mlist [i].len = strlen (mlist [i].str) + 1;
		}
		/* Assumption, this doesn't whack mlist pointers into
		   the send_buffer: verified */
		ORBit_Context_marshal (
			ctx, mlist, m_data->contexts._length, send_buffer);
	}
}

#define BASE_TYPES \
	     CORBA_tk_short: \
	case CORBA_tk_long: \
	case CORBA_tk_enum: \
	case CORBA_tk_ushort: \
	case CORBA_tk_ulong: \
	case CORBA_tk_float: \
	case CORBA_tk_double: \
	case CORBA_tk_boolean: \
	case CORBA_tk_char: \
	case CORBA_tk_octet: \
	case CORBA_tk_longlong: \
	case CORBA_tk_ulonglong: \
	case CORBA_tk_longdouble: \
	case CORBA_tk_wchar

#define STRUCT_UNION_TYPES \
	     CORBA_tk_struct: \
	case CORBA_tk_union: \
	case CORBA_tk_except

#define OBJ_STRING_TYPES \
	     CORBA_tk_objref: \
	case CORBA_tk_TypeCode: \
	case CORBA_tk_string: \
	case CORBA_tk_wstring

#define SEQ_ANY_TYPES \
	     CORBA_tk_sequence: \
	case CORBA_tk_any

typedef struct {
	CORBA_unsigned_long len;
	char                opname[1];
} OpData;

#define do_marshal_value(a,b,c,d)     \
	ORBit_marshal_value   ((a),(gconstpointer *)(b),(c),(d))
#define do_demarshal_value(a,b,c,e) \
	ORBit_demarshal_value ((c),(b),(a),TRUE,(e))

static gboolean
orbit_small_marshal (CORBA_Object           obj,
		     GIOPConnection        *cnx,
		     GIOPMessageQueueEntry *mqe,
		     CORBA_unsigned_long    request_id,
		     ORBit_IMethod         *m_data,
		     gpointer              *args,
		     CORBA_Context          ctx)
{
	GIOPSendBuffer          *send_buffer;
	struct iovec             op_vec;
	CORBA_TypeCode           tc;
	int                      i;
	ORBit_marshal_value_info mi;

	{
		int          align;
		int          len = sizeof (CORBA_unsigned_long) + m_data->name_len + 1;
		guchar      *header = alloca (len + sizeof (CORBA_unsigned_long));

		*(CORBA_unsigned_long *) header = m_data->name_len + 1;
		memcpy (header + sizeof (CORBA_unsigned_long),
			m_data->name, m_data->name_len + 1);
	       
		align = len + (sizeof (CORBA_unsigned_long) - 1);
		align &= ~(sizeof (CORBA_unsigned_long) - 1);
		memset (header + len, 0, align - len);

		dprintf ("Align = %d\n", align);
		op_vec.iov_len  = align;
		op_vec.iov_base = header;
	}

	send_buffer = giop_send_buffer_use_request (
		cnx->giop_version, request_id, CORBA_TRUE,
		&(obj->oki->object_key_vec), &op_vec, NULL);

	if (!send_buffer)
		return FALSE;

	dprintf ("Marshal: id 0x%x\n", request_id);

	for (i = 0; (m_data->arguments._buffer &&
		     m_data->arguments._buffer [i].flags); i++) {

		ORBit_IArg *a = &m_data->arguments._buffer [i];
		gpointer    p;

		if (!(a->flags & (ORBit_I_ARG_IN |
				  ORBit_I_ARG_INOUT)))
			continue;
		tc = a->tc;

		while (tc->kind == CORBA_tk_alias)
			tc = tc->subtypes [0];

		dump_arg (a, tc);

		p = args [i];
		do_marshal_value (send_buffer, &p, tc, &mi);

		p = args [i];
		tprintf_trace_value (&p, tc, &mi);

		if (m_data->arguments._buffer [i+1].flags)
			tprintf (", ");

		dprintf ("\n");
	}

	tprintf (")");

	ORBit_small_marshal_context (send_buffer, m_data, ctx);

	do_giop_dump_send (send_buffer);

	giop_recv_list_setup_queue_entry (mqe, cnx, GIOP_REPLY, request_id);

	if (giop_send_buffer_write (send_buffer, cnx)) {
		giop_recv_list_destroy_queue_entry (mqe);
		return FALSE;
	}

	giop_send_buffer_unuse (send_buffer);

	return TRUE;
}

typedef enum {
	MARSHAL_SYS_EXCEPTION_INCOMPLETE,
	MARSHAL_SYS_EXCEPTION_COMPLETE,
	MARSHAL_EXCEPTION_COMPLETE,
	MARSHAL_RETRY,
	MARSHAL_CLEAN
} DeMarshalRetType;

static DeMarshalRetType
orbit_small_demarshal (CORBA_Object           obj,
		       GIOPConnection       **cnx,
		       GIOPMessageQueueEntry *mqe,
		       CORBA_Environment     *ev,
		       gpointer               ret,
		       ORBit_IMethod         *m_data,
		       gpointer              *args)
{
	gpointer        data;
	CORBA_TypeCode  tc;
	GIOPRecvBuffer *recv_buffer;
	CORBA_ORB       orb = obj->orb;

	recv_buffer = giop_recv_buffer_get (mqe, TRUE);
	if (!recv_buffer) {
		dprintf ("No recv buffer ...\n");
		return MARSHAL_SYS_EXCEPTION_INCOMPLETE;
	}

	do_giop_dump_recv (recv_buffer);

	if (giop_recv_buffer_reply_status (recv_buffer) != GIOP_NO_EXCEPTION)
 		goto msg_exception;

	dprintf ("Demarshal ");

	tprintf (" => ");

	if ((tc = m_data->ret)) {
		dprintf ("ret: ");

		g_assert (ret != NULL);

		while (tc->kind == CORBA_tk_alias)
			tc = tc->subtypes [0];

		switch (tc->kind) {
		case BASE_TYPES:
		case OBJ_STRING_TYPES:
			do_demarshal_value (recv_buffer, &ret, tc, orb);
/*			tprintf_trace_value (&ret, tc, NULL); */
			dprintf ("base type / [p]obj / [w]string");
			break;

		case STRUCT_UNION_TYPES:
			if (m_data->flags & ORBit_I_COMMON_FIXED_SIZE) {
				do_demarshal_value (recv_buffer, &ret, tc, orb);
				dprintf ("misc. fixed");
				break;
			} /* drop through */

		case SEQ_ANY_TYPES:
		case CORBA_tk_array:
		default:
			data = ORBit_demarshal_arg (recv_buffer, tc, TRUE, orb);
			if (!data)
				return MARSHAL_SYS_EXCEPTION_COMPLETE;
			*((gpointer *)ret) = data;
			dprintf ("misc pointer + alloc");
			break;
		}
		dprintf ("\n");
	}

	if (m_data->arguments._buffer)
		dprintf ("args:\n");

	{
		int i;
		for (i = 0; (m_data->arguments._buffer &&
			     m_data->arguments._buffer [i].flags); i++) {
			const ORBit_IArg *a;
			gpointer          arg;

			a = &m_data->arguments._buffer [i];

			if (!(a->flags & (ORBit_I_ARG_OUT |
					  ORBit_I_ARG_INOUT)))
				continue;

			tc = a->tc;

			while (tc->kind == CORBA_tk_alias)
				tc = tc->subtypes [0];

			dump_arg (a, tc);

			if (a->flags & ORBit_I_ARG_OUT)
				arg = *(gpointer *)args [i];
			else
				arg = args [i];

			switch (tc->kind) {
			case OBJ_STRING_TYPES: {
				if (a->flags & ORBit_I_ARG_INOUT) {
					if (tc->kind == CORBA_tk_TypeCode ||
					    tc->kind == CORBA_tk_objref)
						CORBA_Object_release (*(gpointer *)arg, ev);
					else if (tc->kind == CORBA_tk_string ||
						 tc->kind == CORBA_tk_wstring)
						CORBA_free (*(gpointer *) arg);
				}
				/* drop through */
			case BASE_TYPES:
				do_demarshal_value (
					recv_buffer, &arg, tc, orb);
				dprintf ("base / allocated type");
				break;
			}

			case STRUCT_UNION_TYPES:
			case SEQ_ANY_TYPES:
			case CORBA_tk_array:
			default:
				if (a->flags & ORBit_I_COMMON_FIXED_SIZE) {
					do_demarshal_value (recv_buffer, &arg, tc, orb);
					break;
				} else if (a->flags & ORBit_I_ARG_INOUT) {
					ORBit_freekids_via_TypeCode (tc, arg);
					do_demarshal_value (recv_buffer, &arg, tc, orb);
				} else
					*(gpointer *)args [i] = ORBit_demarshal_arg (
						recv_buffer, tc, TRUE, obj->orb);
				break;
			}
			dprintf ("\n");
		}
	}
	
	giop_recv_buffer_unuse (recv_buffer);
	return MARSHAL_CLEAN;

 msg_exception:
	if (giop_recv_buffer_reply_status (recv_buffer) ==
	    GIOP_LOCATION_FORWARD) {
		
		*cnx = ORBit_handle_location_forward (recv_buffer, obj);

		return MARSHAL_RETRY;
	} else {
		ORBit_handle_exception_array (
			recv_buffer, ev, &m_data->exceptions, obj->orb);
		giop_recv_buffer_unuse (recv_buffer);

		return MARSHAL_EXCEPTION_COMPLETE;
	}
}

void
ORBit_small_invoke_stub (CORBA_Object       obj,
			 ORBit_IMethod     *m_data,
			 gpointer           ret,
			 gpointer          *args,
			 CORBA_Context      ctx,
			 CORBA_Environment *ev)
{
	CORBA_unsigned_long     request_id;
	CORBA_completion_status completion_status;
	GIOPConnection         *cnx;
	GIOPMessageQueueEntry   mqe;
	ORBit_OAObject          adaptor_obj = obj->adaptor_obj;

	if (adaptor_obj) {
		ORBit_small_handle_request (adaptor_obj, m_data->name, ret,
					    args, ctx, NULL, ev);
		goto clean_out;
	}

	cnx = ORBit_object_get_connection (obj);

	if (!cnx) {
		dprintf ("Null connection on object '%p'\n", obj);
		completion_status = CORBA_COMPLETED_NO;
		goto system_exception;
	}

#ifdef TRACE_DEBUG
	tprintf ("ORBit2: (");
	ORBit_trace_objref (obj);
	tprintf (")->%s (", m_data->name);
#endif

retry_request:
	request_id = GPOINTER_TO_UINT (&obj);
	completion_status = CORBA_COMPLETED_NO;

	if (!orbit_small_marshal (obj, cnx, &mqe, request_id,
				  m_data, args, ctx))
		goto system_exception;

	completion_status = CORBA_COMPLETED_MAYBE;

	if (m_data->flags & ORBit_I_METHOD_1_WAY)
		goto clean_out;

	switch (orbit_small_demarshal (obj, &cnx, &mqe, ev,
				       ret, m_data, args))
	{
	case MARSHAL_SYS_EXCEPTION_COMPLETE:
		completion_status = CORBA_COMPLETED_YES;
		dprintf ("Sys exception completed on id 0x%x\n\n", request_id);
		goto system_exception;

	case MARSHAL_SYS_EXCEPTION_INCOMPLETE:
		dprintf ("Sys exception incomplete on id 0x%x\n\n", request_id);
		goto system_exception;

	case MARSHAL_EXCEPTION_COMPLETE:
		dprintf ("Clean demarshal of exception on id 0x%x\n\n", request_id);
		break;

	case MARSHAL_RETRY:
		dprintf ("Retry demarshal on id 0x%x\n\n", request_id);
		goto retry_request;

	case MARSHAL_CLEAN:
		dprintf ("Clean demarshal on id 0x%x\n\n", request_id);
		break;
	};
 clean_out:
	tprintf ("\n");
	return;

 system_exception:
	tprintf ("[System exception comm failure] )");
	CORBA_exception_set_system (ev, ex_CORBA_COMM_FAILURE,
				    completion_status);
	goto clean_out;
}

void
ORBit_small_invoke_adaptor (ORBit_OAObject     adaptor_obj,
			    GIOPRecvBuffer    *recv_buffer,
			    ORBit_IMethod     *m_data,
			    gpointer           data,
			    CORBA_Environment *ev)
{
	struct CORBA_Context_type  ctx;
	gpointer                  *args = NULL;
	gpointer                  *scratch = NULL;
	gpointer                   pretval = NULL;
	gpointer                   retval = NULL;
	GIOPSendBuffer            *send_buffer;
	CORBA_ORB                  orb;
	CORBA_TypeCode             tc;
	gboolean                   has_context;
	int                        i;

	do_giop_dump_recv (recv_buffer);

	orb = adaptor_obj->objref->orb;

	has_context = (m_data->contexts._length > 0);

	if ((tc = m_data->ret)) {
		
		while (tc->kind == CORBA_tk_alias)
			tc = tc->subtypes [0];

		switch (tc->kind) {
		case BASE_TYPES:
		case OBJ_STRING_TYPES:
			retval = alloca (ORBit_gather_alloc_info (tc));
			break;
		case STRUCT_UNION_TYPES:
			if (m_data->flags & ORBit_I_COMMON_FIXED_SIZE) {
				retval = ORBit_alloc_tcval (tc, 1);
				break;
			} /* drop through */
		default:
			retval = &pretval;
			pretval = NULL;
			break;
		}
	}

	if (m_data->arguments._length > 0) {
		int len = m_data->arguments._length *
			sizeof (gpointer);

		args = alloca (len);
		scratch = alloca (len);
	}		

	for (i = 0; i < m_data->arguments._length; i++) {
		ORBit_IArg *a = &m_data->arguments._buffer [i];
		
		if (a->flags & ORBit_I_ARG_IN ||
		    a->flags & ORBit_I_ARG_INOUT) {
			gpointer p;

			tc = a->tc;

			while (tc->kind == CORBA_tk_alias)
				tc = tc->subtypes [0];

			switch (tc->kind) {
			case BASE_TYPES:
			case OBJ_STRING_TYPES:
				p = args [i] = alloca (ORBit_gather_alloc_info (tc));
				do_demarshal_value (recv_buffer, &p, tc, orb);
				break;
			case STRUCT_UNION_TYPES:
			case CORBA_tk_array:
				if (m_data->flags & ORBit_I_COMMON_FIXED_SIZE) {
					p = args [i] = alloca (ORBit_gather_alloc_info (tc));
					do_demarshal_value (recv_buffer, &p, tc, orb);
					break;
				} /* drop through */
			default:
				args [i] = ORBit_demarshal_arg (
					recv_buffer, a->tc, TRUE, orb);
				break;
			}

		} else { /* Out */
			tc = a->tc;

			while (tc->kind == CORBA_tk_alias)
				tc = tc->subtypes [0];

			args [i] = &scratch [i];

			switch (tc->kind) {
			case BASE_TYPES:
			case OBJ_STRING_TYPES:
				scratch [i] = alloca (ORBit_gather_alloc_info (tc));
				break;
			case STRUCT_UNION_TYPES:
			case CORBA_tk_array:
				if (m_data->flags & ORBit_I_COMMON_FIXED_SIZE) {
					scratch [i] = ORBit_alloc_tcval (tc, 1);
					break;
				} /* drop through */
			default:
				scratch [i] = NULL;
				break;
			}
		}
	}

	if (has_context) {
		if (ORBit_Context_demarshal (NULL, &ctx, recv_buffer))
			g_warning ("FIXME: handle context demarshaling failure");
	}

	ORBit_OAObject_invoke (adaptor_obj, retval, args, &ctx, data, ev);

	if (has_context)
		ORBit_Context_server_free (&ctx);

	if (m_data->flags & ORBit_I_METHOD_1_WAY)
		goto clean_out;

 sys_exception:
	send_buffer = giop_send_buffer_use_reply (
		recv_buffer->connection->giop_version,
		giop_recv_buffer_get_request_id (recv_buffer),
		ev->_major);

	if (!send_buffer) {
		dprintf ("Weird, no send_buffer");
		return;

	} else if (ev->_major == CORBA_USER_EXCEPTION) {
		if (!ORBit_small_send_user_exception (
			send_buffer, ev, &m_data->exceptions)) {
			/* Tried to marshal an unknown exception,
			   so we throw a system exception next */
			dprintf ("Re-sending an exception, this time %d: '%s'",
				 ev->_major, ev->_id);
			goto sys_exception;
		}

	} else if (ev->_major != CORBA_NO_EXCEPTION)
		ORBit_send_system_exception (send_buffer, ev);
	
	else { /* Marshal return values */
		if ((tc = m_data->ret)) {
			dprintf ("ret: ");

			while (tc->kind == CORBA_tk_alias)
				tc = tc->subtypes [0];

			switch (tc->kind) {
				
			case BASE_TYPES:
			case OBJ_STRING_TYPES:
				ORBit_marshal_arg (send_buffer, retval, m_data->ret);
				dprintf ("base/[p]obj/string");
				break;

			case STRUCT_UNION_TYPES:
				if (m_data->flags & ORBit_I_COMMON_FIXED_SIZE) {
					ORBit_marshal_arg (send_buffer, retval, m_data->ret);
					dprintf ("fixed");
					break;
				} /* drop through */

			case CORBA_tk_any:
			case CORBA_tk_sequence:
			case CORBA_tk_array:
			default:
				ORBit_marshal_arg (send_buffer, *(gpointer *)retval, m_data->ret);
				dprintf ("pointer baa");
				break;
			}
			dprintf ("\n");
		}

		for (i = 0; i < m_data->arguments._length; i++) {
			ORBit_IArg *a = &m_data->arguments._buffer [i];
			
			tc = a->tc;
			while (tc->kind == CORBA_tk_alias)
				tc = tc->subtypes [0];
			
			if (a->flags & ORBit_I_ARG_INOUT)
				ORBit_marshal_arg (send_buffer, args [i], tc);

			else if (a->flags & ORBit_I_ARG_OUT)
				ORBit_marshal_arg (send_buffer, scratch [i], tc);
		}
	}

	do_giop_dump_send (send_buffer);

	giop_send_buffer_write (send_buffer, recv_buffer->connection);
	giop_send_buffer_unuse (send_buffer);

	if (m_data->ret && ev->_major == CORBA_NO_EXCEPTION) {
		switch (m_data->ret->kind) {
		case BASE_TYPES:
			break;
		case CORBA_tk_objref:
		case CORBA_tk_TypeCode:
			CORBA_Object_release (*(CORBA_Object *) retval, ev);
			break;
		case CORBA_tk_string:
		case CORBA_tk_wstring:
			CORBA_free (*(char **) retval);
			break;
		case STRUCT_UNION_TYPES:
			if (m_data->flags & ORBit_I_COMMON_FIXED_SIZE) {
				CORBA_free (retval);
				break;
			} /* drop through */
		default:
			CORBA_free (pretval);
			break;
		}
	}

 clean_out:
	for (i = 0; i < m_data->arguments._length; i++) {
		ORBit_IArg *a = &m_data->arguments._buffer [i];
		
		tc = a->tc;
			
		while (tc->kind == CORBA_tk_alias)
			tc = tc->subtypes [0];
			
		if (a->flags & ORBit_I_ARG_IN ||
		    a->flags & ORBit_I_ARG_INOUT) {
			
			switch (tc->kind) {
			case BASE_TYPES:
				break;
			case CORBA_tk_objref:
			case CORBA_tk_TypeCode:
				CORBA_Object_release (*(CORBA_Object *) args [i], ev);
				break;
			case CORBA_tk_string:
			case CORBA_tk_wstring:
				CORBA_free (*(char **) args [i]);
				break;
			case STRUCT_UNION_TYPES:
			case CORBA_tk_array:
				if (m_data->flags & ORBit_I_COMMON_FIXED_SIZE) {
					ORBit_freekids_via_TypeCode (tc, args [i]);
					break;
				}
				/* drop through */
			default:
				CORBA_free (args [i]);
				break;
			}
		} else if (ev->_major == CORBA_NO_EXCEPTION) { /* Out */
			switch (tc->kind) {
			case BASE_TYPES:
				break;
			case CORBA_tk_objref:
			case CORBA_tk_TypeCode:
				CORBA_Object_release (*(CORBA_Object *) scratch [i], ev);
				break;
			case CORBA_tk_string:
			case CORBA_tk_wstring:
				CORBA_free (*(char **) scratch [i]);
				break;
			default:
				CORBA_free (scratch [i]);
				break;
			}
		}
	}
	
	CORBA_exception_free (ev);
}

#ifdef DEBUG
gpointer
ORBit_small_getepv (CORBA_Object obj, CORBA_unsigned_long class_id)
{
	PortableServer_ServantBase *servant;
	PortableServer_ClassInfo   *class_info;
	CORBA_unsigned_long         offset;
	ORBit_POAObject             pobj;

	if (obj->adaptor_obj->interface->adaptor_type != ORBIT_ADAPTOR_POA)
		return NULL;

	pobj        = (ORBit_POAObject)obj->adaptor_obj;
	servant     = pobj->servant;
	class_info  = servant->vepv[0]->_private;
	g_assert (class_info != NULL);
	g_assert (class_id < class_info->vepvlen);
	offset     = class_info->vepvmap [class_id];

	return servant->vepv [offset];
}
#endif

ORBit_IInterface *
ORBit_iinterface_from_idl (IDL_tree idl)
{
	g_warning ("Implement me");

	return NULL;
}

CORBA_char *
ORBit_small_get_type_id (CORBA_Object       object,
			 CORBA_Environment *ev)
{
	ORBit_OAObject adaptor_obj = object->adaptor_obj;

	/* We stay always maximaly qualified localy */
	if (adaptor_obj && adaptor_obj->interface->is_active (adaptor_obj))
		return CORBA_string_dup (object->type_id);

	else {
		CORBA_char *retval = NULL;

		ORBit_small_invoke_stub (
			object,
			&CORBA_Object__imethods [
				CORBA_OBJECT_SMALL_GET_TYPE_ID],
			&retval, NULL, NULL, ev);

		return retval;
	}
}

static GHashTable *interfaces = NULL;

static GHashTable *
get_type_db (void)
{
	if (!interfaces)
		/* FIXME: need a g_atexit free */
		interfaces = g_hash_table_new (
			g_str_hash, g_str_equal);

	return interfaces;
}

static ORBit_IInterface *
lookup_iinterface (const CORBA_char *type_id)
{
	GHashTable *db = get_type_db ();

	return g_hash_table_lookup (db, type_id);
}

static void
add_iinterface (ORBit_IInterface *idata)
{
	GHashTable *db = get_type_db ();

	g_hash_table_insert (db, idata->tc->repo_id, idata);
}

static ORBit_IInterface *
copy_iinterface (const ORBit_IInterface *idata, gboolean shallow)
{
	/* FIXME: we deep copy always for now - we should speed this up */
	/* FIXME: we need to set a flag here */
	return ORBit_copy_value (idata, TC_ORBit_IInterface);
}

static GSList *type_list = NULL;

typedef struct {
	char *name;
	CORBA_sequence_CORBA_TypeCode *types;
	CORBA_sequence_ORBit_IInterface *iinterfaces;
} TypeList;

static void
add_types (const char *libname,
	   CORBA_sequence_CORBA_TypeCode *types,
	   CORBA_sequence_ORBit_IInterface *iinterfaces)
{
	TypeList *tl = g_new0 (TypeList, 1);

	tl->name = g_strdup (libname);
	tl->types = types;
	tl->iinterfaces = iinterfaces;

	/* FIXME: some g_atexit free loving ? */
	type_list = g_slist_prepend (type_list, tl);
}

static CORBA_sequence_CORBA_TypeCode *
get_types (const char *module_name)
{
	GSList *l;

	for (l = type_list; l; l = l->next) {
		TypeList *tl = l->data;

		if (!strcmp (tl->name, module_name)) {
			CORBA_sequence_CORBA_TypeCode *st;

			st = CORBA_sequence_CORBA_TypeCode__alloc ();
			*st = *tl->types;
			st->_release = FALSE;

			return st;
		}
	}

	return NULL;
}

static CORBA_sequence_ORBit_IInterface *
get_iinterfaces (const char *module_name)
{
	GSList *l;

	for (l = type_list; l; l = l->next) {
		TypeList *tl = l->data;

		if (!strcmp (tl->name, module_name)) {
			CORBA_sequence_ORBit_IInterface *st;

			st = CORBA_sequence_ORBit_IInterface__alloc ();
			*st = *tl->iinterfaces;
			st->_release = FALSE;

			return st;
		}
	}

	return NULL;
}

ORBit_IInterface *
ORBit_small_get_iinterface (CORBA_Object       opt_object,
			    const CORBA_char  *type_id,
			    CORBA_Environment *ev)
{
	ORBit_IInterface *retval;
	PortableServer_ClassInfo *ci;

	if ((retval = lookup_iinterface (type_id)))
		retval = copy_iinterface (retval, TRUE);

	else if ((ci = ORBit_classinfo_lookup (type_id))) {
		retval = copy_iinterface (ci->idata, TRUE);

	} else if (opt_object) {
		/* FIXME: first walk the object's data,
		   if local, we might have unregistered
		   interfaces for some reason */
		gpointer args [1];

		args [0] = &type_id;

		ORBit_small_invoke_stub (
			opt_object,
			&CORBA_Object__imethods [
				CORBA_OBJECT_SMALL_GET_IINTERFACE],
			&retval, args, NULL, ev);

		if (retval != CORBA_OBJECT_NIL) {
			ORBit_IInterface *cache;
			cache = copy_iinterface (retval, FALSE);
			add_iinterface (cache);
		}
	}

	if (!retval &&
	    ev->_major == CORBA_NO_EXCEPTION)
		CORBA_exception_set (
			ev, CORBA_USER_EXCEPTION,
			ex_ORBit_NoIInterface, NULL);

/*	g_warning ("get_iinterface returns '%s' %p",
		   retval ? retval->tc->repo_id : ev->_id,
		   retval); */

	return retval;
}

static void
add_if_unique (GPtrArray  *strings,
	       const char *new_str)
{
	int i, len;

	len = strlen (new_str);

	for (i = 0; i < strings->len; i++) {
		if (!strncmp (g_ptr_array_index (strings, i),
			      new_str, len))
			return;
	}

	g_ptr_array_add (
		strings,
		g_strconcat (new_str, "/lib/orbit", NULL));
}

/* FIXME: this should be called only once at
   ORB init time really */
static char **
get_typelib_paths (void)
{
	const char *path;
	int         i;
	GPtrArray  *paths;

	paths = g_ptr_array_sized_new (8);

	g_ptr_array_add (paths, g_strdup (ORBIT_TYPELIB_DIR));

	if ((path = getenv ("ORBIT_TYPELIB_PATH"))) {
		char **strv;

		strv = g_strsplit (path, ":", -1);
		for (i = 0; strv && strv [i]; i++)
			add_if_unique (paths, strv [i]);
		g_strfreev (strv);
	}

	if ((path = getenv ("GNOME_PATH"))) {
		char **strv;

		strv = g_strsplit (path, ":", -1);
		for (i = 0; strv && strv [i]; i++)
			add_if_unique (paths, strv [i]);
		g_strfreev (strv);
	}

	g_ptr_array_add (paths, NULL);

	return (char **)g_ptr_array_free (paths, FALSE);
}

static gboolean
load_module (const char *fname, const char *libname)
{
	GModule *handle;
	ORBit_IModule *module;

	if (!(handle = g_module_open (fname, G_MODULE_BIND_LAZY))) {
#ifdef TYPE_DEBUG
		g_warning ("Failed to load '%s': '%s'", fname,
			   g_module_error ());
#endif
		return FALSE;
	}

	else if (!g_module_symbol (handle, "orbit_imodule_data",
				     (gpointer *)&module)) {
		g_warning ("type library '%s' has no stored types", fname);
		g_module_close (handle);

		return FALSE;
	} else {
		CORBA_sequence_ORBit_IInterface *iinterfaces;
		CORBA_sequence_CORBA_TypeCode *types;
		ORBit_IInterface **p;
		gulong length, i;

#ifdef TYPE_DEBUG
		g_warning ("Loaded interfaces of serial %d from '%s'",
			   module->version, fname);
#endif

		for (p = module->interfaces, length = 0; p && *p; p++)
			length++;

		iinterfaces = CORBA_sequence_ORBit_IInterface__alloc ();
		iinterfaces->_length = iinterfaces->_maximum = length;
		iinterfaces->_buffer = CORBA_sequence_ORBit_IInterface_allocbuf (length);
		iinterfaces->_release = CORBA_TRUE;

		for (i = 0; i < length; i++) {
			ORBit_IInterface *src, *dest;

			src = module->interfaces [i];
			dest = &iinterfaces->_buffer [i];

			ORBit_copy_value_core ((gconstpointer *) &src,
					       (gpointer *) &dest,
					       TC_ORBit_IInterface);
			dest = &iinterfaces->_buffer [i];
			add_iinterface (dest);

#ifdef TYPE_DEBUG
			g_warning ("Type '%s'", dest->tc->repo_id);
#endif
		}

		types = ORBit_copy_value (
			&module->types, TC_CORBA_sequence_CORBA_TypeCode);


		add_types (libname, types, iinterfaces);

		/* FIXME: before we can close this,
		   we need to deep copy typecodes - that is if
		   in fact we want to close it ? */
/*		g_module_close (handle); */

		return TRUE;
	}
}


gboolean
ORBit_small_load_typelib (const char *libname)
{
	gboolean loaded = FALSE;
	char    *fname = NULL;

	g_return_val_if_fail (libname != NULL, FALSE);

	if (libname [0] != '/' &&
	    !(libname [0] == '.' &&
	      libname [1] == '/')) {
		char   **paths;
		int      i;

		paths = get_typelib_paths ();

		for (i = 0; paths && paths [i]; i++) {
			fname = g_strconcat (
				paths [i], "/", libname, "_module", NULL);

			if ((loaded = load_module (fname, libname)))
				break;

			else {
				g_free (fname);
				fname = NULL;
			}
		}

		g_strfreev (paths);
	} else
		loaded = load_module (libname, libname);

	return loaded;
}

CORBA_sequence_CORBA_TypeCode *
ORBit_small_get_types (const char *name)
{
	return get_types (name);
}

CORBA_sequence_ORBit_IInterface *
ORBit_small_get_iinterfaces (const char *name)
{
	return get_iinterfaces (name);
}
