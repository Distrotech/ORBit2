#ifndef POA_TYPES_H
#define POA_TYPES_H 1

#if !defined(_PortableServer_Servant_defined)
#define _PortableServer_Servant_defined 1
typedef gpointer PortableServer_Servant;
#else
#error "Include mixup"
#endif

typedef struct {
  PortableServer_Servant servant;
  gpointer oid;
  int *use_count;
  GFunc death_callback;
  gpointer user_data;
} ORBit_POAObject;

typedef struct {
} ORBit_POAInvokation;

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
typedef ORBitSkeleton (*ORBit_impl_finder)(PortableServer_ServantBase *servant,
					   gpointer _ORBIT_recv_buffer,
					   gpointer *implementation);
typedef void (*ORBit_vepvmap_init)(void);

typedef struct {
  ORBit_impl_finder relay_call;
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
#define ORBIT_STUB_GetEpv(x,y) NULL

#endif
