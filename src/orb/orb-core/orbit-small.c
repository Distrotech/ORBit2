/*
 * Warning - before reading this file, and while hacking
 * it, it is advisable to examine:
 *
 *    docs/internals/interface-indirection.gnumeric
 *
 * FIXME: We need some global I/F -> m_data lookup action
 * FIXME: We need to map interface inheritance.
 * FIXME: Add #ifdef ORBIT_PURIFY support.
 * FIXME: remove the redundant marshal_fn
 *
 * FIXME: 'Obvious' optimizations
 *  Here:
 *    * 2 demarshalers - 1 straight, 1 endianness switching.
 *    * do more alloca's for basic things
 *    * more IDL compiler help for allocation and indirection
 *    decisions - these are closely tied
 *  Elsewhere:
 *    * store object profiles in GIOP format for fast marshaling
 *    * make locking more chunky _T everything.
 */

#include "config.h"
#include <orbit/orbit.h>
#include <orbit/GIOP/giop.h>
#include <stdio.h>
#include <string.h>

#undef DEBUG
#undef DEBUG_LOCAL_TEST

gpointer
ORBit_small_alloc (CORBA_TypeCode tc)
{
	gpointer mem;

	mem = ORBit_alloc_tcval (tc, 1);

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
dump (FILE *out, guint8 const *ptr, guint32 len)
{
	guint32 lp,lp2;
	guint32 off;

	for (lp = 0;lp<(len+15)/16;lp++)
	{
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

	nvecs = send_buffer->num_used;
	curvec = (struct iovec *) send_buffer->iovecs;

	fprintf (stderr, "Outgoing IIOP data:\n");
	while (nvecs-- > 0) {
		dump (stderr, curvec->iov_base, curvec->iov_len);
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
	      sizeof (GIOPMsgHeader));
}
#endif /* DEBUG */

/**** ORBit_handle_exception_array: based on corba-env.c (ORBit_handle_exception);
      we don't want our own de-marshallers, use a generic mechanism instead.

      Inputs: 'rb' - a receive buffer for which an exception condition has
                     been determined
	      'ev' - memory in which to store the exception information

	      'types' -     list of user exception typecodes
	                    for this particular operation.
      Side-effects: reinitializes '*ev'

      Description:
           During demarshalling a reply, if reply_status != CORBA_NO_EXCEPTION,
	   we must find out what exception was raised and place that information
	   in '*ev'.
*/
static void
ORBit_handle_exception_array (GIOPRecvBuffer *rb, CORBA_Environment *ev,
			      const ORBit_ITypes *types,
			      CORBA_ORB orb)
{
  CORBA_SystemException *new;
  CORBA_unsigned_long len, completion_status, reply_status;
  CORBA_char *my_repoid;

  g_return_if_fail (rb->msg.header.message_type == GIOP_REPLY);

  CORBA_exception_free(ev);

  rb->cur = ALIGN_ADDRESS(rb->cur, sizeof(len));
  if((rb->cur + 4) > rb->end)
    goto errout;
  len = *(CORBA_unsigned_long *)rb->cur;
  rb->cur += 4;
  if(giop_msg_conversion_needed(rb))
    len = GUINT32_SWAP_LE_BE(len);

  if(len)
    {
      my_repoid = rb->cur;
      rb->cur += len;
    }
  else
    my_repoid = NULL;

  dprintf ("Received exception '%s'\n", my_repoid ? my_repoid : "<Null>");

  reply_status = giop_recv_buffer_reply_status(rb);
  if(reply_status == CORBA_SYSTEM_EXCEPTION)
    {
      CORBA_unsigned_long minor;

      ev->_major = CORBA_SYSTEM_EXCEPTION;

      rb->cur = ALIGN_ADDRESS(rb->cur, sizeof(minor));
      if((rb->cur + sizeof(minor)) > rb->end)
	goto errout;
      minor = *(CORBA_unsigned_long*)rb->cur;
      rb->cur += 4;
      if(giop_msg_conversion_needed(rb))
	minor = GUINT32_SWAP_LE_BE(minor);

      rb->cur = ALIGN_ADDRESS(rb->cur, sizeof(completion_status));
      if((rb->cur + sizeof(completion_status)) > rb->end)
	goto errout;
      completion_status = *(CORBA_unsigned_long*)rb->cur;
      rb->cur += 4;
      if(giop_msg_conversion_needed(rb))
	completion_status = GUINT32_SWAP_LE_BE(completion_status);

      new = CORBA_SystemException__alloc();
      new->minor=minor;
      new->completed=completion_status;
			
      /* XXX what should the repo ID be? */
      CORBA_exception_set(ev, CORBA_SYSTEM_EXCEPTION,
			  my_repoid, new);
    }
  else if(reply_status == CORBA_USER_EXCEPTION)
    {
      int i;

      for(i = 0; i < types->_length; i++) {
        if(!strcmp(types->_buffer[i]->repo_id,my_repoid))
          break;
      }

      if (!types)
        {
          /* weirdness; they raised an exception that we don't
	     know about */
	  CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
				     CORBA_COMPLETED_MAYBE);
	}
      else
        {
          gpointer data =
		  ORBit_demarshal_arg (rb, types->_buffer [i], TRUE, orb);
	  CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			       types->_buffer [i]->repo_id, data);
	}
    }

  return;
  
  /* ignore LOCATION_FORWARD here, that gets handled in the stub */
 errout:
  CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
			     CORBA_COMPLETED_MAYBE);
}

