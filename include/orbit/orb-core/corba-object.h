#ifndef CORBA_OBJECT_H
#define CORBA_OBJECT_H 1

GIOPConnection *ORBit_object_get_connection(CORBA_Object obj);

void ORBit_marshal_object(GIOPSendBuffer *buf, CORBA_Object obj);
gboolean ORBit_demarshal_object(CORBA_Object *obj, GIOPRecvBuffer *buf,
				CORBA_ORB orb);
CORBA_Object ORBit_objref_new(CORBA_ORB orb, const char *type_id);
void ORBit_start_servers (CORBA_ORB orb);

/*
 * CORBA_Object interface type data.
 */
#include <orbit/orb-core/orbit-interface.h>

#define CORBA_OBJECT_SMALL_GET_TYPE_ID    12
#define CORBA_OBJECT_SMALL_GET_IINTERFACE 13

extern ORBit_IInterface CORBA_Object__iinterface;
extern ORBit_IMethod    CORBA_Object__imethods[];

#define CORBA_Object_IMETHODS_LEN 12

#endif
