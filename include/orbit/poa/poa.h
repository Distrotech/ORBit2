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

#endif
