#ifndef __ORBIT_ADAPTOR_H__
#define __ORBIT_ADAPTOR_H__

#include <glib.h>

G_BEGIN_DECLS

#ifdef ORBIT2_INTERNAL_API

/*
 * Our object key is 28 bytes long and looks like this:
 *
 * .----- adaptor prefix -----.---------- ObjectId -----------,
 * |      4            16     |      4              4         |
 * | adaptor idx | orb cookie | object idx | object id random |
 */

#define ORBIT_ADAPTOR_KEY_LEN     (128/8)
#define ORBIT_ADAPTOR_PREFIX_LEN  (sizeof (CORBA_long) + ORBIT_ADAPTOR_KEY_LEN)


void                ORBit_handle_request            (CORBA_ORB          orb, 
						     GIOPRecvBuffer    *recv_buffer);

void                ORBit_small_handle_request      (ORBit_OAObject     adaptor_obj,
						     CORBA_Identifier   opname,
						     gpointer           ret,
						     gpointer          *args, 
						     CORBA_Context      ctx,
						     GIOPRecvBuffer    *recv_buffer,
						     CORBA_Environment *ev);

gboolean            ORBit_OAObject_is_active        (ORBit_OAObject     adaptor_obj);

ORBit_ObjectKey    *ORBit_OAObject_object_to_objkey (ORBit_OAObject     adaptor_obj);

void                ORBit_OAObject_invoke           (ORBit_OAObject     adaptor_obj,
						     gpointer           ret,
						     gpointer          *args,
						     CORBA_Context      ctx,
						     gpointer           data,
						     CORBA_Environment *ev);
#endif /* ORBIT2_INTERNAL_API */

/*
 * ORBit_OAObject
 */
#if defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API)

typedef struct ORBit_ObjectAdaptor_type *ORBit_ObjectAdaptor;

typedef gboolean            (*ORBitStateCheckFunc) (ORBit_OAObject     adaptor_obj);

typedef ORBit_ObjectKey    *(*ORBitKeyGenFunc)     (ORBit_OAObject     adaptor_obj);

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
	ORBIT_ADAPTOR_POA = 1 << 0,
	ORBIT_ADAPTOR_GOA = 1 << 1
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

	ORBit_ObjectAdaptor            adaptor;
};

#endif /* defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API) */

/*
 * ORBit_ObjectAdaptor
 */

#ifdef ORBIT2_INTERNAL_API

#define ORBIT_ADAPTOR(o) ((ORBit_ObjectAdaptor) o)

typedef CORBA_sequence_CORBA_octet       ORBit_AdaptorKey;

typedef void (*ORBitReqHandlerFunc) (ORBit_ObjectAdaptor         adaptor,
				     GIOPRecvBuffer             *recv_buffer,
				     ORBit_ObjectKey            *objkey);

struct ORBit_ObjectAdaptor_type {
	struct ORBit_RootObject_struct parent;

	ORBitReqHandlerFunc            handle_request;

	ORBit_Adaptor_type             adaptor_type;
	CORBA_long                     adaptor_id;
	ORBit_AdaptorKey               adaptor_key;

	CORBA_ORB                      orb;
};

CORBA_long ORBit_adaptor_setup (ORBit_ObjectAdaptor adaptor,
				ORBit_Adaptor_type  type,
				CORBA_ORB           orb);

void       ORBit_adaptor_free  (ORBit_ObjectAdaptor adaptor);

/* Utility method */
void       ORBit_recv_buffer_return_sys_exception (GIOPRecvBuffer    *buf,
						   CORBA_Environment *ev);

#endif /* ORBIT2_INTERNAL_API */

G_END_DECLS

#endif /* __ORBIT_ADAPTOR_H__ */
