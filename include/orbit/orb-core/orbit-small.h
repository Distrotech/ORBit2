/*
 * An attempt to shrink the beast to a managable size.
 */
#ifndef CORBA_SMALL_H
#define CORBA_SMALL_H 1

#include <orbit/GIOP/giop.h>
#include <orbit/orb-core/orbit-interface.h>

typedef struct {
	CORBA_unsigned_long           version;
	ORBit_IInterface            **interfaces;
	CORBA_sequence_CORBA_TypeCode types;
} ORBit_IModule;


gpointer       ORBit_small_alloc       (CORBA_TypeCode      tc);
gpointer       ORBit_small_allocbuf    (CORBA_TypeCode      tc,
					CORBA_unsigned_long length);
void           ORBit_small_freekids    (CORBA_TypeCode      tc,
					gpointer            p,
					gpointer            d);

typedef enum {
	ORBIT_CONNECTION_CONNECTED,
	ORBIT_CONNECTION_CONNECTING,
	ORBIT_CONNECTION_DISCONNECTED,
	ORBIT_CONNECTION_IN_PROC
} ORBitConnectionStatus;

gpointer              ORBit_small_get_servant           (CORBA_Object obj);
ORBitConnectionStatus ORBit_small_get_connection_status (CORBA_Object obj);
ORBitConnectionStatus ORBit_small_listen_for_broken     (CORBA_Object obj,
							 GCallback    fn,
							 gpointer     user_data);

#if defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API)

void           ORBit_small_invoke_stub (CORBA_Object        object,
					ORBit_IMethod      *m_data,
					gpointer            ret,
					gpointer           *args,
					CORBA_Context       ctx,
					CORBA_Environment  *ev);

#endif /* defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API) */

#ifdef ORBIT2_INTERNAL_API

void           ORBit_small_invoke_adaptor (ORBit_OAObject     adaptor_obj,
					   GIOPRecvBuffer    *recv_buffer,
					   ORBit_IMethod     *m_data,
					   gpointer           data,
					   CORBA_Environment *ev);

#endif /* ORBIT2_INTERNAL_API */

/* Type library work */
CORBA_char       *ORBit_small_get_type_id         (CORBA_Object       object,
						   CORBA_Environment *ev);
ORBit_IInterface *ORBit_small_get_iinterface      (CORBA_Object       opt_object,
					           const CORBA_char  *repo_id,
						   CORBA_Environment *ev);
gboolean          ORBit_small_load_typelib        (const char        *libname);
CORBA_sequence_CORBA_TypeCode *
                  ORBit_small_get_types           (const char        *name);
CORBA_sequence_ORBit_IInterface *
                  ORBit_small_get_iinterfaces     (const char        *name);

typedef struct _ORBitAsyncQueueEntry ORBitAsyncQueueEntry;

typedef void (*ORBitAsyncInvokeFunc) (CORBA_Object          object,
				      ORBit_IMethod        *m_data,
				      ORBitAsyncQueueEntry *aqe,
				      gpointer              user_data, 
				      CORBA_Environment    *ev);

/* Various bits for Async work */
void ORBit_small_invoke_async        (CORBA_Object           obj,
				      ORBit_IMethod         *m_data,
				      ORBitAsyncInvokeFunc   fn,
				      gpointer               user_data,
				      gpointer              *args,
				      CORBA_Context          ctx,
				      CORBA_Environment     *ev);

void ORBit_small_demarshal_async     (ORBitAsyncQueueEntry  *aqe,
				      gpointer               ret,
				      gpointer              *args,
				      CORBA_Environment     *ev);

#endif /* CORBA_SMALL_H */
