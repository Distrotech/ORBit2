/*
 * An attempt to shrink the beast to a managable size.
 */

/*
 * FIXME: would a function table be a good solution -
 * and also manage the complexity - and also allow us
 * to marshal direct to the wire ?
 * Could we manage offsets and deal with the wierd
 * indirection ?
 */

#include "config.h"
#include <orbit/orbit.h>
#include <orbit/GIOP/giop.h>
#include <stdio.h>

#define DEBUG

gpointer
ORBit_small_alloc (CORBA_TypeCode tc)
{
	/* Wasted stack frame overhead, not to loose semantic
	   value of an _alloc, and hence retaining maintainability
	   NB. speed is not a concern whatsoever */
	return ORBit_alloc_tcval (tc, 1);
}

gpointer
ORBit_small_allocbuf (CORBA_TypeCode tc, CORBA_unsigned_long length)
{
	/* see above */
	return ORBit_alloc_tcval (tc, length);
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

/* CORBA_tk_, CORBA_ type, C-stack type, wire size in bits, wire size in bytes, print format */
#define _ORBIT_BASE_TYPES							\
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


typedef struct {
	CORBA_unsigned_long len;
	char                opname[1];
} OpData;

#define do_marshal_value(a,b,c,d)     \
	ORBit_marshal_value   ((a),(gconstpointer *)(b),(c),(d))
#define do_demarshal_value(a,b,c,d,e) \
	ORBit_demarshal_value ((c),(b),(a),(d),(e))

static gboolean
_ORBit_generic_marshal (CORBA_Object           obj,
			GIOPConnection        *cnx,
			GIOPMessageQueueEntry *mqe,
			CORBA_unsigned_long    request_id,
			ORBit_IMethod         *m_data,
			gpointer              *args)
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
 alias_on_arg:
		dump_arg (a, tc);

		if (tc->kind == CORBA_tk_alias) {
			tc = tc->subtypes [0];
			goto alias_on_arg;
		}

		/* FIXME: This looks too simple ! complicate it. */
		if (a->flags & ORBit_I_ARG_INOUT)
			p = *(gpointer *)args [i];
		else if (a->flags & ORBit_I_ARG_IN)
			p = args [i];
		
		do_marshal_value (send_buffer, &p, tc, &mi);

		dprintf ("\n");
	}

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

 alias_on_return:
		switch (tc->kind) {

		case CORBA_tk_alias:
			tc = tc->subtypes[0];
			goto alias_on_return;

#define _ORBIT_HANDLE_TYPE(tk,ct,st,bt,by,fmt) \
			case CORBA_tk_##tk:
			_ORBIT_BASE_TYPES
				do_demarshal_value (recv_buffer, &ret, tc, TRUE, obj->orb);
				dprintf ("base type");
				break;
#undef _ORBIT_HANDLE_TYPE				       

		case CORBA_tk_objref:
		case CORBA_tk_TypeCode:
		case CORBA_tk_string:
		case CORBA_tk_wstring:
			data = ORBit_demarshal_arg (recv_buffer, tc, TRUE, obj->orb);
			if (!data)
				return _ORBIT_MARSHAL_SYS_EXCEPTION_COMPLETE;
			*((gpointer *)ret) = *((gpointer *) data);
			dprintf ("obj/string");
			break;

		case CORBA_tk_array:
			data = ORBit_demarshal_arg (recv_buffer, tc, TRUE, obj->orb);
			if (!data)
				return _ORBIT_MARSHAL_SYS_EXCEPTION_COMPLETE;
			*((gpointer *)ret) = data;
			dprintf ("array");
			break;

		case CORBA_tk_any:
		case CORBA_tk_struct:
		case CORBA_tk_union:
		case CORBA_tk_sequence:
		case CORBA_tk_except:
			if (m_data->flags & ORBit_I_METHOD_RET_FIXED_SIZE) {
				do_demarshal_value (recv_buffer, &ret, tc, TRUE, obj->orb);
				dprintf ("fixed");
			} else {
				data = ORBit_demarshal_arg (recv_buffer, tc, TRUE, obj->orb);
				if (!data)
					return _ORBIT_MARSHAL_SYS_EXCEPTION_COMPLETE;
				*((gpointer *)ret) = data;
				dprintf ("pointer");
			}
			break;
		default:
			g_assert_not_reached ();
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

			a = &m_data->arguments._buffer [i];

			if (!(a->flags & (ORBit_I_ARG_OUT |
					  ORBit_I_ARG_INOUT)))
				continue;

			tc = a->tc;

 alias_on_arg:
			dump_arg (a, tc);

			switch (tc->kind) {
			case CORBA_tk_alias:
				tc = tc->subtypes[0];
				goto alias_on_arg;
				
				/* FIXME: we need to release the original
				   reference */
#define _ORBIT_HANDLE_TYPE(tk,ct,st,bt,by,fmt) \
			case CORBA_tk_##tk:
			_ORBIT_BASE_TYPES {
				gpointer arg = *(gpointer *)args [i];
				do_demarshal_value (
					recv_buffer, &arg, tc, TRUE, obj->orb);
				dprintf ("base type");
				break;
			}
#undef _ORBIT_HANDLE_TYPE				       

			case CORBA_tk_objref:
			case CORBA_tk_TypeCode: {
				CORBA_TypeCode *arg = args [i];

				if (a->flags & ORBit_I_ARG_INOUT)
					CORBA_Object_release ((CORBA_Object) *arg, ev);

				do_demarshal_value (
					recv_buffer, (gpointer) &arg, tc, TRUE, obj->orb);

				dprintf ("typecode");
				break;
			}

			case CORBA_tk_string:
			case CORBA_tk_wstring: {
				gpointer *arg = *(gpointer *)args [i];
				data = ORBit_demarshal_arg (recv_buffer,
							    tc, TRUE, obj->orb);
				if (!data || !arg)
					return _ORBIT_MARSHAL_SYS_EXCEPTION_COMPLETE;
				
				if (a->flags & ORBit_I_ARG_INOUT &&
				    !(a->flags & ORBit_I_ARG_FIXED_SIZE))
					CORBA_free (*arg);
				
				*arg = *((gpointer *) data);
				dprintf (" '%s' into %p", (char *) *arg, arg);
				break;
			}

			case CORBA_tk_union:
			case CORBA_tk_except:
			case CORBA_tk_sequence:
			case CORBA_tk_struct:
			case CORBA_tk_array:
			case CORBA_tk_any: {
				gpointer p;

				if (a->flags & ORBit_I_ARG_FIXED_SIZE) {
					p = *(gpointer *)args [i];
					do_demarshal_value (recv_buffer, &p, tc, TRUE, obj->orb);
				} else if (a->flags & ORBit_I_ARG_INOUT) {
					/* FIXME: ghastly - we need to free any child
					 * elements of this non dynamicaly allocated
					 * structure: use __freekids ?*/
/*					CORBA_free (*(gpointer *)args [i]);*/
					p = *(gpointer *)args [i];
					do_demarshal_value (recv_buffer, &p, tc, TRUE, obj->orb);
				} else /* Out */
					*(gpointer *)args [i] = ORBit_demarshal_arg (
						recv_buffer, tc, TRUE, obj->orb);
				break;
			}
 
			case CORBA_tk_longlong:
			case CORBA_tk_ulonglong:
			case CORBA_tk_longdouble:
			case CORBA_tk_null:
			case CORBA_tk_void:
				g_warning ("Non-sensible type %d",
					   tc->kind);
				break;
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

		return _ORBIT_MARSHAL_SYS_EXCEPTION_COMPLETE;
	}
}


