#ifndef CORBA_OBJECT_H
#define CORBA_OBJECT_H 1

CORBA_Object CORBA_Object_duplicate(CORBA_Object obj, CORBA_Environment *ev);
void CORBA_Object_release(CORBA_Object obj, CORBA_Environment *ev);
void ORBit_marshal_object(GIOPSendBuffer *buf, CORBA_Object obj);
gboolean ORBit_demarshal_object(CORBA_Object *obj, GIOPRecvBuffer *buf,
				CORBA_ORB orb);

#endif