void
ORBit_small_send_user_exception(GIOPSendBuffer *send_buffer,
				CORBA_Environment *ev,
				const ORBit_ITypes *types)
{
  int i;

  for (i = 0; i < types->_length; i++)
    {
      if(!strcmp(types->_buffer[i]->repo_id, ev->_id))
	break;
    }

  if (i >= types->_length)
    {
      CORBA_Environment fakeev;
      CORBA_exception_init(&fakeev);
      CORBA_exception_set_system(&fakeev, ex_CORBA_UNKNOWN,
				 CORBA_COMPLETED_MAYBE);
      ORBit_send_system_exception(send_buffer, &fakeev);
      CORBA_exception_free(&fakeev);
      g_warning ("Some clown returned undeclared exception '%s' ", ev->_id);
    }
  else
    {
      CORBA_unsigned_long len = strlen(ev->_id) + 1;

      giop_send_buffer_align(send_buffer, sizeof(len));
      giop_send_buffer_append_indirect(send_buffer, &len, sizeof(len));
      giop_send_buffer_append(send_buffer, ev->_id, len);

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

#define CORBA_BASE_TYPES \
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

typedef struct {
	CORBA_unsigned_long len;
	char                opname[1];
} OpData;

#define do_marshal_value(a,b,c,d)     \
	ORBit_marshal_value   ((a),(gconstpointer *)(b),(c),(d))
#define do_demarshal_value(a,b,c,e) \
	ORBit_demarshal_value ((c),(b),(a),TRUE,(e))

static gboolean
_ORBit_generic_marshal (CORBA_Object           obj,
			GIOPConnection        *cnx,
			GIOPMessageQueueEntry *mqe,
			CORBA_unsigned_long    request_id,
			ORBit_IMethod         *m_data,
			gpointer              *args,
			CORBA_Context          ctx)
{
	GIOPSendBuffer          *send_buffer;
	struct iovec             op_vec;
	guchar                  *header;
	CORBA_TypeCode           tc;
	int                      i;
	ORBit_marshal_value_info mi;

	{
		int len = sizeof (CORBA_unsigned_long) + m_data->name_len + 1;
		int align;
		header = alloca (len + sizeof (CORBA_unsigned_long));
		*(CORBA_unsigned_long *)header = m_data->name_len + 1;
		memcpy (header + sizeof (CORBA_unsigned_long),
			m_data->name, m_data->name_len + 1);
		op_vec.iov_base = header;
	       
		align = len + (sizeof (CORBA_unsigned_long) - 1);
		align &= ~(sizeof (CORBA_unsigned_long) - 1);
		memset (header + len, 0, align - len);

		dprintf ("Align = %d\n", align);
		op_vec.iov_len  = align;
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
	_ORBIT_MARSHAL_SYS_EXCEPTION_INCOMPLETE,
	_ORBIT_MARSHAL_SYS_EXCEPTION_COMPLETE,
	_ORBIT_MARSHAL_EXCEPTION_COMPLETE,
	_ORBIT_MARSHAL_RETRY,
	_ORBIT_MARSHAL_CLEAN
} DeMarshalRetType;

static DeMarshalRetType
_ORBit_generic_demarshal (CORBA_Object           obj,
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
		return _ORBIT_MARSHAL_SYS_EXCEPTION_INCOMPLETE;
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
		case CORBA_BASE_TYPES:
		case CORBA_tk_objref:
		case CORBA_tk_TypeCode:
		case CORBA_tk_string:
		case CORBA_tk_wstring:
			do_demarshal_value (recv_buffer, &ret, tc, orb);
			dprintf ("base type / [p]obj / [w]string");
			break;

		case CORBA_tk_struct:
		case CORBA_tk_union:
		case CORBA_tk_except:
			if (m_data->flags & ORBit_I_COMMON_FIXED_SIZE) {
				do_demarshal_value (recv_buffer, &ret, tc, orb);
				dprintf ("misc. fixed");
				break;
			} /* drop through */

		case CORBA_tk_any:
		case CORBA_tk_sequence:
		case CORBA_tk_array:
		default:
			data = ORBit_demarshal_arg (recv_buffer, tc, TRUE, orb);
			if (!data)
				return _ORBIT_MARSHAL_SYS_EXCEPTION_COMPLETE;
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
			case CORBA_BASE_TYPES:
			case CORBA_tk_objref:
			case CORBA_tk_TypeCode:
			case CORBA_tk_string:
			case CORBA_tk_wstring: {
				if (a->flags & ORBit_I_ARG_INOUT) {
					if (tc->kind == CORBA_tk_TypeCode ||
					    tc->kind == CORBA_tk_objref)
						CORBA_Object_release (*(gpointer *)arg, ev);
					else if (tc->kind == CORBA_tk_string ||
						 tc->kind == CORBA_tk_wstring)
						CORBA_free (*(gpointer *) arg);
				}

				do_demarshal_value (
					recv_buffer, &arg, tc, obj);
				dprintf ("base / allocated type");
				break;
			}

			case CORBA_tk_union:
			case CORBA_tk_struct:
			case CORBA_tk_except:
			case CORBA_tk_sequence:
			case CORBA_tk_array:
			case CORBA_tk_any: {
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
 
			default:
				g_warning ("Unknown type %d",
					   tc->kind);
				break;
			}
			dprintf ("\n");
		}
	}
	
	giop_recv_buffer_unuse (recv_buffer);
	return _ORBIT_MARSHAL_CLEAN;

 msg_exception:
	if (giop_recv_buffer_reply_status (recv_buffer) ==
	    GIOP_LOCATION_FORWARD) {
		
		*cnx = ORBit_handle_location_forward (recv_buffer, obj);

		return _ORBIT_MARSHAL_RETRY;
	} else {
		ORBit_handle_exception_array (
			recv_buffer, ev, &m_data->exceptions, obj->orb);
		giop_recv_buffer_unuse (recv_buffer);

		return _ORBIT_MARSHAL_EXCEPTION_COMPLETE;
	}
}


void
ORBit_small_invoke_stub (CORBA_Object       obj,
			 ORBit_IMethod     *m_data,
			 gpointer           marshal_fn,
			 gpointer           ret,
			 gpointer          *args,
			 CORBA_Context      ctx,
			 CORBA_Environment *ev)
{
	CORBA_unsigned_long     request_id;
	CORBA_completion_status completion_status;
	GIOPConnection         *cnx;
	GIOPMessageQueueEntry   mqe;

#ifdef DEBUG_LOCAL_TEST
	if (obj->bypass_obj) {
		ORBit_small_invoke_skel (
			ORBIT_STUB_GetServant (obj),
			m_data, marshal_fn,
			ret, args, ctx, ev);
		return;
	}
#endif

	g_return_if_fail (marshal_fn == NULL);

	cnx = ORBit_object_get_connection (obj);

	if (!cnx) {
		dprintf ("Null connection on object '%p'\n", obj);
		completion_status = CORBA_COMPLETED_NO;
		goto system_exception;
	}

retry_request:
	request_id = GPOINTER_TO_UINT (alloca (0));
	completion_status = CORBA_COMPLETED_NO;

	if (!_ORBit_generic_marshal (obj, cnx, &mqe, request_id,
				     m_data, args, ctx))
		goto system_exception;

	completion_status = CORBA_COMPLETED_MAYBE;

	if (m_data->flags & ORBit_I_METHOD_1_WAY)
		return;

	switch (_ORBit_generic_demarshal (obj, &cnx, &mqe, ev,
					  ret, m_data, args))
	{
	case _ORBIT_MARSHAL_SYS_EXCEPTION_COMPLETE:
		completion_status = CORBA_COMPLETED_YES;
		dprintf ("Sys exception completed on id 0x%x\n\n", request_id);
		goto system_exception;

	case _ORBIT_MARSHAL_SYS_EXCEPTION_INCOMPLETE:
		dprintf ("Sys exception incomplete on id 0x%x\n\n", request_id);
		goto system_exception;

	case _ORBIT_MARSHAL_EXCEPTION_COMPLETE:
		dprintf ("Clean demarshal of exception on id 0x%x\n\n", request_id);
		break;

	case _ORBIT_MARSHAL_RETRY:
		dprintf ("Retry demarshal on id 0x%x\n\n", request_id);
		goto retry_request;

	case _ORBIT_MARSHAL_CLEAN:
		dprintf ("Clean demarshal on id 0x%x\n\n", request_id);
		break;
	};
	return;

 system_exception:
	CORBA_exception_set_system (ev, ex_CORBA_COMM_FAILURE,
				    completion_status);
	return;
}

void
ORBit_small_invoke_skel (PortableServer_ServantBase *servant,
			 ORBit_IMethod              *m_data,
			 gpointer                    marshal_fn,
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

	dprintf ("Method '%s' on '%p'\n", m_data->name, servant);

	giop_dump_recv (recv_buffer);

	orb = ORBIT_SERVANT_TO_ORB (servant);

	has_context = (m_data->contexts._length > 0);

	if ((tc = m_data->ret)) {
		
		while (tc->kind == CORBA_tk_alias)
			tc = tc->subtypes [0];

		switch (tc->kind) {
		case CORBA_BASE_TYPES:
		case CORBA_tk_objref:
		case CORBA_tk_TypeCode:
		case CORBA_tk_string:
		case CORBA_tk_wstring:
			/* FIXME: could be alloca */
			retval = ORBit_alloc_tcval (tc, 1);
			break;
		case CORBA_tk_struct:
		case CORBA_tk_union:
		case CORBA_tk_except:
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
			case CORBA_BASE_TYPES:
			case CORBA_tk_objref:
			case CORBA_tk_TypeCode:
			case CORBA_tk_string:
			case CORBA_tk_wstring:
				p = args [i] = alloca (ORBit_gather_alloc_info (tc));
				do_demarshal_value (recv_buffer, &p, tc, orb);
				break;
			case CORBA_tk_struct:
			case CORBA_tk_union:
			case CORBA_tk_except:
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
			case CORBA_BASE_TYPES:
			case CORBA_tk_objref:
			case CORBA_tk_TypeCode:
			case CORBA_tk_string:
			case CORBA_tk_wstring:
				/* FIXME: could be alloca */
				scratch [i] = ORBit_alloc_tcval (tc, 1);
				break;
			case CORBA_tk_struct:
			case CORBA_tk_union:
			case CORBA_tk_except:
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
				
			case CORBA_BASE_TYPES:
			case CORBA_tk_objref:
			case CORBA_tk_TypeCode:
			case CORBA_tk_string:
			case CORBA_tk_wstring:
				ORBit_marshal_arg (send_buffer, retval, m_data->ret);
				dprintf ("base/[p]obj/string");
				break;

			case CORBA_tk_struct:
			case CORBA_tk_union:
			case CORBA_tk_except:
				if (m_data->flags & ORBit_I_COMMON_FIXED_SIZE) {
					ORBit_marshal_arg (send_buffer, retval, m_data->ret);
					dprintf ("fixed");
					break;
				} /* drop through */

			case CORBA_tk_any:
			case CORBA_tk_sequence:
			case CORBA_tk_array:
				ORBit_marshal_arg (send_buffer, *(gpointer *)retval, m_data->ret);
				dprintf ("pointer baa");
				break;
			default:
				g_assert_not_reached ();
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
		case CORBA_BASE_TYPES:
		case CORBA_tk_objref:
		case CORBA_tk_TypeCode:
		case CORBA_tk_string:
		case CORBA_tk_wstring:
			CORBA_free (retval);
			break;
		case CORBA_tk_struct:
		case CORBA_tk_union:
		case CORBA_tk_except:
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
		
		if (a->flags & ORBit_I_ARG_IN ||
		    a->flags & ORBit_I_ARG_INOUT) {
			
			tc = a->tc;
			
			while (tc->kind == CORBA_tk_alias)
				tc = tc->subtypes [0];
			
			switch (tc->kind) {
			case CORBA_BASE_TYPES:
				break;
			case CORBA_tk_objref:
			case CORBA_tk_TypeCode:
				CORBA_Object_release (*(CORBA_Object *) args [i], ev);
				break;
			case CORBA_tk_string:
			case CORBA_tk_wstring:
				CORBA_free (*(char **) args [i]);
				break;
			case CORBA_tk_struct:
			case CORBA_tk_union:
			case CORBA_tk_except:
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
		} else /* Out */
			if (ev->_major == CORBA_NO_EXCEPTION)
				CORBA_free (scratch [i]);
	}
	
	CORBA_exception_free (ev);
}

#ifdef DEBUG
gpointer
ORBit_small_getepv (CORBA_Object obj, CORBA_unsigned_long class_id)
{
	gpointer epv;
	ORBit_POAObject *pobj;
	PortableServer_ServantBase *servant;
	PortableServer_ClassInfo   *class_info;
	CORBA_unsigned_long         offset;
	
	pobj       = obj->bypass_obj;
	servant    = pobj->servant;
	class_info = servant->vepv[0]->_private;
	g_assert (class_info != NULL);
	g_assert (class_id < class_info->vepvlen);
	offset     = class_info->vepvmap [class_id];

	epv = servant->vepv [offset];
		
	return epv;
}
#endif
