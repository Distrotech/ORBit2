#ifndef POA_TYPES_H
#define POA_TYPES_H 1

#if !defined(_PortableServer_Servant_defined)
#define _PortableServer_Servant_defined 1
typedef gpointer PortableServer_Servant;
#else
#error "Include mixup"
#endif

#define ORBit_LifeF_NeedPostInvoke      (1<<0)
#define ORBit_LifeF_DoEtherealize       (1<<1)
#define ORBit_LifeF_IsCleanup           (1<<2)
#define ORBit_LifeF_DeactivateDo        (1<<4)
#define ORBit_LifeF_Deactivating        (1<<5)
#define ORBit_LifeF_Deactivated         (1<<6)
#define ORBit_LifeF_DestroyDo           (1<<8)
#define ORBit_LifeF_Destroying          (1<<9)
#define ORBit_LifeF_Destroyed           (1<<10)

typedef struct {
  PortableServer_Servant servant;
  gpointer poa;
  gpointer object_id;
  int *use_count;
  GFunc death_callback;
  gpointer user_data;
  guint16 life_flags;
  guint16 use_cnt; /* method invokations */
} ORBit_POAObject;

#define ORBIT_SERVANT_TO_ORB(servant) \
  (((PortableServer_POA)ORBIT_SERVANT_TO_POAOBJECT(servant)->poa)->orb )
#define ORBIT_SERVANT_TO_POAOBJECT(s) \
((ORBit_POAObject *)((PortableServer_ServantBase*)(s))->_private)

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

typedef struct ORBit_POAInvocation ORBit_POAInvocation;

#if !defined(ORBIT_DECL_PortableServer_POA) && !defined(_PortableServer_POA_defined)
#define ORBIT_DECL_PortableServer_POA 1
#define _PortableServer_POA_defined 1
   typedef struct PortableServer_POA_type *PortableServer_POA;
#endif

typedef struct {
  void *_private;
  void (*finalize)(PortableServer_Servant servant, CORBA_Environment *ev);
  PortableServer_POA (*default_POA)(PortableServer_Servant servant,
				    CORBA_Environment *ev);
} PortableServer_ServantBase__epv;

typedef PortableServer_ServantBase__epv *PortableServer_ServantBase__vepv;

typedef struct {
  void *_private;
  PortableServer_ServantBase__vepv *vepv;
} PortableServer_ServantBase;

typedef gshort ORBit_VepvIdx;

typedef void (*ORBitSkeleton)(PortableServer_ServantBase *_ORBIT_servant,
                              gpointer _ORBIT_recv_buffer,
                              CORBA_Environment *ev,
                              gpointer implementation);

typedef void (*ORBitSmallSkeleton) (PortableServer_ServantBase *_ORBIT_servant,
				    gpointer ret, gpointer *args,
				    gpointer ctx, CORBA_Environment *ev,
				    gpointer implementation);

typedef ORBitSkeleton (*ORBit_impl_finder)(PortableServer_ServantBase *servant,
					   gpointer _ORBIT_recv_buffer,
					   gpointer *implementation);
typedef ORBitSmallSkeleton (*ORBit_small_impl_finder)(PortableServer_ServantBase *servant,
						      const char                 *method,
						      gpointer                   *m_data,
						      gpointer                   *implementation);
typedef void (*ORBit_vepvmap_init)(ORBit_VepvIdx *map);

typedef struct {
  ORBit_impl_finder relay_call;
  ORBit_small_impl_finder small_relay_call;
  const char *class_name;
  CORBA_unsigned_long *class_id;
  ORBit_vepvmap_init init_vepvmap;
  ORBit_VepvIdx*          vepvmap;
  int                     vepvlen;
} PortableServer_ClassInfo;

#define ORBIT_STUB_GetPoaObj(x) (((CORBA_Object)x)->bypass_obj)
#define ORBIT_STUB_IsBypass(obj, classid) (((CORBA_Object)obj)->bypass_obj)
#define ORBIT_STUB_PreCall(x,y)
#define ORBIT_STUB_PostCall(x,y)
#define ORBIT_STUB_GetServant(x) NULL
#define ORBIT_STUB_GetEpv(x,y) \
	ORBIT_POAOBJECT_TO_EPVPTR( (x)->bypass_obj, (y))
#endif
