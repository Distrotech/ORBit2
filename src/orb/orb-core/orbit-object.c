#include <orbit/orbit.h>

static glong alive_root_objects = 0;
static glong total_refs = 0;

int
ORBit_RootObject_shutdown (void)
{
	int valid_running = 1; /* The ORB */

	if (!ORBit_RootObject_lifecycle_lock &&
	    alive_root_objects - valid_running)
		g_warning ("ORB: a total of %ld refs to %ld ORB "
			   "objects were leaked",
			   total_refs - valid_running,
			   alive_root_objects - valid_running);
	else if (total_refs - valid_running)
		g_warning ("ORB: a total of %ld refs to ORB "
			   "objects were leaked",
			   total_refs - valid_running);
	else
		return 0;

	return 1;
}

void
ORBit_RootObject_init (ORBit_RootObject obj,
		       const ORBit_RootObject_Interface * interface)
{
	if (!ORBit_RootObject_lifecycle_lock) /* No locking */
		alive_root_objects++;

	obj->interface = interface;
	obj->refs = 0;
}

gpointer
ORBit_RootObject_duplicate (gpointer obj)
{
	ORBit_RootObject robj = obj;

	if (robj && robj->refs != ORBIT_REFCOUNT_STATIC) {
		LINC_MUTEX_LOCK   (ORBit_RootObject_lifecycle_lock);
		robj->refs++;
		total_refs++;
		LINC_MUTEX_UNLOCK (ORBit_RootObject_lifecycle_lock);
	}

	return obj;
}

gpointer
ORBit_RootObject_duplicate_T (gpointer obj)
{
	ORBit_RootObject robj = obj;

	if (robj && robj->refs != ORBIT_REFCOUNT_STATIC) {
		robj->refs++;
		total_refs++;
	}

	return obj;
}

static void
do_unref (ORBit_RootObject robj)
{
	g_assert (robj->refs < ORBIT_REFCOUNT_MAX && robj->refs > 0);

	robj->refs--;
	total_refs--;

	if (robj->refs == 0) {
		if (!ORBit_RootObject_lifecycle_lock) /* No locking */
			alive_root_objects--;

		if (robj->interface && robj->interface->destroy)
			robj->interface->destroy (robj);
		else
			g_free (robj);
	}
}

void
ORBit_RootObject_release_T (gpointer obj)
{
	ORBit_RootObject robj = obj;

	if (robj && robj->refs != ORBIT_REFCOUNT_STATIC)
		do_unref (robj);
}

void
ORBit_RootObject_release (gpointer obj)
{
	ORBit_RootObject robj = obj;

	if (robj && robj->refs != ORBIT_REFCOUNT_STATIC) {

		LINC_MUTEX_LOCK   (ORBit_RootObject_lifecycle_lock);

		do_unref (robj);

		LINC_MUTEX_UNLOCK (ORBit_RootObject_lifecycle_lock);
	}
}