void
ORBit_small_invoke_stub (CORBA_Object       obj,
			 ORBit_IMethod     *m_data,
			 gpointer           marshal_fn,
			 gpointer           ret,
			 gpointer          *args,
			 CORBA_Environment *ev)
{
	CORBA_unsigned_long     request_id;
	CORBA_completion_status completion_status;
	GIOPConnection         *cnx;
	GIOPMessageQueueEntry   mqe;

	g_return_if_fail (marshal_fn == NULL);

	cnx = ORBit_object_get_connection (obj);

	if (!cnx) {
		dprintf ("Null connection on object '%p'", obj);
		completion_status = CORBA_COMPLETED_NO;
		goto system_exception;
	}

retry_request:
	request_id = GPOINTER_TO_UINT (alloca (0));
	completion_status = CORBA_COMPLETED_NO;

	if (!_ORBit_generic_marshal (obj, cnx, &mqe, request_id,
				     m_data, args))
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
			 CORBA_Environment          *ev)
{
	g_warning ("Stubbed - fill in for scripting");
}

void
ORBit_small_invoke_poa (PortableServer_ServantBase *servant,
			GIOPRecvBuffer             *recv_buffer,
			ORBit_IMethod              *m_data,
			ORBitSmallSkeleton          small_skel,
			gpointer                    impl,
			CORBA_Environment          *ev)
{
	int             i, size;
	gpointer       *args = NULL;
	gpointer       *scratch = NULL;
	gpointer        retval = NULL;
	GIOPSendBuffer *send_buffer;
	CORBA_ORB       orb;

	dprintf ("Method '%s' on '%p'\n", m_data->name, servant);

	orb = ORBIT_SERVANT_TO_ORB (servant);

	if (m_data->ret)
		retval = alloca (ORBit_gather_alloc_info (
			m_data->ret));

	if (m_data->arguments._length > 0) {
		args = alloca (m_data->arguments._length * sizeof (gpointer));
		scratch = alloca (m_data->arguments._length * sizeof (gpointer));
	}		

	for (i = 0; i < m_data->arguments._length; i++) {
		ORBit_IArg *a = &m_data->arguments._buffer [i];
		
		if (a->flags & ORBit_I_ARG_IN)
			args [i] = ORBit_demarshal_arg (
				recv_buffer, a->tc, TRUE, orb);
		
		else if (a->flags & ORBit_I_ARG_INOUT) {
			args [i] = &scratch [i];
			scratch [i] = ORBit_demarshal_arg (
				recv_buffer, a->tc, TRUE, orb);
			
		} else { /* OUT */
			args [i] = &scratch [i];
			scratch [i] = NULL;
		}
	}

	small_skel (servant, retval, args, ev, impl);

	send_buffer = giop_send_buffer_use_reply (
		recv_buffer->connection->giop_version,
		giop_recv_buffer_get_request_id (recv_buffer),
		ev->_major);

	if (!send_buffer) {
		dprintf ("Weird, no send_buffer");
		return;
	} else if (ev->_major != CORBA_NO_EXCEPTION)
		ORBit_send_system_exception (send_buffer, ev);
	
	else { /* Marshal return values */

		if (m_data->ret)
			ORBit_marshal_arg (send_buffer, retval, m_data->ret);

		for (i = 0; i < m_data->arguments._length; i++) {
			ORBit_IArg *a = &m_data->arguments._buffer [i];
			
			if (a->flags & ORBit_I_ARG_IN)
				CORBA_free (args [i]);
			
			else if (a->flags & ORBit_I_ARG_INOUT) {
				ORBit_marshal_arg (send_buffer, scratch [i], a->tc);
/* FIXME: deal with allocation fun */
/*				CORBA_free (scratch [i]); */
			} else { /* OUT */
				ORBit_marshal_arg (send_buffer, scratch [i], a->tc);
/*				CORBA_free (scratch [i]); */
			}
		}
	}

	giop_send_buffer_write (send_buffer, recv_buffer->connection);
	giop_send_buffer_unuse (send_buffer);
}

