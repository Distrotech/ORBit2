/*
 * An attempt to shrink the beast to a managable size.
 */
#ifndef CORBA_SMALL_H
#define CORBA_SMALL_H 1

#include <libIDL/IDL.h>
#include <orbit/GIOP/giop.h>
#include <orbit/orb-core/orbit-interface.h>

void           giop_dump_send          (GIOPSendBuffer *send_buffer);
void           giop_dump_recv          (GIOPRecvBuffer *recv_buffer);

gpointer       ORBit_small_alloc       (CORBA_TypeCode      tc);
gpointer       ORBit_small_allocbuf    (CORBA_TypeCode      tc,
					CORBA_unsigned_long length);
void           ORBit_small_freekids    (CORBA_TypeCode      tc,
					gpointer            p,
					gpointer            d);

void           ORBit_small_invoke      (CORBA_Object                object,
					ORBit_IMethod              *m_data,
					gpointer                    ret,
					gpointer                   *args,
					CORBA_Context               ctx,
					CORBA_Environment          *ev);

void           ORBit_small_invoke_stub (CORBA_Object        object,
					ORBit_IMethod      *m_data,
					gpointer            ret,
					gpointer           *args,
					CORBA_Context       ctx,
					CORBA_Environment  *ev);

void           ORBit_small_invoke_poa  (PortableServer_ServantBase *servant,
					GIOPRecvBuffer             *recv_buffer,
					ORBit_IMethod              *m_data,
					ORBitSmallSkeleton          small_skel,
					gpointer                    impl,
					CORBA_Environment          *ev);

CORBA_char       *ORBit_small_get_type_id         (CORBA_Object       object,
						   CORBA_Environment *ev);
ORBit_IInterface *ORBit_small_get_iinterface      (CORBA_Object       opt_object,
					           const CORBA_char  *repo_id,
						   CORBA_Environment *ev);
gboolean          ORBit_small_load_typelib        (const char        *libname);

#endif /* CORBA_SMALL_H */
