#ifndef ORBIT_POA_H
#define ORBIT_POA_H 1

#include "config.h"

#define ORBIT_RAND_DATA_LEN 8

/*
 * Exported by poa.c.
 */
void               ORBit_POA_handle_held_requests  (PortableServer_POA poa);

PortableServer_POA ORBit_POA_setup_root            (CORBA_ORB          orb,
						    CORBA_Environment *ev);

CORBA_boolean      ORBit_POA_deactivate            (PortableServer_POA poa,
						    CORBA_boolean      etherealize_objects,
						    CORBA_Environment  *ev);

gboolean           ORBit_POA_is_inuse              (PortableServer_POA  poa,
					            CORBA_boolean       consider_children,
                                                    CORBA_Environment  *ev);

/*
 * Exported by poa-manager.c
 */
void               ORBit_POAManager_register_poa   (PortableServer_POAManager  poa_mgr, 
						    PortableServer_POA         poa,
						    CORBA_Environment         *ev);

void               ORBit_POAManager_unregister_poa (PortableServer_POAManager  poa_mgr, 
						    PortableServer_POA         poa,
						    CORBA_Environment         *ev);
PortableServer_POAManager
                   ORBit_POAManager_new            (CORBA_ORB                  orb,
						    CORBA_Environment         *ev);

#endif /* ORBIT_POA_H */
