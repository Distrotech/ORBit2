#ifndef ORB_H
#define ORB_H 1

#include <orbit/orb-core/orb-types.h>
#include <orbit/orb-core/orbit-object.h>

typedef struct _CORBA_ORB {
  struct _ORBit_RootObject root_object;

  GList *servers;
  GPtrArray *poas;
  CORBA_Context default_ctx;
} *CORBA_ORB;

CORBA_ORB CORBA_ORB_init(int *argc, char **argv, CORBA_ORBid orb_identifier, CORBA_Environment *ev);

CORBA_Current *CORBA_ORB_get_current(CORBA_ORB orb, CORBA_Environment *ev);

/* ORBit extension */
void CORBA_ORB_set_initial_reference(CORBA_ORB orb, CORBA_ORB_ObjectId identifier, CORBA_Object obj, CORBA_Environment *ev);
#endif
