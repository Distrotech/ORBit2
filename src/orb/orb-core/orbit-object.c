#include <orbit/orb-core/orbit-object.h>

O_MUTEX_DEFINE(ORBit_RootObject_lifecycle_lock);

void
ORBit_RootObject_init(ORBit_RootObject obj, ORBit_RootObject_Interface * const interface)
{
#ifdef ORBIT_THREADSAFE
  if(!ORBit_RootObject_lifecycle_lock)
    O_MUTEX_INIT(ORBit_RootObject_lifecycle_lock);
#endif
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

  if(obj && obj->refs != ORBIT_REFCOUNT_STATIC)
    {
      g_assert(obj->refs < ORBIT_REFCOUNT_MAX && obj->refs > 0);

      obj->refs--;
      if(obj->refs == 0)
	(* obj->interface->destroy)(robj);
    }

  O_MUTEX_UNLOCK(ORBit_RootObject_lifecycle_lock);
}
