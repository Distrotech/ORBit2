#ifndef ORBIT_OBJECT_H
#define ORBIT_OBJECT_H 1

#include <glib.h>
#include <linc/linc.h>

typedef enum {
	ORBIT_ROT_NULL,
	ORBIT_ROT_OBJREF,
	/* values below here all signify psuedo-objects */
	ORBIT_ROT_ORB,
	ORBIT_ROT_POA,
	ORBIT_ROT_POAMANAGER,
	ORBIT_ROT_POLICY,
	ORBIT_ROT_TYPECODE,
	ORBIT_ROT_REQUEST,
	ORBIT_ROT_SERVERREQUEST,
	ORBIT_ROT_CONTEXT,
	ORBIT_ROT_DYNANY,
	ORBIT_ROT_POAOBJECT,
	ORBIT_ROT_ORBGROUP,
	ORBIT_ROT_POACURRENT
} ORBit_RootObject_Type;

typedef struct ORBit_RootObject_struct *ORBit_RootObject;

typedef void (* ORBit_RootObject_DestroyFunc)(ORBit_RootObject obj);

typedef struct _ORBit_RootObject_Interface {
  ORBit_RootObject_Type type;
  ORBit_RootObject_DestroyFunc destroy;
} ORBit_RootObject_Interface;

struct ORBit_RootObject_struct {
  const ORBit_RootObject_Interface * interface;
  int refs;
};

#define ORBIT_REFCOUNT_STATIC -10

/* Used to determine whether the refcount is valid or not */
#define ORBIT_REFCOUNT_MAX (1<<20)

#define ORBIT_ROOT_OBJECT(obj) ((ORBit_RootObject)(obj))
#define ORBIT_ROOT_OBJECT_TYPE(obj) (((ORBit_RootObject)(obj))->interface->type)

void ORBit_RootObject_init(ORBit_RootObject obj, const ORBit_RootObject_Interface * interface);

gpointer ORBit_RootObject_duplicate(gpointer obj);
void ORBit_RootObject_release(gpointer obj);

O_MUTEX_DEFINE_EXTERN(ORBit_RootObject_lifecycle_lock);

#endif
