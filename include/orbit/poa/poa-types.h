#ifndef POA_TYPES_H
#define POA_TYPES_H 1

#include <orbit/poa/orbit-adaptor.h>

G_BEGIN_DECLS

#if defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API)

typedef struct {
	void (*dummy_padding) (void);
	ORBit_impl_finder        impl_finder;
	const char              *class_name;
	CORBA_unsigned_long     *class_id;
	ORBit_VepvIdx           *vepvmap;
	ORBit_IInterface        *idata;
} PortableServer_ClassInfo;

struct ORBit_POAObject_type {
	struct ORBit_OAObject_type     base;

	PortableServer_Servant         servant;
	PortableServer_POA             poa;
	PortableServer_ObjectId       *object_id;

	ORBit_VepvIdx                 *vepvmap_cache;

	guint16                        life_flags;
	guint16                        use_cnt;

	ORBit_POAObject                next;
};

#endif /* defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API) */

#ifdef ORBIT2_INTERNAL_API

#define ORBit_LifeF_NeedPostInvoke      (1<<0)
#define ORBit_LifeF_DoEtherealize       (1<<1)
#define ORBit_LifeF_IsCleanup           (1<<2)
#define ORBit_LifeF_DeactivateDo        (1<<4)
#define ORBit_LifeF_Deactivating        (1<<5)
#define ORBit_LifeF_Deactivated         (1<<6)
#define ORBit_LifeF_DestroyDo           (1<<8)
#define ORBit_LifeF_Destroying          (1<<9)
#define ORBit_LifeF_Destroyed           (1<<10)

#endif /* ORBIT2_INTERNAL_API */

#if defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API)

#define ORBIT_VEPV_OFFSET(vepv_type,epv_member) \
	((CORBA_unsigned_long) ((long)((guint8 *) &((vepv_type *) 0)->epv_member)) / sizeof (GFunc))

#define ORBIT_SERVANT_TO_CLASSINFO(servant) ( 				\
  (PortableServer_ClassInfo*) 						\
  ( ((PortableServer_ServantBase *)(servant))->vepv[0]->_private )	\
)

#define ORBIT_SERVANT_SET_CLASSINFO(servant,ci) { 			\
  ((PortableServer_ServantBase *)(servant))->vepv[0]->_private = (ci);	\
}

#define ORBIT_SERVANT_MAJOR_TO_EPVPTR(servant, major)				\
	( ((PortableServer_ServantBase *)(servant))->vepv [major] )
 
#define ORBIT_POAOBJECT_TO_EPVPTR(pobj, clsid)								\
		ORBIT_SERVANT_MAJOR_TO_EPVPTR ((pobj)->servant, (pobj)->vepvmap_cache [(clsid)])

#define ORBIT_STUB_IsBypass(obj, classid)								\
		(classid && (obj) && ((CORBA_Object)obj)->adaptor_obj &&				\
		((CORBA_Object)obj)->adaptor_obj->interface->adaptor_type == ORBIT_ADAPTOR_POA &&	\
		((ORBit_POAObject)((CORBA_Object)obj)->adaptor_obj)->servant)

#define ORBIT_STUB_GetEpv(obj, clsid) \
		((gpointer)ORBIT_POAOBJECT_TO_EPVPTR(((ORBit_POAObject)((CORBA_Object)obj)->adaptor_obj), (clsid)))

#define ORBIT_STUB_GetServant(obj) \
		(((ORBit_POAObject)((CORBA_Object)obj)->adaptor_obj)->servant)

#ifdef ORBIT_IN_PROC_COMPLIANT
#define ORBIT_STUB_PreCall(obj) {                                   \
	++( ((ORBit_POAObject)(obj)->adaptor_obj)->use_cnt );	    \
	(obj)->orb->current_invocations =                           \
		g_slist_prepend ((obj)->orb->current_invocations,   \
				 (obj)->adaptor_obj);               \
}

#define ORBIT_STUB_PostCall(obj) {                                                             \
	(obj)->orb->current_invocations =                                                      \
                g_slist_remove ((obj)->orb->current_invocations, pobj);                        \
	--(((ORBit_POAObject)(obj)->adaptor_obj)->use_cnt);                                    \
	if (((ORBit_POAObject)(obj)->adaptor_obj)->life_flags & ORBit_LifeF_NeedPostInvoke)    \
		ORBit_POAObject_post_invoke (((ORBit_POAObject)(obj)->adaptor_obj));           \
}
#else
#define ORBIT_STUB_PreCall(x)
#define ORBIT_STUB_PostCall(x)
#endif

#endif /* defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API) */

G_END_DECLS

#endif
