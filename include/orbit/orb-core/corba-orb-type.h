#ifndef CORBA_ORB_TYPE
#define CORBA_ORB_TYPE 1

#include <orbit/orb-core/orb-types.h>
#include <orbit/orb-core/orbit-object.h>

struct CORBA_ORB_type {
  struct ORBit_RootObject_struct root_object;

  GList *servers;
  GPtrArray *poas;
};

#endif
