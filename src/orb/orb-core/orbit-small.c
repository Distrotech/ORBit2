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

#include "../poa/orbit-poa-export.h"

#undef DEBUG
#define DEBUG_LOCAL_TEST

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
#define giop_dump_send(a)
#define giop_dump_recv(a)

#else /* DEBUG */

#define dprintf(format...) fprintf(stderr, format)

static void
dump_arg (const ORBit_IArg *a, CORBA_TypeCode tc)
{
	fprintf (stderr, " '%s' : kind - %d, %c%c",
		 a->name, tc->kind, a->flags & (ORBit_I_ARG_IN | ORBit_I_ARG_INOUT) ? 'i' : ' ',
		 a->flags & (ORBit_I_ARG_OUT | ORBit_I_ARG_INOUT) ? 'o' : ' ');
}
	
static void
dump (FILE *out, guint8 const *ptr, guint32 len, guint32 offset)
{
	guint32 lp,lp2;
	guint32 off;

	for (lp = 0;lp<(len+15)/16;lp++) {
		fprintf (out, "0x%4x ", offset + lp * 16);
		for (lp2=0;lp2<16;lp2++) {
			off = lp2 + (lp<<4);
			off<len?fprintf (out, "%2x ", ptr[off]):fprintf (out, "XX ");
		}
		fprintf (out, "| ");
		for (lp2=0;lp2<16;lp2++) {
			off = lp2 + (lp<<4);
			fprintf (out, "%c", off<len?(ptr[off]>'!'&&ptr[off]<127?ptr[off]:'.'):'*');
		}
		if (lp == 0)
			fprintf (out, " --- \n");
		else
			fprintf (out, "\n");
	}
}

static void
giop_dump_send (GIOPSendBuffer *send_buffer)
{
	gulong nvecs;
	struct iovec *curvec;
	guint32 offset = 0;

	nvecs = send_buffer->num_used;
	curvec = (struct iovec *) send_buffer->iovecs;

	fprintf (stderr, "Outgoing IIOP data:\n");
	while (nvecs-- > 0) {
		dump (stderr, curvec->iov_base, curvec->iov_len, offset);
		offset += curvec->iov_len;
		curvec++;
	}
}

static void
giop_dump_recv (GIOPRecvBuffer *recv_buffer)
{
	g_assert (LINC_CONNECTION (recv_buffer->connection)->status == LINC_CONNECTED);

	fprintf (stderr, "Incoming IIOP data:\n");

	dump (stderr, recv_buffer->message_body,
	      recv_buffer->msg.header.message_size +
	      sizeof (GIOPMsgHeader), 0);
}
#endif /* DEBUG */

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

	dprintf ("Received exception '%s'\n", my_repoid ? my_repoid : "<Null>");

	reply_status = giop_recv_buffer_reply_status (rb);

	if (reply_status == CORBA_SYSTEM_EXCEPTION) {
		CORBA_unsigned_long minor;

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
		new->minor=minor;
		new->completed=completion_status;
			
		/* FIXME: check what should the repo ID be? */
		CORBA_exception_set (ev, CORBA_SYSTEM_EXCEPTION,
				     my_repoid, new);
	} else if (reply_status == CORBA_USER_EXCEPTION) {
		int i;

		for (i = 0; i < types->_length; i++) {
			if(!strcmp (types->_buffer[i]->repo_id, my_repoid))
				break;
		}

		if (!types) /* weirdness; they raised an exception that we don't
			       know about */
			CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
						   CORBA_COMPLETED_MAYBE);
		else {
			gpointer data = ORBit_demarshal_arg (
				rb, types->_buffer [i], TRUE, orb);
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     types->_buffer [i]->repo_id, data);
		}
	}
	return;
  
 errout:
	/* ignore LOCATION_FORWARD here, that gets handled in the stub */
	CORBA_exception_set_system (ev, ex_CORBA_MARSHAL,
				    CORBA_COMPLETED_MAYBE);
}

