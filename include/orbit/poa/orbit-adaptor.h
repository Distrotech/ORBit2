#ifndef __ORBIT_ADAPTOR_H__
#define __ORBIT_ADAPTOR_H__

void ORBit_handle_request       (CORBA_ORB orb, GIOPRecvBuffer *recv_buffer);

void ORBit_small_handle_request (ORBit_OAObject     adaptor_obj,
				 CORBA_Identifier   opname,
				 gpointer           ret,
				 gpointer          *args, 
				 CORBA_Context      ctx,
				 GIOPRecvBuffer    *recv_buffer,
				 CORBA_Environment *ev);

/*
 * ORBit_OAObject
 */

typedef gboolean            (*ORBitStateCheckFunc) (ORBit_OAObject adaptor_obj);

typedef IOP_ObjectKey_info* (*ORBitKeyGenFunc)     (ORBit_OAObject  adaptor_obj);

typedef void                (*ORBitInvokeFunc)     (ORBit_OAObject     adaptor_obj,
						    gpointer           ret,
						    gpointer          *args,
						    CORBA_Context      ctx,
						    gpointer           data, 
						    CORBA_Environment *ev);

typedef void                (*ORBitReqFunc)        (ORBit_OAObject     adaptor_obj,
						    CORBA_Identifier   opname,
						    gpointer           ret,
						    gpointer          *args,
						    CORBA_Context      ctx,
						    GIOPRecvBuffer    *recv_buffer,
						    CORBA_Environment *ev);

typedef enum {
	ORBIT_ADAPTOR_POA = 1 << 0
} ORBit_Adaptor_type;

struct ORBit_OAObject_Interface_type {
	ORBit_Adaptor_type  adaptor_type;

	ORBitStateCheckFunc is_active;
	ORBitKeyGenFunc     object_to_objkey;
	ORBitInvokeFunc     invoke;
	ORBitReqFunc        handle_request;
};

typedef struct ORBit_OAObject_Interface_type *ORBit_OAObject_Interface;

struct ORBit_OAObject_type {
	struct ORBit_RootObject_struct parent;

	CORBA_Object                   objref;

	ORBit_OAObject_Interface       interface;
};

/*
 * ORBit_ObjectAdaptor
 */

typedef struct ORBit_ObjectAdaptor_type *ORBit_ObjectAdaptor;

typedef void (*ORBitReqHandlerFunc) (ORBit_ObjectAdaptor         adaptor,
				     GIOPRecvBuffer             *recv_buffer,
				     CORBA_sequence_CORBA_octet *objkey);

struct ORBit_ObjectAdaptor_type {
	struct ORBit_RootObject_struct parent;

	ORBitReqHandlerFunc            handle_request;

	CORBA_sequence_CORBA_octet     adaptor_key;
};

#endif /* __ORBIT_ADAPTOR_H__ */
