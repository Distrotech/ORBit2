#include <config.h>
#include <string.h>
#include <unistd.h>

#include <orbit/orbit.h>
#include <orbit/poa/orbit-adaptor.h>

#include "orbit-poa.h"
#include "../orb-core/orbit-debug.h"

int
ORBit_adaptor_setup (ORBit_ObjectAdaptor adaptor,
		     CORBA_ORB           orb)
{
	CORBA_long *tptr;
	int         adaptor_id;

	adaptor_id = orb->adaptors->len;

	g_ptr_array_set_size (orb->adaptors, adaptor_id + 1);
	g_ptr_array_index    (orb->adaptors, adaptor_id) = adaptor;

	adaptor->adaptor_key._length  = ORBIT_ADAPTOR_KEY_LEN;
	adaptor->adaptor_key._buffer  =
		CORBA_sequence_CORBA_octet_allocbuf (ORBIT_ADAPTOR_KEY_LEN);
	adaptor->adaptor_key._release = CORBA_TRUE;

	tptr = (CORBA_long *) adaptor->adaptor_key._buffer;
	*tptr = adaptor_id;
	ORBit_genrand_buf (&orb->genrand, 
			   adaptor->adaptor_key._buffer + sizeof (CORBA_long),
			   ORBIT_RAND_DATA_LEN);

	return adaptor_id;
}

static ORBit_ObjectAdaptor
ORBit_adaptor_find (CORBA_ORB orb, ORBit_ObjectKey *objkey)
{
	ORBit_ObjectAdaptor   adaptor;
	gint32                adaptorId;

	if (!objkey)
		return NULL;

	if (objkey->_length < ORBIT_ADAPTOR_KEY_LEN)
		return NULL;

	memcpy (&adaptorId, objkey->_buffer, sizeof (gint32));

	if (adaptorId < 0 || adaptorId >= orb->adaptors->len)
		return NULL;

	adaptor = g_ptr_array_index (orb->adaptors, adaptorId);

	if (memcmp (objkey->_buffer, adaptor->adaptor_key._buffer,
	            ORBIT_ADAPTOR_KEY_LEN))
		return NULL;

	return adaptor;
}

void
ORBit_handle_request (CORBA_ORB orb, GIOPRecvBuffer *recv_buffer)
{
	ORBit_ObjectAdaptor  adaptor;
	ORBit_ObjectKey     *objkey;

	objkey = giop_recv_buffer_get_objkey (recv_buffer);
	adaptor = ORBit_adaptor_find (orb, objkey);

	if (!adaptor || !objkey) {
		tprintf ("Error: failed to find adaptor or objkey for "
			 "object while invoking method '%s'",
			 giop_recv_buffer_get_opname (recv_buffer));
		return;
	}
	dprintf ("p %d: handle request '%s'\n", getpid (),
		 giop_recv_buffer_get_opname (recv_buffer));
	adaptor->handle_request (adaptor, recv_buffer, objkey);
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
