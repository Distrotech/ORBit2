#ifndef _POA_BASICS_H_
#define _POA_BASICS_H_ 1

typedef struct ORBit_POAObject_type *ORBit_POAObject;
typedef struct ORBit_OAObject_type  *ORBit_OAObject;

#if !defined(_PortableServer_Servant_defined)
#define _PortableServer_Servant_defined 1
	typedef gpointer PortableServer_Servant;
#else
#error "Include mixup: poa-defs.h included before poa-basics.h"
#endif

#if !defined(ORBIT_DECL_PortableServer_POA) && !defined(_PortableServer_POA_defined)
#define ORBIT_DECL_PortableServer_POA 1
#define _PortableServer_POA_defined 1
	typedef struct PortableServer_POA_type *PortableServer_POA;
#endif

typedef struct {
	void                *_private;
	void               (*finalize)    (PortableServer_Servant  servant,
					   CORBA_Environment      *ev);
	PortableServer_POA (*default_POA) (PortableServer_Servant  servant,
					   CORBA_Environment      *ev);
} PortableServer_ServantBase__epv;

typedef PortableServer_ServantBase__epv *PortableServer_ServantBase__vepv;

typedef struct {
	void                             *_private;
	PortableServer_ServantBase__vepv *vepv;
} PortableServer_ServantBase;

typedef gshort ORBit_VepvIdx;

typedef void               (*ORBitSkeleton)      (PortableServer_ServantBase *servant,
						  gpointer                    recv_buffer,
						  CORBA_Environment          *ev,
						  gpointer                    implementation);

typedef void               (*ORBitSmallSkeleton) (PortableServer_ServantBase *servant,
						  gpointer                    ret,
						  gpointer                   *args,
						  gpointer                    ctx,
						  CORBA_Environment          *ev,
						  gpointer                    implementation);

typedef ORBitSkeleton      (*ORBit_impl_finder)  (PortableServer_ServantBase *servant,
						  gpointer                    recv_buffer,
						  gpointer                   *implementation);

typedef ORBitSmallSkeleton (*ORBit_small_impl_finder)
						 (PortableServer_ServantBase *servant,
						  const char                 *method,
						  gpointer                   *m_data,
						  gpointer                   *implementation);

#endif /* _POA_BASICS_H_ 1 */
