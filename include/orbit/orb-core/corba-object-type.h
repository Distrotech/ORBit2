#ifndef CORBA_OBJECT_TYPE_H
#define CORBA_OBJECT_TYPE_H 1

void ORBit_marshal_object(GIOPSendBuffer *buf, CORBA_Object obj);
gboolean ORBit_demarshal_object(CORBA_Object *obj, GIOPRecvBuffer *buf,
				CORBA_ORB orb);

#endif
