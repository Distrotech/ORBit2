#ifndef POA_H
#define POA_H 1

#include <orbit/poa/poa-defs.h>
#include <orbit/poa/poa-types.h>
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

typedef struct ORBit_POAInvocation ORBit_POAInvocation;
struct ORBit_POAInvocation {
  ORBit_POAInvocation                *prev;
  ORBit_POAObject                     pobj;
};

#include <orbit/poa/portableserver-poa-type.h>
#include <orbit/poa/portableserver-current-type.h>

void PortableServer_ServantBase__init(PortableServer_Servant p_servant,
				      CORBA_Environment *ev);
void PortableServer_ServantBase__fini(PortableServer_Servant p_servant,
				      CORBA_Environment *ev);
void ORBit_classinfo_register(PortableServer_ClassInfo *ci);
PortableServer_ClassInfo *ORBit_classinfo_lookup(const char *type_id);
void ORBit_POAObject_post_invoke(ORBit_POAObject obj);


PortableServer_ObjectId *PortableServer_string_to_ObjectId(CORBA_char *str,
                                                           CORBA_Environment *nv);

PortableServer_ObjectId *PortableServer_wstring_to_ObjectId(CORBA_wchar *str,
                                                            CORBA_Environment *ev);

CORBA_char *PortableServer_ObjectId_to_string(PortableServer_ObjectId *oid,
                                              CORBA_Environment *ev);

CORBA_wchar *PortableServer_ObjectId_to_wstring(PortableServer_ObjectId *oid,
                                                CORBA_Environment *ev);

#endif
