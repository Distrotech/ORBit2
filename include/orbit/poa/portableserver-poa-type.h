#ifndef PORTABLESERVER_POA_TYPE_H
#define PORTABLESERVER_POA_TYPE_H 1

struct PortableServer_POAManager_type {
  struct ORBit_RootObject_struct parent;

  GSList *poa_collection;
  PortableServer_POAManager_State state;
};

struct PortableServer_POA_type {
  struct ORBit_RootObject_struct parent;

  guint life_flags;
  CORBA_unsigned_long poaID;
  PortableServer_ObjectId poa_key; /* poaID + rand_data */
  char *name;
  CORBA_ORB orb;
  PortableServer_POA parent_poa;
  PortableServer_POAManager poa_manager;
  PortableServer_AdapterActivator the_activator;
  PortableServer_ServantManager servant_manager;
  ORBit_POAObject *default_pobj;

  GHashTable *oid_to_obj_map;
  GPtrArray *num_to_koid_map;
  CORBA_unsigned_long next_id;

  GSList *held_requests;
  GHashTable *child_poas;

  PortableServer_ThreadPolicyValue p_thread;
  PortableServer_LifespanPolicyValue p_lifespan;
  PortableServer_IdUniquenessPolicyValue p_id_uniqueness;
  PortableServer_IdAssignmentPolicyValue p_id_assignment;
  PortableServer_ImplicitActivationPolicyValue p_implicit_activation;
  PortableServer_ServantRetentionPolicyValue p_servant_retention;
  PortableServer_RequestProcessingPolicyValue p_request_processing;
  int obj_rand_len;
  int poa_rand_len;
  int koid_rand_len;
};

#endif
