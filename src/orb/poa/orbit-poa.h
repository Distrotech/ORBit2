#ifndef ORBIT_POA_H
#define ORBIT_POA_H 1

#include "config.h"

#define ORBIT_RAND_DATA_LEN 8

void ORBit_POA_handle_held_requests(PortableServer_POA poa);
gboolean ORBit_POA_is_inuse(PortableServer_POA poa,
			    CORBA_boolean consider_children,
			    CORBA_Environment *ev);
CORBA_boolean ORBit_POA_deactivate(PortableServer_POA poa,
				   CORBA_boolean etherealize_objects,
				   CORBA_Environment *ev);
gboolean ORBit_POA_destroy(PortableServer_POA poa,
			   CORBA_boolean etherealize_objects,
			   CORBA_Environment *ev);
PortableServer_POA ORBit_POA_setup_root(CORBA_ORB orb, CORBA_Environment *ev);
PortableServer_POA ORBit_POA_new(CORBA_ORB orb, const CORBA_char *nom,
				 const PortableServer_POAManager manager,
				 const CORBA_PolicyList *policies,
				 CORBA_Environment *ev);
ORBit_POAObject ORBit_POA_oid_to_obj(PortableServer_POA poa,
				     const PortableServer_ObjectId *oid,
				     gboolean active,
				     CORBA_Environment *ev);
CORBA_Object ORBit_POA_obj_to_ref(PortableServer_POA poa,
				  ORBit_POAObject pobj,
				  const CORBA_RepositoryId intf,
				  CORBA_Environment *ev);
void ORBit_POA_activate_object(PortableServer_POA poa, 
			       ORBit_POAObject pobj,
			       PortableServer_ServantBase *servant, 
			       CORBA_Environment *ev);
void ORBit_POA_deactivate_object(PortableServer_POA poa, ORBit_POAObject pobj,
				 CORBA_boolean do_etherealize,
				 CORBA_boolean is_cleanup);
void ORBit_POAObject_post_invoke(ORBit_POAObject pobj);
void ORBit_POAManager_register_poa(PortableServer_POAManager poa_mgr, 
				   PortableServer_POA poa,
				   CORBA_Environment *ev);
void ORBit_POAManager_unregister_poa(PortableServer_POAManager poa_mgr, 
				     PortableServer_POA poa,
				     CORBA_Environment *ev);
PortableServer_POAManager ORBit_POAManager_new(CORBA_ORB orb,
					       CORBA_Environment *ev);

#endif
