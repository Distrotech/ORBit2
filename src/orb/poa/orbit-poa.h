#ifndef ORBIT_POA_H
#define ORBIT_POA_H 1

typedef struct ORBit_POAKOid_struct {
        gushort                   oid_length;
        /* oid octets comes here */
        /* rand_data octets come here */
} ORBit_POAKOid;
#define ORBIT_POAKOID_OidLenOf(koid) \
        ((koid)->oid_length)
#define ORBIT_POAKOID_OidBufOf(koid) \
        ( (CORBA_octet*) (((char*)(koid))+sizeof(*(koid))) )
#define ORBIT_POAKOID_RandBufOf(koid) \
        ( (CORBA_octet*) (((char*)(koid))+sizeof(*(koid))+(koid)->oid_length) )

void ORBit_POA_handle_held_requests(PortableServer_POA poa);
gboolean ORBit_POA_is_inuse(PortableServer_POA poa,
			    CORBA_boolean consider_children,
			    CORBA_Environment *ev);
void ORBit_POA_deactivate(PortableServer_POA poa,
			  CORBA_boolean etherealize_objects,
			  CORBA_Environment *ev);
gboolean ORBit_POA_destroy(PortableServer_POA poa,
			   CORBA_boolean etherealize_objects,
			   CORBA_Environment *ev);
void ORBit_POA_add_child(PortableServer_POA poa,
			 PortableServer_POA child, 
			 CORBA_Environment *ev);
PortableServer_POA ORBit_POA_setup_root(CORBA_ORB orb, CORBA_Environment *ev);
PortableServer_POA ORBit_POA_new(CORBA_ORB orb, const CORBA_char *nom,
				 const PortableServer_POAManager manager,
				 const CORBA_PolicyList *policies,
				 CORBA_Environment *ev);
ORBit_POAObject *ORBit_POA_create_object(PortableServer_POA poa,
					 const PortableServer_ObjectId *oid,
					 CORBA_boolean isDefault,
					 CORBA_Environment *ev);
CORBA_Object ORBit_POA_oid_to_ref(PortableServer_POA poa,
				  const PortableServer_ObjectId *oid,
				  const CORBA_RepositoryId intf,
				  CORBA_Environment *ev);
ORBit_POAObject *ORBit_POA_oid_to_obj(PortableServer_POA poa,
				      const PortableServer_ObjectId *oid,
				      gboolean active,
				      CORBA_Environment *ev);
CORBA_Object ORBit_POA_obj_to_ref(PortableServer_POA poa,
				  ORBit_POAObject *pobj,
				  const CORBA_RepositoryId intf,
				  CORBA_Environment *ev);
void ORBit_POA_make_sysoid(PortableServer_POA poa, PortableServer_ObjectId *oid);
void ORBit_POA_activate_object(PortableServer_POA poa, 
			       ORBit_POAObject *pobj,
			       PortableServer_ServantBase *servant, 
			       CORBA_Environment *ev);
void ORBit_POA_deactivate_object(PortableServer_POA poa, ORBit_POAObject *pobj,
				 CORBA_boolean do_etherealize,
				 CORBA_boolean is_cleanup);
extern int ORBit_class_assignment_counter;


#endif