void
ORBit_small_send_user_exception (GIOPSendBuffer *send_buffer,
				 CORBA_Environment *ev,
				 const ORBit_ITypes *types)
{
	int i;

	for (i = 0; i < types->_length; i++) {
		if(!strcmp (types->_buffer[i]->repo_id, ev->_id))
			break;
	}

	if (i >= types->_length) {
		CORBA_Environment fakeev;

		g_warning ("Some clown returned undeclared "
			   "exception '%s' ", ev->_id);

		CORBA_exception_init (&fakeev);
		CORBA_exception_set_system (&fakeev, ex_CORBA_UNKNOWN,
					    CORBA_COMPLETED_MAYBE);
		ORBit_send_system_exception (send_buffer, &fakeev);
		CORBA_exception_free (&fakeev);
	} else {
		CORBA_unsigned_long len = strlen (ev->_id) + 1;

		giop_send_buffer_align (send_buffer, sizeof (len));
		giop_send_buffer_append_indirect (send_buffer, &len, sizeof (len));
		giop_send_buffer_append (send_buffer, ev->_id, len);

		dprintf ("Returning exception of type '%s'\n", ev->_id);

		ORBit_marshal_arg (send_buffer, ev->_any._value,
				   types->_buffer[i]);
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

/* CORBA_tk_, CORBA_ type, C-stack type, wire size in bits, wire size in bytes, print format */
/*
 * For future use in an strace type utility.
 *
#define _ORBIT_BASE_TYPE_MACRO							\
	_ORBIT_HANDLE_TYPE (short, short, int, 16, 2, "%d");			\
	_ORBIT_HANDLE_TYPE (long, long, int, 32, 4, "0x%x");			\
	_ORBIT_HANDLE_TYPE (enum, long, int, 32, 4, "%d");			\
	_ORBIT_HANDLE_TYPE (ushort, unsigned_short, unsigned int, 16, 2, "%u"); \
	_ORBIT_HANDLE_TYPE (ulong, unsigned_long, unsigned int, 32, 4, "0x%x");	\
	_ORBIT_HANDLE_TYPE (boolean, boolean, int, 8, 1, "%d");			\
	_ORBIT_HANDLE_TYPE (char, char, int, 8, 1, "%c");			\
	_ORBIT_HANDLE_TYPE (wchar, wchar, int, 16, 2, "%c");			\
	_ORBIT_HANDLE_TYPE (octet, octet, int, 8, 1, "0x%x");			\
	_ORBIT_HANDLE_TYPE (float, float, double, 32, 4, "%f");			\
	_ORBIT_HANDLE_TYPE (double, double, double, 64, 8, "%g");
*/

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

		dprintf ("\n");
	}

	ORBit_small_marshal_context (send_buffer, m_data, ctx);

	giop_dump_send (send_buffer);

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

	giop_dump_recv (recv_buffer);

	if (giop_recv_buffer_reply_status (recv_buffer) != GIOP_NO_EXCEPTION)
		goto msg_exception;

	dprintf ("Demarshal ");

	if ((tc = m_data->ret)) {
		dprintf ("ret: ");

		g_assert (ret != NULL);

		while (tc->kind == CORBA_tk_alias)
			tc = tc->subtypes [0];

		switch (tc->kind) {
		case BASE_TYPES:
		case OBJ_STRING_TYPES:
			do_demarshal_value (recv_buffer, &ret, tc, orb);
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
	ORBit_POAInvocation     invoke_rec G_GNUC_UNUSED;

	ORBIT_STUB_PreCall (obj, invoke_rec);

#ifdef DEBUG_LOCAL_TEST
	if (obj->pobj) {
#if 0
		ORBit_small_invoke_skel (
			ORBIT_STUB_GetServant (obj),
			m_data, ret, args, ctx, ev);
#endif

		ORBit_small_handle_request(obj->pobj, m_data->name, ret,
					   args, ctx, NULL, ev);
		goto clean_out;
	}
#endif

	cnx = ORBit_object_get_connection (obj);

	if (!cnx) {
		dprintf ("Null connection on object '%p'\n", obj);
		completion_status = CORBA_COMPLETED_NO;
		goto system_exception;
	}

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
	ORBIT_STUB_PostCall (obj, invoke_rec);
	return;

 system_exception:
	CORBA_exception_set_system (ev, ex_CORBA_COMM_FAILURE,
				    completion_status);
	goto clean_out;
}

#if 0
void
ORBit_small_invoke_skel (PortableServer_ServantBase *servant,
			 ORBit_IMethod              *m_data,
			 gpointer                    ret,
			 gpointer                   *args,
			 CORBA_Context               ctx,
			 CORBA_Environment          *ev)
{
	ORBitSmallSkeleton        small_skel;
	PortableServer_ClassInfo *klass;
	ORBit_IMethod            *real_mdata;
	gpointer                  imp;

	klass = ORBIT_SERVANT_TO_CLASSINFO (servant);

        small_skel = klass->small_relay_call(
		servant, m_data->name, (gpointer *)&real_mdata, &imp);

	if (real_mdata != m_data)
		g_warning ("Wierd, new type data");

	if (!imp) /* FIXME: is_a ? */
		CORBA_exception_set_system (ev, ex_CORBA_NO_IMPLEMENT,
					    CORBA_COMPLETED_NO);
	else
		small_skel (servant, ret, args, ctx, ev, imp);
}
#endif

void
ORBit_small_invoke_poa (PortableServer_ServantBase *servant,
			GIOPRecvBuffer             *recv_buffer,
			ORBit_IMethod              *m_data,
			ORBitSmallSkeleton          small_skel,
			gpointer                    impl,
			CORBA_Environment          *ev)
{
	int             i;
	gpointer       *args = NULL;
	gpointer       *scratch = NULL;
	gpointer        pretval = NULL;
	gpointer        retval = NULL;
	GIOPSendBuffer *send_buffer;
	CORBA_ORB       orb;
	CORBA_TypeCode  tc;
	gboolean        has_context;
	struct CORBA_Context_type ctx;

/*	fprintf (stderr, "Method '%s' on '%p'\n", m_data->name, servant); */

	giop_dump_recv (recv_buffer);

	orb = ORBIT_SERVANT_TO_ORB (servant);

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

	small_skel (servant, retval, args, &ctx, ev, impl);

	if (has_context)
		ORBit_Context_server_free (&ctx);

	send_buffer = giop_send_buffer_use_reply (
		recv_buffer->connection->giop_version,
		giop_recv_buffer_get_request_id (recv_buffer),
		ev->_major);

	if (!send_buffer) {
		dprintf ("Weird, no send_buffer");
		return;

	} else if (ev->_major == CORBA_USER_EXCEPTION) {
		ORBit_small_send_user_exception (
			send_buffer, ev, &m_data->exceptions);

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

	giop_dump_send (send_buffer);

	giop_send_buffer_write (send_buffer, recv_buffer->connection);
	giop_send_buffer_unuse (send_buffer);

	if (m_data->ret) {
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
	gpointer epv;
	ORBit_POAObject pobj;
	PortableServer_ServantBase *servant;
	PortableServer_ClassInfo   *class_info;
	CORBA_unsigned_long         offset;
	
	pobj       = obj->pobj;
	servant    = pobj->servant;
	class_info = servant->vepv[0]->_private;
	g_assert (class_info != NULL);
	g_assert (class_id < class_info->vepvlen);
	offset     = class_info->vepvmap [class_id];

	epv = servant->vepv [offset];
		
	return epv;
}
#endif
