#include <config.h>
#include <string.h>
#include <unistd.h>

#include <orbit/orbit.h>
#include <orbit/orb-core/orbit-adaptor.h>

#include "orbit-debug.h"
#include "../GIOP/giop-debug.h"

CORBA_long
ORBit_adaptor_setup (ORBit_ObjectAdaptor adaptor,
		     ORBit_Adaptor_type  type,
		     CORBA_ORB           orb)
{
	CORBA_long *tptr;

	adaptor->adaptor_type = type;
	adaptor->adaptor_id   = orb->adaptors->len;
	adaptor->orb          = ORBit_RootObject_duplicate (orb);

	g_ptr_array_set_size (orb->adaptors, adaptor->adaptor_id + 1);
	g_ptr_array_index    (orb->adaptors, adaptor->adaptor_id) = adaptor;

	adaptor->adaptor_key._length  = ORBIT_ADAPTOR_PREFIX_LEN;
	adaptor->adaptor_key._buffer  =
		CORBA_sequence_CORBA_octet_allocbuf (adaptor->adaptor_key._length);
	adaptor->adaptor_key._release = CORBA_TRUE;

	ORBit_genuid_buffer (adaptor->adaptor_key._buffer + sizeof (CORBA_long),
			     ORBIT_ADAPTOR_KEY_LEN, ORBIT_GENUID_COOKIE);

	tptr = (CORBA_long *) adaptor->adaptor_key._buffer;
	*tptr = adaptor->adaptor_id;

	return adaptor->adaptor_id;
}

void
ORBit_adaptor_free (ORBit_ObjectAdaptor adaptor)
{
	if (adaptor->adaptor_key._buffer)
		ORBit_free_T (adaptor->adaptor_key._buffer);

	g_ptr_array_index (adaptor->orb->adaptors, adaptor->adaptor_id) = NULL;
	adaptor->adaptor_id = -1;

        ORBit_RootObject_release_T (adaptor->orb);
}

static ORBit_ObjectAdaptor
ORBit_adaptor_find (CORBA_ORB orb, ORBit_ObjectKey *objkey)
{
	gint32 adaptorId;
	ORBit_ObjectAdaptor adaptor;

	if (!objkey)
		return NULL;

	if (objkey->_length < ORBIT_ADAPTOR_PREFIX_LEN)
		return NULL;

	memcpy (&adaptorId, objkey->_buffer, sizeof (gint32));

	if (adaptorId < 0 || adaptorId >= orb->adaptors->len)
		return NULL;

	adaptor = g_ptr_array_index (orb->adaptors, adaptorId);

	if (memcmp (objkey->_buffer,
		    adaptor->adaptor_key._buffer,
	            ORBIT_ADAPTOR_PREFIX_LEN))
		return NULL;

	return adaptor;
}

void
ORBit_handle_request (CORBA_ORB orb, GIOPRecvBuffer *recv_buffer)
{
	ORBit_ObjectKey *objkey;
	ORBit_ObjectAdaptor adaptor;

	objkey = giop_recv_buffer_get_objkey (recv_buffer);
	adaptor = ORBit_adaptor_find (orb, objkey);

	if (!adaptor || !objkey) {
		CORBA_Environment ev;

		tprintf ("Error: failed to find adaptor or objkey for "
			 "object while invoking method '%s'",
			 giop_recv_buffer_get_opname (recv_buffer));
		
		CORBA_exception_set_system (
			&ev, ex_CORBA_OBJECT_NOT_EXIST, 
			CORBA_COMPLETED_NO);
		ORBit_recv_buffer_return_sys_exception (recv_buffer, &ev);

	} else {
		dprintf (MESSAGES, "p %d: handle request '%s'\n",
			 getpid (),
			 giop_recv_buffer_get_opname (recv_buffer));

		adaptor->handle_request (adaptor, recv_buffer, objkey);
	}
}

void
ORBit_small_handle_request (ORBit_OAObject     adaptor_obj,
			    CORBA_Identifier   opname,
			    gpointer           ret,
			    gpointer          *args,
			    CORBA_Context      ctx,
			    GIOPRecvBuffer    *recv_buffer,
	 		    CORBA_Environment *ev)
{
	adaptor_obj->interface->handle_request (adaptor_obj, opname, ret, 
					      args, ctx, recv_buffer, ev);
}

gboolean
ORBit_OAObject_is_active (ORBit_OAObject adaptor_obj) 
{
	return adaptor_obj->interface->is_active (adaptor_obj);

}

ORBit_ObjectKey*
ORBit_OAObject_object_to_objkey (ORBit_OAObject adaptor_obj)
{
	return adaptor_obj->interface->object_to_objkey (adaptor_obj);
}

void
ORBit_OAObject_invoke (ORBit_OAObject     adaptor_obj,
		       gpointer           ret,
		       gpointer          *args,
		       CORBA_Context      ctx,
		       gpointer           data,
		       CORBA_Environment *ev)
{
	adaptor_obj->interface->invoke(adaptor_obj, ret, args, ctx, data, ev);
}

/*
 * giop_recv_buffer_return_sys_exception:
 * @recv_buffer:
 * @m_data:
 * @ev:
 *
 * Return a system exception in @ev to the client. If @m_data
 * is not nil, it used to determine whether the call is a
 * oneway and, hence, whether to return the exception. If
 * @m_data is nil, we are not far enough along in the processing
 * of the reqeust to be able to determine if this is a oneway
 * method.
 */
void
ORBit_recv_buffer_return_sys_exception (GIOPRecvBuffer    *recv_buffer,
					CORBA_Environment *ev)
{
	GIOPSendBuffer *send_buffer;

	if (!recv_buffer) /* In Proc */
		return;

	g_return_if_fail (ev->_major == CORBA_SYSTEM_EXCEPTION);

	send_buffer = giop_send_buffer_use_reply (
		recv_buffer->connection->giop_version,
		giop_recv_buffer_get_request_id (recv_buffer),
		ev->_major);

	ORBit_send_system_exception (send_buffer, ev);

	tprintf ("Return exception:\n");
	do_giop_dump_send (send_buffer);
	giop_send_buffer_write (send_buffer, recv_buffer->connection, FALSE);
	giop_send_buffer_unuse (send_buffer);
}
