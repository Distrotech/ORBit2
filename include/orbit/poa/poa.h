#ifndef POA_H
#define POA_H 1

#include <orbit/poa/poa-defs.h>
struct ORBit_POAInvocation {
  ORBit_POAInvocation*                prev;
  ORBit_POAObject*                    pobj;
  PortableServer_ObjectId*            object_id;
  char                                doUnuse;
};

#include <orbit/poa/portableserver-poa-type.h>
#include <orbit/poa/portableserver-current-type.h>

void PortableServer_ServantBase__init(PortableServer_Servant p_servant,
				      CORBA_Environment *ev);
void PortableServer_ServantBase__fini(PortableServer_Servant p_servant,
				      CORBA_Environment *ev);
void ORBit_classinfo_register(PortableServer_ClassInfo *ci);
#define ORBIT_SERVANT_SET_CLASSINFO(x, y)
#endif
