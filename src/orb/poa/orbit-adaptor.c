#include <config.h>
#include <string.h>
#include <unistd.h>

#include <orbit/orbit.h>
#include <orbit/poa/orbit-adaptor.h>

#include "orbit-poa.h"
#include "../orb-core/orbit-debug.h"

void
ORBit_ObjectAdaptor_set_thread_hint (ORBit_ObjectAdaptor adaptor,
				     ORBitThreadHint     thread_hint)
{
	g_return_if_fail (adaptor != NULL);
	g_return_if_fail (thread_hint >= ORBIT_THREAD_HINT_NONE &&
			  thread_hint <= ORBIT_THREAD_HINT_PER_CONNECTION);

	adaptor->thread_hint = thread_hint;
}

ORBitThreadHint
ORBit_ObjectAdaptor_get_thread_hint (ORBit_ObjectAdaptor adaptor)
{
	g_return_val_if_fail (adaptor != NULL, ORBIT_THREAD_HINT_NONE);

	return adaptor->thread_hint;
}

CORBA_long
ORBit_adaptor_setup (ORBit_ObjectAdaptor adaptor,
		     CORBA_ORB           orb)
{
	int adaptor_id;
	CORBA_long *tptr;

	LINC_MUTEX_LOCK (ORBit_RootObject_lifecycle_lock);
	{
		adaptor_id = orb->adaptors->len;

		g_ptr_array_set_size (orb->adaptors, adaptor_id + 1);
		g_ptr_array_index    (orb->adaptors, adaptor_id) = adaptor;
	}
	LINC_MUTEX_UNLOCK (ORBit_RootObject_lifecycle_lock);

	adaptor->thread_hint = ORBIT_THREAD_HINT_NONE;

	adaptor->adaptor_key._length  = ORBIT_ADAPTOR_PREFIX_LEN;
	adaptor->adaptor_key._buffer  =
		CORBA_sequence_CORBA_octet_allocbuf (adaptor->adaptor_key._length);
	adaptor->adaptor_key._release = CORBA_TRUE;

	ORBit_genuid_buffer (adaptor->adaptor_key._buffer + sizeof (CORBA_long),
			     ORBIT_ADAPTOR_KEY_LEN, ORBIT_GENUID_COOKIE);

	tptr = (CORBA_long *) adaptor->adaptor_key._buffer;
	*tptr = adaptor_id;

	return *tptr;
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

	LINC_MUTEX_LOCK (ORBit_RootObject_lifecycle_lock);
	{
		adaptor = g_ptr_array_index (orb->adaptors, adaptorId);

		if (memcmp (objkey->_buffer,
			    adaptor->adaptor_key._buffer,
			    ORBIT_ADAPTOR_PREFIX_LEN))
			adaptor = NULL;
		else
			ORBit_RootObject_duplicate_T (adaptor);
	}
	LINC_MUTEX_UNLOCK (ORBit_RootObject_lifecycle_lock);

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
		CORBA_Environment env;

		CORBA_exception_init (&env);

		tprintf ("Error: failed to find adaptor or objkey for "
			 "object while invoking method '%s'",
			 giop_recv_buffer_get_opname (recv_buffer));
		
		CORBA_exception_set_system (
			&env, ex_CORBA_OBJECT_NOT_EXIST, 
			CORBA_COMPLETED_NO);
		ORBit_recv_buffer_return_sys_exception (recv_buffer, &env);

		CORBA_exception_free (&env);

	} else {
		dprintf (MESSAGES, "p %d: handle request '%s'\n",
			 getpid (),
			 giop_recv_buffer_get_opname (recv_buffer));

		adaptor->handle_request (adaptor, recv_buffer, objkey);
	}

	ORBit_RootObject_release (adaptor);
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
