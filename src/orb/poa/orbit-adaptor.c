#include <orbit/orbit.h>
#include <orbit/poa/orbit-adaptor.h>

static ORBit_ObjectAdaptor
ORBit_adaptor_find (CORBA_ORB orb, CORBA_sequence_CORBA_octet *objkey)
{
	ORBit_ObjectAdaptor   adaptor;
	gint32                adaptorId;

	if (objkey->_length < sizeof(gint32))
		return NULL;

	memcpy (&adaptorId, objkey->_buffer, sizeof(gint32));

	if (adaptorId < 0 || adaptorId >= orb->adaptors->len)
		return NULL;

	adaptor = g_ptr_array_index (orb->adaptors, adaptorId);

	if (objkey->_length < adaptor->adaptor_key._length)
		return NULL;

	if (memcmp (objkey->_buffer, adaptor->adaptor_key._buffer,
	            adaptor->adaptor_key._length))
		return NULL;

	return adaptor;
}

void
ORBit_handle_request (CORBA_ORB orb, GIOPRecvBuffer *recv_buffer)
{
	ORBit_ObjectAdaptor         adaptor;
	CORBA_sequence_CORBA_octet *objkey;

	objkey = giop_recv_buffer_get_objkey (recv_buffer);
	adaptor = ORBit_adaptor_find (orb, objkey);

	if (!adaptor)
		return;

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
