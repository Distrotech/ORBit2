#include <orbit/orbit.h>

O_MUTEX_DEFINE(ORBit_RootObject_lifecycle_lock);

void
ORBit_RootObject_init(ORBit_RootObject obj, const ORBit_RootObject_Interface * interface)
{
  obj->interface = interface;
  obj->refs = 0;
}

gpointer
ORBit_RootObject_duplicate(gpointer obj)
{
  ORBit_RootObject robj = obj;

  if(robj->refs != ORBIT_REFCOUNT_STATIC)
    robj->refs++;

  return obj;
}

void
ORBit_RootObject_release(gpointer obj)
{
  ORBit_RootObject robj = obj;

  O_MUTEX_LOCK(ORBit_RootObject_lifecycle_lock);

  if(robj && robj->refs != ORBIT_REFCOUNT_STATIC)
    {
      g_assert(robj->refs < ORBIT_REFCOUNT_MAX && robj->refs > 0);

      robj->refs--;
      if(robj->refs == 0)
	(* robj->interface->destroy)(robj);
    }

  O_MUTEX_UNLOCK(ORBit_RootObject_lifecycle_lock);
}
