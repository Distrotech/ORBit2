#ifndef POA_H
#define POA_H 1

#include <orbit/poa/poa-defs.h>

#include <orbit/poa/poa-policy.h>

typedef struct {
  void *_private;
} POA_PortableServer_ServantManager__epv;

typedef struct {
  void *_private;
      
  PortableServer_Servant(*incarnate) (PortableServer_Servant _servant,
				      const PortableServer_ObjectId *
				      oid,
				      const PortableServer_POA adapter,
				      CORBA_Environment * ev);
  void (*etherealize) (PortableServer_Servant _servant,
		       const PortableServer_ObjectId * oid,
		       const PortableServer_POA adapter,
		       const PortableServer_Servant serv,
		       const CORBA_boolean cleanup_in_progress,
		       const CORBA_boolean remaining_activations,
		       CORBA_Environment * ev);
} POA_PortableServer_ServantActivator__epv;
typedef struct {
  PortableServer_ServantBase__epv *_base_epv;
  POA_PortableServer_ServantManager__epv
  *PortableServer_ServantManager_epv;
  POA_PortableServer_ServantActivator__epv
  *PortableServer_ServantActivator_epv;
} POA_PortableServer_ServantActivator__vepv;
typedef struct {
  void *_private;
  POA_PortableServer_ServantActivator__vepv *vepv;
} POA_PortableServer_ServantActivator;

typedef struct {
  void *_private;
      
  PortableServer_Servant(*preinvoke) (PortableServer_Servant _servant,
				      const PortableServer_ObjectId *
				      oid,
				      const PortableServer_POA adapter,
				      const CORBA_Identifier operation,
				      PortableServer_ServantLocator_Cookie
				      * the_cookie,
				      CORBA_Environment * ev);
  void (*postinvoke) (PortableServer_Servant _servant,
		      const PortableServer_ObjectId * oid,
		      const PortableServer_POA adapter,
		      const CORBA_Identifier operation,
		      const PortableServer_ServantLocator_Cookie
		      the_cookie,
		      const PortableServer_Servant the_servant,
		      CORBA_Environment * ev);
} POA_PortableServer_ServantLocator__epv;
typedef struct {
  PortableServer_ServantBase__epv *_base_epv;
  POA_PortableServer_ServantManager__epv
  *PortableServer_ServantManager_epv;
  POA_PortableServer_ServantLocator__epv
  *PortableServer_ServantLocator_epv;
} POA_PortableServer_ServantLocator__vepv;
typedef struct {
  void *_private;
  POA_PortableServer_ServantLocator__vepv *vepv;
} POA_PortableServer_ServantLocator;

struct ORBit_POAInvocation {
  ORBit_POAInvocation*                prev;
  ORBit_POAObject*                    pobj;
  PortableServer_ObjectId*            object_id;
  char                                doUnuse;
};

#include <orbit/poa/portableserver-poa-type.h>
#include <orbit/poa/portableserver-current-type.h>

void PortableServer_ServantBase__init(PortableServer_Servant p_servant,
				      CORBA_Environment *ev);
void PortableServer_ServantBase__fini(PortableServer_Servant p_servant,
				      CORBA_Environment *ev);
void ORBit_classinfo_register(PortableServer_ClassInfo *ci);
#define ORBIT_SERVANT_SET_CLASSINFO(servant,ci) {                       \
  ((PortableServer_ServantBase *)(servant))->vepv[0]->_private = (ci);  \
}
#define ORBIT_SERVANT_TO_CLASSINFO(servant) (                           \
  (PortableServer_ClassInfo*)                                           \
  ( ((PortableServer_ServantBase *)(servant))->vepv[0]->_private )      \
)

#endif
