#ifndef CORBA_OBJECT_H
#define CORBA_OBJECT_H 1

#define ORBit_object_get_connection(obj) \
     (((obj)->connection && (LINC_CONNECTION((obj)->connection)->status == LINC_CONNECTED))?((obj)->connection):_ORBit_object_get_connection(obj))

GIOPConnection *_ORBit_object_get_connection(CORBA_Object obj);

void ORBit_marshal_object(GIOPSendBuffer *buf, CORBA_Object obj);
gboolean ORBit_demarshal_object(CORBA_Object *obj, GIOPRecvBuffer *buf,
				CORBA_ORB orb);
CORBA_Object ORBit_objref_new(CORBA_ORB orb, const char *type_id);

/*
 * CORBA_Object interface type data.
 */
#include <orbit/orb-core/orbit-interface.h>

#define CORBA_OBJECT_SMALL_GET_TYPE_ID    12
#define CORBA_OBJECT_SMALL_GET_IINTERFACE 13

extern ORBit_IInterface CORBA_Object__itype;
extern ORBit_IMethod    CORBA_Object__imethods[];

#define CORBA_Object_IMETHODS_LEN 12

#endif
