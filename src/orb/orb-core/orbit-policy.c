#include <stdarg.h>
#include "orbit-policy.h"
#include "orbit-debug.h"

GType
ORBit_policy_ex_get_type (void)
{
	return 0;
}

ORBitPolicy *
ORBit_policy_new (GType        type,
		  const char  *first_prop,
		  ...)
{
	return NULL;
}

ORBitPolicy *
ORBit_policy_ref (ORBitPolicy *p)
{
	if (p) {
		LINK_MUTEX_LOCK (ORBit_RootObject_lifecycle_lock);
		g_object_ref (p);
		LINK_MUTEX_UNLOCK (ORBit_RootObject_lifecycle_lock);
	}

	return p;
}

void
ORBit_policy_unref (ORBitPolicy *p)
{
	if (!p)
		return;

	LINK_MUTEX_LOCK (ORBit_RootObject_lifecycle_lock);
	g_object_unref (p);
	LINK_MUTEX_UNLOCK (ORBit_RootObject_lifecycle_lock);
}

void
ORBit_object_set_policy (CORBA_Object obj,
			 ORBitPolicy *p)
{
	if (obj == CORBA_OBJECT_NIL)
		return;
}

static GStaticPrivate policy_private = G_STATIC_PRIVATE_INIT;

static void
policy_queue_free (gpointer data)
{
	ORBitPolicy *p;
	GQueue *queue = data;

	if (queue->length)
		dprintf (MESSAGES, "Leaked %d policies\n", queue->length);

	while ((p = g_queue_pop_head (queue)))
		ORBit_policy_unref (p);

	g_queue_free (queue);
}

void
ORBit_policy_push (ORBitPolicy *p)
{
	GQueue *queue;

	if (!(queue = g_static_private_get (&policy_private))) {
		queue = g_queue_new ();
		/* FIXME: should check the queue on free */
		g_static_private_set (&policy_private, queue,
				      policy_queue_free);
	}
	
	g_queue_push_head (queue, ORBit_policy_ref (p));
}

void
ORBit_policy_pop (void)
{
	GQueue *queue;

	if (!(queue = g_static_private_get (&policy_private)))
		g_warning ("No policy queue to pop from");
	else {
		ORBitPolicy *p;

		p = g_queue_pop_head (queue);
		ORBit_policy_unref (p);
	}
}

gboolean
ORBit_policy_validate (ORBitPolicy *policy)
{
	return TRUE;
}
