#ifndef CORBA_ORB_TYPE
#define CORBA_ORB_TYPE 1

#include <orbit/orb-core/orb-types.h>
#include <orbit/orb-core/orbit-object.h>

typedef CORBA_char *CORBA_ORB_ObjectId;
typedef struct {
  CORBA_unsigned_long _maximum, _length;
  CORBA_ORB_ObjectId *_buffer;
  CORBA_boolean _release;
} CORBA_ORB_ObjectIdList;

typedef struct _CORBA_ORB {
  struct _ORBit_RootObject root_object;

  GList *servers;
  GPtrArray *poas;
} *CORBA_ORB;

#endif
