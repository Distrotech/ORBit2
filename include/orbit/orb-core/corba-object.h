#ifndef CORBA_OBJECT_H
#define CORBA_OBJECT_H 1

#include <glib.h>

G_BEGIN_DECLS

#ifdef ORBIT2_INTERNAL_API

GIOPConnection *ORBit_object_get_connection (CORBA_Object    obj);

void            ORBit_marshal_object        (GIOPSendBuffer *buf,
					     CORBA_Object    obj);
gboolean        ORBit_demarshal_object      (CORBA_Object   *obj,
					     GIOPRecvBuffer *buf,
					     CORBA_ORB       orb);

CORBA_Object    ORBit_objref_new            (CORBA_ORB       orb,
					     GQuark          type_id);

#endif /* ORBIT2_INTERNAL_API */

/*
 * CORBA_Object interface type data.
 */
#include <orbit/orb-core/orbit-interface.h>

#define CORBA_OBJECT_SMALL_GET_TYPE_ID    12
#define CORBA_OBJECT_SMALL_GET_IINTERFACE 13

extern ORBit_IInterface CORBA_Object__iinterface;
extern ORBit_IMethod    CORBA_Object__imethods[];

#define CORBA_Object_IMETHODS_LEN 12

G_END_DECLS

#endif /* CORBA_OBJECT_H */
