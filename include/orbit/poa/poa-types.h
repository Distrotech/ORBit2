#ifndef POA_TYPES_H
#define POA_TYPES_H 1

#include <orbit/poa/orbit-adaptor.h>

#define ORBit_LifeF_NeedPostInvoke      (1<<0)
#define ORBit_LifeF_DoEtherealize       (1<<1)
#define ORBit_LifeF_IsCleanup           (1<<2)
#define ORBit_LifeF_DeactivateDo        (1<<4)
#define ORBit_LifeF_Deactivating        (1<<5)
#define ORBit_LifeF_Deactivated         (1<<6)
#define ORBit_LifeF_DestroyDo           (1<<8)
#define ORBit_LifeF_Destroying          (1<<9)
#define ORBit_LifeF_Destroyed           (1<<10)

struct ORBit_POAObject_type {
	struct ORBit_OAObject_type     base;

	PortableServer_Servant         servant;
	PortableServer_POA             poa;
	PortableServer_ObjectId       *object_id;

#ifdef ORBIT_BYPASS_MAPCACHE
	ORBit_VepvIdx                 *vepvmap_cache;
#endif

	guint16                        life_flags;
	guint16                        use_cnt;
};

typedef void (*ORBit_vepvmap_init)(ORBit_VepvIdx *map);

typedef struct {
  ORBit_impl_finder       relay_call;
  ORBit_small_impl_finder small_relay_call;
  const char             *class_name;
  CORBA_unsigned_long    *class_id;
  ORBit_vepvmap_init      init_vepvmap;
  ORBit_VepvIdx*          vepvmap;
  int                     vepvlen;
  ORBit_IInterface       *idata;
} PortableServer_ClassInfo;


#define ORBIT_SERVANT_SET_CLASSINFO(servant,ci) { 			\
  ((PortableServer_ServantBase *)(servant))->vepv[0]->_private = (ci);	\
}
#define ORBIT_SERVANT_TO_CLASSINFO(servant) ( 				\
  (PortableServer_ClassInfo*) 						\
  ( ((PortableServer_ServantBase *)(servant))->vepv[0]->_private )	\
)
#define ORBIT_SERVANT_TO_POAOBJECT_LIST(servant) (                      \
  (GSList *)                                                            \
  ( ((PortableServer_ServantBase *)(servant))->_private )               \
)
#define ORBIT_SERVANT_TO_POAOBJECT_LIST_ADDR(servant) (                 \
  (GSList **)                                                           \
  ( &((PortableServer_ServantBase *)(servant))->_private )              \
)
#define ORBIT_SERVANT_TO_FIRST_POAOBJECT(servant) (                               \
  ( (PortableServer_ServantBase *)(servant) )->_private == NULL ? NULL :          \
    (ORBit_POAObject)                                                             \
    ( ( (GSList *)( (PortableServer_ServantBase *)(servant) )->_private )->data ) \
)
#define ORBIT_SERVANT_MAJOR_TO_EPVPTR(servant,major) 			\
  ( ((PortableServer_ServantBase *)(servant))->vepv[major] )

#ifdef ORBIT_BYPASS_MAPCACHE
#define ORBIT_POAOBJECT_TO_EPVIDX(pobj,clsid) \
  ( (pobj)->vepvmap_cache[(clsid)] )
#else
#define ORBIT_POAOBJECT_TO_EPVIDX(pobj,clsid) \
  ( ORBIT_SERVANT_TO_CLASSINFO((pobj)->servant)->vepvmap[(clsid)] )
#endif
#define ORBIT_POAOBJECT_TO_EPVPTR(pobj,clsid) \
  ORBIT_SERVANT_MAJOR_TO_EPVPTR((pobj)->servant, \
    ORBIT_POAOBJECT_TO_EPVIDX((pobj),(clsid)) )

#define ORBIT_SERVANT_TO_EPVIDX(servant,clsid) \
  ( ORBIT_SERVANT_TO_CLASSINFO(servant)->vepvmap[(clsid)] )

#define ORBIT_SERVANT_TO_EPVPTR(servant,clsid) \
  ORBIT_SERVANT_MAJOR_TO_EPVPTR((servant), \
    ORBIT_SERVANT_TO_EPVIDX((servant),(clsid)) )

#define ORBIT_STUB_GetPoaObj(x) ((ORBit_POAObject)(((CORBA_Object)x)->adaptor_obj))

#define ORBIT_STUB_IsBypass(obj, classid) \
		(((CORBA_Object)obj)->adaptor_obj && \
		((CORBA_Object)obj)->adaptor_obj->interface->adaptor_type == ORBIT_ADAPTOR_POA && \
		((ORBit_POAObject)((CORBA_Object)obj)->adaptor_obj)->servant && classid)

#define ORBIT_STUB_GetServant(obj) \
		(((ORBit_POAObject)((CORBA_Object)obj)->adaptor_obj)->servant)

#define ORBIT_STUB_GetEpv(obj,clsid) \
		ORBIT_POAOBJECT_TO_EPVPTR(((ORBit_POAObject)((CORBA_Object)obj)->adaptor_obj), (clsid))

#ifdef ORBIT_IN_PROC_COMPLIANT
#define ORBIT_STUB_PreCall(obj, iframe) {		\
  ++( ((ORBit_POAObject)(obj)->adaptor_obj)->use_cnt );	\
  iframe.pobj = ((ORBit_POAObject)(obj)->adaptor_obj);	\
  iframe.prev = (obj)->orb->poa_current_invocations;	\
  (obj)->orb->poa_current_invocations = &iframe;	\
}

#define ORBIT_STUB_PostCall(obj, iframe) {						\
  (obj)->orb->poa_current_invocations = iframe.prev;					\
  --( ((ORBit_POAObject)(obj)->adaptor_obj)->use_cnt );					\
  if ( ((ORBit_POAObject)(obj)->adaptor_obj)->life_flags & ORBit_LifeF_NeedPostInvoke )	\
	ORBit_POAObject_post_invoke( ((ORBit_POAObject)(obj)->adaptor_obj) );		\
}
#else
#define ORBIT_STUB_PreCall(x,y)
#define ORBIT_STUB_PostCall(x,y)
#endif

#endif
