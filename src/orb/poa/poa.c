#include <orbit/orbit.h>
#include "orbit-poa.h"
#include <string.h>
#include <stdlib.h>

#include "orb-core-export.h"
#include "orbit-poa-export.h"
#include "corba-ops.h"

static PortableServer_Servant ORBit_POA_ServantManager_use_servant(
				     PortableServer_POA poa,
				     ORBit_POAObject pobj,
				     CORBA_Identifier opname,
				     PortableServer_ServantLocator_Cookie *the_cookie,
				     PortableServer_ObjectId *oid,
				     CORBA_Environment *ev );

static void ORBit_POA_ServantManager_unuse_servant(
				       PortableServer_POA poa,
				       ORBit_POAObject pobj,
				       CORBA_Identifier opname,
				       PortableServer_ServantLocator_Cookie cookie,
				       PortableServer_ObjectId *oid,
				       PortableServer_Servant servant,
				       CORBA_Environment *ev );

static void ORBit_POA_handle_request (PortableServer_POA          poa,
				      GIOPRecvBuffer             *recv_buffer,
				      ORBit_ObjectKey            *objkey);


static void               ORBit_POAObject_invoke           (ORBit_POAObject    pobj,
							    gpointer           ret,
							    gpointer          *args,
							    CORBA_Context      ctx,
							    gpointer           data,
							    CORBA_Environment *ev);

static void               ORBit_POAObject_handle_request    (ORBit_POAObject    pobj,
							     CORBA_Identifier   opname,
							     gpointer           ret,
							     gpointer          *args,
							     CORBA_Context      ctx,
							     GIOPRecvBuffer    *recv_buffer,
							     CORBA_Environment *ev);

static void               ORBit_POA_deactivate_object       (PortableServer_POA  poa,
							     ORBit_POAObject     pobj,
							     CORBA_boolean       do_etherealize,
							     CORBA_boolean       is_cleanup);

static gboolean           ORBit_POA_okey_to_objid           (PortableServer_POA       poa,
							     ORBit_ObjectKey         *objkey,
							     PortableServer_ObjectId *objid);

static GHashTable *ORBit_class_assignments = NULL;
static guint ORBit_class_assignment_counter = 0;

static void
ORBit_POAList_add_child (gpointer key, gpointer value, gpointer data)
{
	PortableServer_POA      poa    = value;
	PortableServer_POAList *retval = data;

	retval->_buffer[retval->_maximum++] = CORBA_Object_duplicate ((CORBA_Object)poa, NULL);
}

/* PortableServer_Current interface */
static void
ORBit_POACurrent_free_fn(ORBit_RootObject obj_in)
{
  PortableServer_Current poacur = (PortableServer_Current)obj_in;
  ORBit_RootObject_release_T(poacur->orb);
  poacur->orb = NULL;
  g_free(poacur);
}

static const ORBit_RootObject_Interface ORBit_POACurrent_epv = {
  ORBIT_ROT_POACURRENT,
  ORBit_POACurrent_free_fn
};

/**
    The returned object has already been dup'd; caller must free
    if it doesnt want it!
**/
PortableServer_Current
ORBit_POACurrent_new(CORBA_ORB orb)
{
  PortableServer_Current poacur;

  poacur = (PortableServer_Current) 
    g_new0(struct PortableServer_Current_type, 1);
  ORBit_RootObject_init(&poacur->parent, &ORBit_POACurrent_epv);
  poacur->orb = ORBit_RootObject_duplicate(orb);
  return ORBit_RootObject_duplicate(poacur);
}

static ORBit_POAInvocation*
ORBit_POACurrent_get_invocation(PortableServer_Current obj,
				CORBA_Environment *ev)
{
  g_assert( obj && obj->parent.interface->type == ORBIT_ROT_POACURRENT );

  if ( obj->orb->poa_current_invocations == 0 )
    {
      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_PortableServer_Current_NoContext,
			  NULL);
      return 0;
    }

  return obj->orb->poa_current_invocations;
}

CORBA_unsigned_long
ORBit_classinfo_lookup_id(const char *type_id)
{
	PortableServer_ClassInfo *ci;

	if (!ORBit_class_assignments)
		return 0;

	ci = g_hash_table_lookup (ORBit_class_assignments, type_id);

	if (!ci)
		return 0;

	return *ci->class_id;
}

PortableServer_ClassInfo *
ORBit_classinfo_lookup(const char *type_id)
{
	if (!ORBit_class_assignments)
		return NULL;

	return g_hash_table_lookup (ORBit_class_assignments, type_id);
}

void
ORBit_classinfo_register(PortableServer_ClassInfo *ci)
{
  if(!ORBit_class_assignments)
    ORBit_class_assignments = g_hash_table_new(g_str_hash, g_str_equal);

  if ( *(ci->class_id) != 0 )
    return;	/* already registered! */

  /* This needs to be pre-increment - we don't want to give out
     classid 0, because (a) that is reserved for the base Object class
     (b) all the routines allocate a new id if the variable
     storing their ID == 0 */
  *(ci->class_id) = ++ORBit_class_assignment_counter;
  g_hash_table_insert(ORBit_class_assignments, 
		      (gpointer)ci->class_name, ci);
}

static void check_child_poa_inuse(gpointer key, gpointer value, gpointer data)
{
  gboolean *is_inuse = data;
  PortableServer_POA poa = value;
  if(ORBit_POA_is_inuse(poa, CORBA_TRUE, NULL))
    *is_inuse = TRUE;
}

typedef struct TraverseInfo {
    int			num_in_use;
    gboolean		do_deact;
    PortableServer_POA	poa;
    CORBA_boolean	do_etherealize;
    CORBA_boolean	is_cleanup;
} TraverseInfo;

static void
traverse_cb(PortableServer_ObjectId *oid, ORBit_POAObject pobj, 
	    TraverseInfo *info)
{
    if ( pobj->use_cnt > 0 ) {
	++(info->num_in_use);
    }
    if ( info->do_deact ) {
        ORBit_POA_deactivate_object(info->poa, pobj,
			     info->do_etherealize,
			     info->is_cleanup);
    }
}

/*
 * traverse through oid_to_obj_map and remove
 * any destroyed POAObjects.
 * This *can't* be done in traverse_cb.
 */
static gboolean
remove_cb(PortableServer_ObjectId *oid, ORBit_POAObject pobj, gpointer dummy)
{
  if ( pobj->life_flags & ORBit_LifeF_Destroyed ) {
    g_free( pobj );
    return TRUE;
  }
  return FALSE;
}

gboolean
ORBit_POA_is_inuse(PortableServer_POA poa,
		   CORBA_boolean consider_children,
		   CORBA_Environment *ev)
{
  if ( poa->use_cnt > 0 ) 
    return CORBA_TRUE;
  if ( consider_children )
    {
      gboolean is_inuse = FALSE;
      if (poa->child_poas)
        g_hash_table_foreach(poa->child_poas, check_child_poa_inuse, &is_inuse);
      if(is_inuse)
	return TRUE;
    }
  if ( poa->oid_to_obj_map )
    {
      TraverseInfo	info;
      info.do_deact = 0;
      info.num_in_use = 0;
      g_hash_table_foreach(poa->oid_to_obj_map, (GHFunc)traverse_cb, &info);
      if ( info.num_in_use )
	return CORBA_TRUE;
    }

  return CORBA_FALSE;
}

static PortableServer_ObjectId*
ORBit_POA_new_system_objid (PortableServer_POA poa)
{
	PortableServer_ObjectId *objid;
	int                      randlen;

	g_assert (poa->p_id_assignment == PortableServer_SYSTEM_ID);

	randlen = ORBIT_RAND_DATA_LEN;

	objid           = PortableServer_ObjectId__alloc ();
	objid->_length  = objid->_maximum = sizeof (CORBA_unsigned_long) + ORBIT_RAND_DATA_LEN;
	objid->_buffer  = PortableServer_ObjectId_allocbuf (objid->_length);
	objid->_release = CORBA_TRUE;

	*(guint32 *)(objid->_buffer) = ++(poa->next_sysid);

	if (randlen > 0)
		ORBit_genrand_buf (&poa->orb->genrand,
				   objid->_buffer + sizeof (CORBA_unsigned_long),
				   randlen);

	return objid;
}

static ORBit_ObjectKey*
ORBit_POAObject_object_to_objkey (ORBit_POAObject pobj)
{
	ORBit_ObjectAdaptor         adaptor;
	ORBit_ObjectKey            *objkey;
	gpointer                    mem;

	g_return_val_if_fail (pobj != NULL, NULL);

	adaptor = (ORBit_ObjectAdaptor)pobj->poa;

	objkey           = CORBA_sequence_CORBA_octet__alloc ();
	objkey->_length  = adaptor->adaptor_key._length + pobj->object_id->_length;
	objkey->_maximum = objkey->_length;
	objkey->_buffer  = CORBA_sequence_CORBA_octet_allocbuf (objkey->_length);
	objkey->_release = CORBA_TRUE;

	mem = objkey->_buffer;
	memcpy (mem, adaptor->adaptor_key._buffer, adaptor->adaptor_key._length);

	mem += adaptor->adaptor_key._length;
	memcpy (mem, pobj->object_id->_buffer, pobj->object_id->_length);

	return objkey;
}

static ORBit_POAObject
ORBit_POA_oid_to_obj(PortableServer_POA poa,
		     const PortableServer_ObjectId *oid,
		     gboolean only_active,
		     CORBA_Environment *ev)
{
  ORBit_POAObject pobj;

  /*
   * FIXME, FIXME, FIXME
   */
  if ( poa->p_servant_retention != PortableServer_RETAIN )
    {
      if ( ev )
	CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			    ex_PortableServer_POA_WrongPolicy, NULL);
      return NULL;
    }

  pobj = g_hash_table_lookup(poa->oid_to_obj_map, oid);

  if ( only_active && pobj && pobj->servant==0 )
    {
      if ( ev )
	CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			    ex_PortableServer_POA_ObjectNotActive, NULL);
      return NULL;
    }

  return pobj;
}

static void
ORBit_POA_set_life(PortableServer_POA poa, 
		   CORBA_boolean etherealize_objects, int action_do)
{
  if ( (poa->life_flags 
	& (ORBit_LifeF_DeactivateDo|ORBit_LifeF_DestroyDo))==0 )
    {
      if ( etherealize_objects )
	poa->life_flags |= ORBit_LifeF_DoEtherealize;
    }
  poa->life_flags |= action_do;
}

static void
ORBit_POA_add_child(PortableServer_POA poa,
		    PortableServer_POA child)
{
 /* released in ORBit_POA_remove_child */
 child->parent_poa = ORBit_RootObject_duplicate(poa);
 g_hash_table_insert(poa->child_poas, child->name, child);
}

static void
ORBit_POA_remove_child(PortableServer_POA poa,
		       PortableServer_POA child_poa,
		       CORBA_Environment *ev)
{
	g_assert( child_poa->parent_poa == poa );
	g_hash_table_remove(poa->child_poas, child_poa->name);
	child_poa->parent_poa = CORBA_OBJECT_NIL;
	ORBit_RootObject_release(poa);
}

static gboolean
ORBit_POA_destroy(PortableServer_POA poa,
		  CORBA_boolean etherealize_objects,
		  CORBA_Environment *ev)
{
  CORBA_ORB orb = poa->orb;
  int       cpidx;
  int       numobjs;

  g_assert( orb != NULL );

  ORBit_POA_set_life(poa, etherealize_objects, ORBit_LifeF_DestroyDo);
  if ( poa->life_flags & ORBit_LifeF_Destroyed )
    return TRUE;	/* already did it */
  if ( poa->life_flags & (ORBit_LifeF_Deactivating|ORBit_LifeF_Destroying) )
    return FALSE;	/* recursion */
  poa->life_flags |= ORBit_LifeF_Destroying;

  /* Destroying the children is tricky, b/c they may die
     * while we are traversing. We traverse over the
     * ORB's global list (rather than poa->child_poas) 
     * to avoid walking into dead children.
     */
  for (cpidx=0; cpidx < orb->adaptors->len; cpidx++ ) {
    PortableServer_POA cpoa = g_ptr_array_index(orb->adaptors, cpidx);
    if ( cpoa && cpoa->parent_poa == poa ) {
      ORBit_POA_destroy(cpoa, etherealize_objects, ev);
    }
  }
  /* Get rid of our default servant, if we have one.
     * This usage is non-standard.
     * It does not make any attempt to etherialize the servant.
     */
  PortableServer_POA_set_servant(poa, NULL, ev);

  /*
     * Get rid of all our children.
     */
  if ( g_hash_table_size(poa->child_poas) > 0
       || poa->use_cnt
       || !ORBit_POA_deactivate(poa, etherealize_objects, ev) ) {
    poa->life_flags &= ~ORBit_LifeF_Destroying;
    return CORBA_FALSE;
  }

  /* We're commited at this point.
     * Remove links so POA's name can be re-used. Most memory
     * will be free'd up during POA_release. */
  if(poa->poa_manager)
    ORBit_POAManager_unregister_poa(poa->poa_manager, poa, ev);
  if ( poa->parent_poa )
    ORBit_POA_remove_child(poa->parent_poa, poa, ev);

  /*
   * Don't clear the RootPOA entry.
   */
  if ( poa->poaID > 0 ) {
    g_ptr_array_index(orb->adaptors, poa->poaID) = NULL;
    poa->poaID = -1;
  }

  /* each objref holds a POAObj, and each POAObj holds a ref 
     * to the POA. In addition, the app can hold open refs
     * to the POA itself.
     */
  numobjs = poa->oid_to_obj_map ? g_hash_table_size(poa->oid_to_obj_map) : 0;
  g_assert( ((ORBit_RootObject)poa)->refs > numobjs );

  poa->life_flags |= ORBit_LifeF_Destroyed;
  poa->life_flags &= ~ORBit_LifeF_Destroying;
  ORBit_RootObject_release(poa);
  /* 
   * At this point we should hold no refs to a POA unless it is   
   * the RootPOA for which we will still have one  ref that will  
   * be released in CORBA_ORB_destroy. The application and 
   * POAObjects may still hold refs to the POA.
   */
  g_assert( g_ptr_array_index(orb->adaptors, 0) != NULL );

  return CORBA_TRUE;
}

/**
    Returns TRUE if POA (and all its objects) are sucessfully
    deactivated (and optionally etherealized). Returns FALSE
    if this cannot be performed because some object is currently
    in-use servicing some request. Note that deactivating
    has no affect of children POAs.
**/
CORBA_boolean
ORBit_POA_deactivate(PortableServer_POA poa, CORBA_boolean etherealize_objects,
		     CORBA_Environment *ev)
{
    CORBA_boolean	done = CORBA_TRUE;

    ORBit_POA_set_life(poa, etherealize_objects, ORBit_LifeF_DeactivateDo);
    if ( poa->life_flags & ORBit_LifeF_Deactivated )
	return TRUE;	/* already did it */
    if ( poa->life_flags & ORBit_LifeF_Deactivating )
	return FALSE;	/* recursion */
    poa->life_flags |= ORBit_LifeF_Deactivating;

    /* bounce all pending requested (OBJECT_NOT_EXIST
     * exceptions raised); none should get requeued. */
    ORBit_POA_handle_held_requests(poa);
    g_assert( poa->held_requests == 0 );

    if ( poa->p_servant_retention == PortableServer_RETAIN ) {
	TraverseInfo	info;
	info.num_in_use = 0;
	info.do_deact = 1;
	info.poa = poa;
	info.do_etherealize = (poa->life_flags&ORBit_LifeF_DoEtherealize)
		  		?CORBA_TRUE:CORBA_FALSE;
	info.is_cleanup = TRUE;
    	g_assert( poa->oid_to_obj_map );
	g_hash_table_foreach(poa->oid_to_obj_map, (GHFunc)traverse_cb, &info);
	g_hash_table_foreach_remove(poa->oid_to_obj_map, (GHRFunc)remove_cb, NULL);
    	done = info.num_in_use == 0 ? CORBA_TRUE : CORBA_FALSE;
    }
    if ( done )
    	poa->life_flags |= ORBit_LifeF_Deactivated;
    poa->life_flags &= ~ORBit_LifeF_Deactivating;
    return done;
}

void
ORBit_POA_handle_held_requests(PortableServer_POA poa)
{
  GSList	*reqlist = poa->held_requests, *req;
  poa->held_requests = 0;
  /* zero out the held_requests first, because a given request may
     * get re-queued below.
     */
  for ( req=reqlist; req; req=req->next) {
    GIOPRecvBuffer *buf = req->data;
    ORBit_handle_request (poa->orb, buf);
  }
  g_slist_free(reqlist);
}

static void
ORBit_POA_free_fn(ORBit_RootObject obj_in)
{
    PortableServer_POA poa = (PortableServer_POA) obj_in;
    ORBit_RootObject_release_T (poa->orb);
    g_hash_table_destroy (poa->oid_to_obj_map);
    g_hash_table_destroy (poa->child_poas);
    ORBit_free_T (poa->name);
    ORBit_free_T (((ORBit_ObjectAdaptor)poa)->adaptor_key._buffer);
    poa->orb = NULL;
    g_free (poa);
}

static const ORBit_RootObject_Interface ORBit_POA_epv = {
    ORBIT_ROT_ADAPTOR,
    ORBit_POA_free_fn
};


static guint
ORBit_ObjectId_sysid_hash(gconstpointer ptr)
{
  const PortableServer_ObjectId *oid = ptr;
    return *(guint*)oid->_buffer;
}

guint
ORBit_sequence_CORBA_octet_hash(gconstpointer ptr)
{
  const CORBA_sequence_CORBA_octet *so = ptr;
	const char *s = (char*)so->_buffer;
  	const char *p, *e = ((char *)so->_buffer) + so->_length;
  	guint h=0, g;

  	for(p = s; p < e; p ++) {
    		h = ( h << 4 ) + *p;
    		if ( ( g = h & 0xf0000000 ) ) {
      			h = h ^ (g >> 24);
      			h = h ^ g;
    		}
  	}
  	return h;
}

/*
 * Returns TRUE if s1 and s2 are the same, FALSE otherwise.
 * Note that this is what the glib hash module wants.
 */
gint
ORBit_sequence_CORBA_octet_equal(gconstpointer p1, gconstpointer p2)
{
  const CORBA_sequence_CORBA_octet *s1 = p1;
  const CORBA_sequence_CORBA_octet *s2 = p2;
    int	same;
    same = s1->_length == s2->_length 
      && memcmp(s1->_buffer,s2->_buffer,s1->_length)==0;
    return same;
}


static void
ORBit_POA_set_policy(PortableServer_POA poa,
		     CORBA_Policy policy,
		     CORBA_Environment *ev)
{
  switch(CORBA_Policy__get_policy_type(policy, ev))
    {
    case PortableServer_THREAD_POLICY_ID:
      poa->p_thread = ORBit_Policy_get(policy);
      break;
    case PortableServer_LIFESPAN_POLICY_ID:
      poa->p_lifespan = ORBit_Policy_get(policy);
      break;
    case PortableServer_ID_UNIQUENESS_POLICY_ID:
      poa->p_id_uniqueness = ORBit_Policy_get(policy);
      break;
    case PortableServer_ID_ASSIGNMENT_POLICY_ID:
      poa->p_id_assignment = ORBit_Policy_get(policy);
      break;
    case PortableServer_IMPLICIT_ACTIVATION_POLICY_ID:
      poa->p_implicit_activation = ORBit_Policy_get(policy);
      break;
    case PortableServer_SERVANT_RETENTION_POLICY_ID:
      poa->p_servant_retention = ORBit_Policy_get(policy);
      break;
    case PortableServer_REQUEST_PROCESSING_POLICY_ID:
      poa->p_request_processing = ORBit_Policy_get(policy);
      break;
    case ORBit_PortableServer_OKEYRAND_POLICY_ID:
      {
	poa->poa_rand_len = ORBit_Policy_get(policy);
      }
      break;
    default:
      g_warning("Unknown policy type, cannot set it on this POA");
    }
}

static void
ORBit_POA_check_policy_conflicts(PortableServer_POA poa,
				 CORBA_Environment *ev)
{
  gboolean bad = FALSE;
  if ( poa->p_lifespan == PortableServer_TRANSIENT )
    {
      if ( poa->poa_rand_len < 0 ) poa->poa_rand_len = ORBIT_RAND_DATA_LEN;
    }
  if ( poa->p_lifespan == PortableServer_PERSISTENT )
    {
      if ( poa->poa_rand_len < 0 ) poa->poa_rand_len = 0;
      if ( poa->poa_rand_len!=0)
	bad = TRUE;
#if 0
      if ( poa->orb->cnx.ipv4==0 || !poa->orb->cnx.ipv4_isPersistent )
	{
	  g_error("PERSISTENT POAs require -ORBIPv4Port option.");
	  bad = TRUE;
	}
#endif
    }

  /* Check for those policy combinations that aren't allowed */
  if ( bad ||
       (poa->p_servant_retention == PortableServer_NON_RETAIN &&
	poa->p_request_processing == PortableServer_USE_ACTIVE_OBJECT_MAP_ONLY) ||
      
       (poa->p_request_processing == PortableServer_USE_DEFAULT_SERVANT &&
	poa->p_id_uniqueness == PortableServer_UNIQUE_ID) ||
      
       (poa->p_implicit_activation == PortableServer_IMPLICIT_ACTIVATION &&
	(poa->p_id_assignment == PortableServer_USER_ID ||
	 poa->p_servant_retention == PortableServer_NON_RETAIN))
       )
    {
      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_PortableServer_POA_InvalidPolicy,
			  NULL);
    }
}

static void
ORBit_POA_set_policylist(PortableServer_POA poa,
			 const CORBA_PolicyList *policies,
			 CORBA_Environment *ev)
{
  CORBA_unsigned_long i;

  for(i = 0; i < policies->_length; i++)
    {
      if(ev->_major != CORBA_NO_EXCEPTION)
	break;
      ORBit_POA_set_policy(poa, policies->_buffer[i], ev);
    }
}

static PortableServer_POA
ORBit_POA_new(CORBA_ORB orb, const CORBA_char *nom,
	      PortableServer_POAManager manager,
	      const CORBA_PolicyList *policies,
	      CORBA_Environment *ev)
{
  PortableServer_POA   poa;
  ORBit_ObjectAdaptor  adaptor;
  CORBA_long          *tptr;
  
  poa = g_new0(struct PortableServer_POA_type, 1);
  ORBit_RootObject_init((ORBit_RootObject)poa, &ORBit_POA_epv);
  /* released in ORBit_POA_destroy */
  ORBit_RootObject_duplicate(poa);
  if(!manager)
    manager = ORBit_POAManager_new(orb, ev);
  if(ev->_major != CORBA_NO_EXCEPTION) goto error;

  adaptor = (ORBit_ObjectAdaptor)poa;
  adaptor->handle_request = (ORBitReqHandlerFunc)ORBit_POA_handle_request;

  poa->child_poas = g_hash_table_new(g_str_hash, g_str_equal);
  poa->orb = ORBit_RootObject_duplicate(orb);
  poa->poaID = orb->adaptors->len;
  g_ptr_array_set_size(orb->adaptors, orb->adaptors->len + 1);
  g_ptr_array_index(orb->adaptors, poa->poaID) = poa;

  /* Need to initialise poa policies etc.. here */
  poa->p_thread = PortableServer_ORB_CTRL_MODEL;
  poa->p_lifespan = PortableServer_TRANSIENT;
  poa->p_id_uniqueness = PortableServer_UNIQUE_ID;
  poa->p_id_assignment = PortableServer_SYSTEM_ID;
  poa->p_servant_retention = PortableServer_RETAIN;
  poa->p_request_processing = PortableServer_USE_ACTIVE_OBJECT_MAP_ONLY;
  poa->p_implicit_activation = PortableServer_NO_IMPLICIT_ACTIVATION;

  if (policies)
    {
      ORBit_POA_set_policylist(poa, policies, ev);
      if(ev->_major != CORBA_NO_EXCEPTION) goto error;
    }
  /* check_policy also sets up some defaults, so need to always call */
  ORBit_POA_check_policy_conflicts(poa, ev);

  poa->poa_rand_len = 8;
  
  poa->name = CORBA_string_dup(nom);

  adaptor->adaptor_key._length  = sizeof(CORBA_long) + poa->poa_rand_len;
  adaptor->adaptor_key._buffer  =
		CORBA_sequence_CORBA_octet_allocbuf(adaptor->adaptor_key._length);
  adaptor->adaptor_key._release = CORBA_TRUE;
  
  tptr = (CORBA_long *)adaptor->adaptor_key._buffer;
  *tptr = poa->poaID;
  if(poa->poa_rand_len)
    ORBit_genrand_buf(&poa->orb->genrand,
		      adaptor->adaptor_key._buffer + sizeof(CORBA_long),
		      poa->poa_rand_len);

  if ( poa->p_id_assignment == PortableServer_SYSTEM_ID )
    poa->oid_to_obj_map = g_hash_table_new(ORBit_ObjectId_sysid_hash,
					   ORBit_sequence_CORBA_octet_equal);
  else /* USER_ID */
    poa->oid_to_obj_map = g_hash_table_new(ORBit_sequence_CORBA_octet_hash,
					   ORBit_sequence_CORBA_octet_equal);

  ORBit_POAManager_register_poa(manager, poa, ev);

  return poa;
error:
  g_free(poa); /* FIXME */
  return NULL;
}

static CORBA_Object
ORBit_POA_obj_to_ref(PortableServer_POA poa,
		     ORBit_POAObject pobj,
		     const CORBA_RepositoryId intf,
		     CORBA_Environment *ev)
{
  PortableServer_ObjectId *oid;
  CORBA_Object             objref;
  const char              *type_id = intf;

  g_assert( pobj && ((ORBit_OAObject)pobj)->objref == NULL );

  if(!type_id)
    {
      g_assert( pobj->servant );
      type_id = ORBIT_SERVANT_TO_CLASSINFO(pobj->servant)->class_name;
    }

  g_assert(type_id);

  oid = pobj->object_id;

  objref = ORBit_objref_new(poa->poa_manager->orb, type_id);

  /* released by CORBA_Object_release */
  objref->adaptor_obj = ORBit_RootObject_duplicate(pobj);

  ((ORBit_OAObject)pobj)->objref = ORBit_RootObject_duplicate(objref);

  return objref;
}

PortableServer_POA
ORBit_POA_setup_root (CORBA_ORB orb, CORBA_Environment *ev)
{
	PortableServer_POA poa;
	CORBA_Policy       policybuf[1];
	CORBA_PolicyList   policies = {1, 1, (CORBA_Object *)policybuf, CORBA_FALSE};

	/*
	 * The only non-default policy used by 
	 * the RootPOA is IMPLICIT ACTIVATION.
	 */
	policies._buffer[0] = (CORBA_Policy)
		PortableServer_POA_create_implicit_activation_policy (
							NULL,
							PortableServer_IMPLICIT_ACTIVATION,
							ev);


	poa = ORBit_POA_new (orb, "RootPOA", CORBA_OBJECT_NIL, &policies, ev);
 
	CORBA_Policy_destroy (policies._buffer[0], ev);

	return poa;
}

static gboolean
ORBit_POAObject_is_active (ORBit_POAObject pobj)
{
	if (pobj && pobj->servant)
		return TRUE;

	return FALSE;
}

/*
 * POAObject RootObject stuff
 */
static void
ORBit_POAObject_release_cb(ORBit_RootObject robj)
{
  ORBit_POAObject    pobj = (ORBit_POAObject) robj;
  PortableServer_POA poa = pobj->poa;
  PortableServer_ObjectId *object_id;
 
  /* object *must* be deactivated */
  g_assert (pobj->servant == NULL);

  object_id = pobj->object_id;
  pobj->object_id = NULL;

  /*
   * Don't want to remove from oid_to_obj_map if we 
   * are currently traversing across it !
   * Just mark it as destroyed
   */
  if ((poa->life_flags & ORBit_LifeF_Deactivating) == 0) {
    g_hash_table_remove (poa->oid_to_obj_map, object_id);
    g_free (pobj);
  }
  else
    pobj->life_flags = ORBit_LifeF_Destroyed;

  object_id->_release = CORBA_TRUE;
  ORBit_free_T (object_id);

  ORBit_RootObject_release_T (poa);
}

static ORBit_RootObject_Interface ORBit_POAObject_if = {
  ORBIT_ROT_OAOBJECT,
  ORBit_POAObject_release_cb
  };

static struct 
ORBit_OAObject_Interface_type ORBit_POAObject_methods = {
	ORBIT_ADAPTOR_POA,
	(ORBitStateCheckFunc)ORBit_POAObject_is_active,
	(ORBitKeyGenFunc)ORBit_POAObject_object_to_objkey,
	(ORBitInvokeFunc)ORBit_POAObject_invoke,
	(ORBitReqFunc) ORBit_POAObject_handle_request
};

/*
    If USER_ID policy, {oid} must be non-NULL.
    If SYSTEM_ID policy, {oid} must ether be NULL, or must have
    been previously created by the POA. If the user passes in
    a bogus oid under SYSTEM_ID, we will assert or segfault. This
    is allowed by the CORBA spec.
*/
static ORBit_POAObject
ORBit_POA_create_object (PortableServer_POA             poa,
			 const PortableServer_ObjectId *objid,
			 CORBA_Environment             *ev)
{
	ORBit_POAObject newobj;

	newobj = g_new0 (struct ORBit_POAObject_type, 1);
	ORBit_RootObject_init ((ORBit_RootObject)newobj, &ORBit_POAObject_if);

	/* released in ORBit_POAObject_release_cb */
	newobj->poa = ORBit_RootObject_duplicate (poa);

	((ORBit_OAObject)newobj)->interface = &ORBit_POAObject_methods;

	if (poa->p_id_assignment == PortableServer_SYSTEM_ID) {
		if (objid) {
			g_assert(objid->_length == sizeof (CORBA_unsigned_long) + ORBIT_RAND_DATA_LEN);

			newobj->object_id          = PortableServer_ObjectId__alloc ();
			newobj->object_id->_length = objid->_length;
			newobj->object_id->_buffer = PortableServer_ObjectId_allocbuf (objid->_length);
			newobj->object_id->_release = CORBA_TRUE;

			memcpy (newobj->object_id->_buffer, objid->_buffer, objid->_length);
		}
		else
			newobj->object_id = ORBit_POA_new_system_objid (poa);
	}
	else {
		newobj->object_id           = PortableServer_ObjectId__alloc ();
		newobj->object_id->_length  = objid->_length;
		newobj->object_id->_buffer  = PortableServer_ObjectId_allocbuf (objid->_length);
		newobj->object_id->_release = CORBA_TRUE;

		memcpy(newobj->object_id->_buffer, objid->_buffer, objid->_length);
	}

	g_hash_table_insert(poa->oid_to_obj_map, newobj->object_id, newobj);

	return newobj;
}

/*
    Normally this is called for normal servants in RETAIN mode. 
    However, it may also be invoked on the default servant when
    it is installed. In this later case, it may be either RETAIN
    or NON_RETAIN.
*/
static void
ORBit_POA_activate_object(PortableServer_POA poa, 
			  ORBit_POAObject pobj,
			  PortableServer_ServantBase *servant, 
			  CORBA_Environment *ev) 
{
    GSList **obj_list;

    g_assert( pobj->servant == 0 );	/* must not be already active */
    g_assert( (poa->life_flags & ORBit_LifeF_DeactivateDo) == 0 );
    g_assert( pobj->use_cnt == 0 );
    /* XXX: above should be an exception? */
    {
	PortableServer_ClassInfo *ci = ORBIT_SERVANT_TO_CLASSINFO(servant);
	if ( ci->vepvmap==0 ) {
	    ci->vepvmap 
	      = g_new0(ORBit_VepvIdx, ORBit_class_assignment_counter+1);
	    ci->vepvlen = ORBit_class_assignment_counter + 1;
	    ci->init_vepvmap(ci->vepvmap);
	}
#  ifdef ORBIT_BYPASS_MAPCACHE
	pobj->vepvmap_cache = ci->vepvmap;
#  endif
    }
    pobj->servant = servant;
    obj_list = ORBIT_SERVANT_TO_POAOBJECT_LIST_ADDR(servant);
    *obj_list = g_slist_append(*obj_list,pobj);

    /* released in ORBit_POA_deactivate_object */
    ORBit_RootObject_duplicate(pobj);
}


/*
    Note that this doesn't necessarily remove the object from
    the oid_to_obj_map; it just removes knowledge of the servant.
    If object is currentin use (servicing a request), etherialization
    and memory release will occur later.
**/
static void
ORBit_POA_deactivate_object(PortableServer_POA poa, ORBit_POAObject pobj,
			     CORBA_boolean do_etherealize,
			     CORBA_boolean is_cleanup)
{
    PortableServer_ServantBase	*serv = pobj->servant;
    GSList                      **obj_list;

    if ( serv == NULL ) {
	    /* deactivation has already occured, or is in progress */
	    return;
    }
    if ( do_etherealize && (pobj->life_flags&ORBit_LifeF_DeactivateDo)==0 )
    	pobj->life_flags |= ORBit_LifeF_DoEtherealize;
    if ( is_cleanup )
    	pobj->life_flags |= ORBit_LifeF_IsCleanup;
    if ( pobj->use_cnt > 0 ) {
        pobj->life_flags |= ORBit_LifeF_DeactivateDo;
	pobj->life_flags |= ORBit_LifeF_NeedPostInvoke;
	return;
    }
    pobj->servant = NULL;
    obj_list = ORBIT_SERVANT_TO_POAOBJECT_LIST_ADDR(serv);
    *obj_list = g_slist_remove(*obj_list,pobj);

    if ( (pobj->life_flags & ORBit_LifeF_DoEtherealize)!=0 ) {
	CORBA_Environment env, *ev = &env;
	CORBA_exception_init(ev);
	++(pobj->use_cnt);	/* prevent re-activation */
        if ( poa->p_request_processing == PortableServer_USE_SERVANT_MANAGER) {
	    POA_PortableServer_ServantActivator__epv *epv;
	    POA_PortableServer_ServantActivator *sm;

	    sm = (POA_PortableServer_ServantActivator *)poa->servant_manager;
	    epv = sm->vepv->PortableServer_ServantActivator_epv;
	    (*(epv->etherealize))(sm, pobj->object_id, poa,
			    serv,
			    (pobj->life_flags & ORBit_LifeF_IsCleanup)
			      ? CORBA_TRUE : CORBA_FALSE,
			    /* remaining_activations */ CORBA_FALSE,
			    ev);
        }
        {
	    PortableServer_ServantBase__epv *epv = serv->vepv[0];
	    /* In theory, the finalize fnc should always be non-NULL;
	     * however, for backward compat. and general extended
	     * applications we dont insist on it.
	     */
	    if ( epv && epv->finalize ) {
	        (*(epv->finalize))(serv, ev);
	    }
	}
	--(pobj->use_cnt);	/* allow re-activation */
	g_assert( ev->_major == 0 );
    }

    pobj->life_flags &= ~(ORBit_LifeF_DeactivateDo
      |ORBit_LifeF_IsCleanup|ORBit_LifeF_DoEtherealize);

    ORBit_RootObject_release(pobj);
}

/* Request processing */
static PortableServer_POA
ORBit_POA_okey_to_poa (CORBA_ORB        orb,
		       ORBit_ObjectKey *objkey)
{
	ORBit_ObjectAdaptor	adaptor;
	CORBA_long              poanum;

	if (objkey->_length < sizeof (CORBA_long))
		return NULL;

	poanum = *(CORBA_long*)(objkey->_buffer);
	if (poanum < 0 || poanum >= orb->adaptors->len)
		return NULL;

	adaptor = g_ptr_array_index (orb->adaptors, poanum);

	if (objkey->_length < adaptor->adaptor_key._length)
		return NULL;

	if (memcmp (objkey->_buffer,
		    adaptor->adaptor_key._buffer, 
		    adaptor->adaptor_key._length))
		return NULL;

	return (PortableServer_POA)adaptor;
}

struct ORBit_POA_invoke_data {
	ORBitSmallSkeleton small_skel;
	gpointer           imp;
};

static void
ORBit_POAObject_invoke (ORBit_POAObject    pobj,
			gpointer           ret,
			gpointer          *args,
			CORBA_Context      ctx,
			gpointer           data,
			CORBA_Environment *ev)
{
	ORBitSmallSkeleton small_skel = ((struct ORBit_POA_invoke_data *)data)->small_skel;
	gpointer           imp = ((struct ORBit_POA_invoke_data *)data)->imp;

	small_skel (pobj->servant, ret, args, ctx, ev, imp);
}

/*
 * If invoked in the local case, recv_buffer == NULL.
 * If invoked in the remote cse, ret = args = ctx == NULL.
 */
static void
ORBit_POAObject_handle_request (ORBit_POAObject    pobj,
				CORBA_Identifier   opname,
				gpointer           ret,
				gpointer          *args,
				CORBA_Context      ctx,
				GIOPRecvBuffer    *recv_buffer,
				CORBA_Environment *ev)
{
	PortableServer_POA                   poa = pobj->poa;
	PortableServer_ServantLocator_Cookie cookie;
	PortableServer_ObjectId             *oid;
	PortableServer_ClassInfo            *klass;
	ORBit_IMethod                       *m_data;
	ORBitSkeleton                        skel = NULL;
	ORBitSmallSkeleton                   small_skel = NULL;
	ORBit_POAInvocation		     iframe;
	gpointer                             imp = NULL;

	switch (poa->poa_manager->state) {

	case PortableServer_POAManager_HOLDING:
		if (recv_buffer)
			poa->held_requests = g_slist_prepend (
				poa->held_requests, recv_buffer);
		else
			CORBA_exception_set_system (
				ev, ex_CORBA_TRANSIENT,
				CORBA_COMPLETED_NO);
		return;

	case PortableServer_POAManager_DISCARDING:
		CORBA_exception_set_system (
			ev, ex_CORBA_TRANSIENT,
			CORBA_COMPLETED_NO);
		return;

	case PortableServer_POAManager_INACTIVE:
		CORBA_exception_set_system (
			ev, ex_CORBA_OBJ_ADAPTER,
			CORBA_COMPLETED_NO);
		return;

	case PortableServer_POAManager_ACTIVE:
		break;

	default:
		g_assert_not_reached ();
		break;
	}
	
	oid = pobj->object_id;

	if (!pobj->servant) {
		switch (poa->p_request_processing) {

		case PortableServer_USE_ACTIVE_OBJECT_MAP_ONLY:
			CORBA_exception_set_system (
				ev, ex_CORBA_OBJECT_NOT_EXIST, 
				CORBA_COMPLETED_NO);
			break;

		case PortableServer_USE_DEFAULT_SERVANT:
			ORBit_POA_activate_object (
				poa, pobj, poa->default_servant, ev);
			break;

		case PortableServer_USE_SERVANT_MANAGER:
			ORBit_POA_ServantManager_use_servant (
				poa, pobj, opname,  &cookie, oid, ev);
			break;
		default:
			g_assert_not_reached();
			break;
		}
	}

	pobj->use_cnt++;
	iframe.pobj = pobj;
	iframe.prev = poa->orb->poa_current_invocations;
	poa->orb->poa_current_invocations = &iframe;
	
	if (ev->_major == CORBA_NO_EXCEPTION && !pobj->servant)
		CORBA_exception_set_system (
			ev, ex_CORBA_OBJECT_NOT_EXIST, 
			CORBA_COMPLETED_NO);

	if (ev->_major != CORBA_NO_EXCEPTION)
		goto clean_out;

	klass = ORBIT_SERVANT_TO_CLASSINFO (pobj->servant);

	if (klass->relay_call)
		skel = klass->relay_call (
			pobj->servant, recv_buffer, &imp);

	if (!skel && klass->small_relay_call)
		small_skel = klass->small_relay_call (
			pobj->servant, opname, (gpointer *)&m_data, &imp);

	if (!skel && !small_skel) {
		small_skel = get_small_skel_CORBA_Object (
			pobj->servant, opname,
			(gpointer *)&m_data, &imp);
		if (!small_skel)
			CORBA_exception_set_system (
				ev, ex_CORBA_BAD_OPERATION,
				CORBA_COMPLETED_NO);

	} else if (!imp)
		CORBA_exception_set_system (
			ev, ex_CORBA_NO_IMPLEMENT,
			CORBA_COMPLETED_NO);

	if (ev->_major != CORBA_NO_EXCEPTION)
		goto clean_out;

	if (skel)
		skel (pobj->servant, recv_buffer, ev, imp);

	else if (recv_buffer) {
		struct ORBit_POA_invoke_data invoke_data;

		invoke_data.small_skel = small_skel;
		invoke_data.imp        = imp;

		ORBit_small_invoke_adaptor (
				(ORBit_OAObject)pobj, recv_buffer, m_data, 
				(gpointer)&invoke_data, ev);
	}
	else
		small_skel (pobj->servant, ret, args, ctx, ev, imp);

	CORBA_exception_free (ev);

 clean_out:
	if (poa->p_servant_retention == PortableServer_NON_RETAIN)
		switch (poa->p_request_processing) {
		case PortableServer_USE_SERVANT_MANAGER:
			ORBit_POA_ServantManager_unuse_servant (
				poa, pobj, opname, cookie, 
				oid, pobj->servant, ev);
			break;
		case PortableServer_USE_DEFAULT_SERVANT:
			ORBit_POA_deactivate_object (poa, pobj, FALSE, FALSE);
			break;
		default:
			g_assert_not_reached ();
			break;
		}

	g_assert (poa->orb->poa_current_invocations == &iframe);
	poa->orb->poa_current_invocations = iframe.prev;
	pobj->use_cnt--;

	if (pobj->life_flags & ORBit_LifeF_NeedPostInvoke)
		ORBit_POAObject_post_invoke (pobj);             
}

static void
ORBit_POA_handle_request (PortableServer_POA          poa,
			  GIOPRecvBuffer             *recv_buffer,
			  ORBit_ObjectKey            *objkey)
{
	ORBit_POAObject         pobj;
	CORBA_Identifier        opname;
	PortableServer_ObjectId oid;
	CORBA_Environment       env;

	CORBA_exception_init(&env);

	if (!ORBit_POA_okey_to_objid (poa, objkey, &oid)) {
		CORBA_exception_set_system (&env, 
					    ex_CORBA_OBJECT_NOT_EXIST,
					    CORBA_COMPLETED_NO);
		goto send_sys_ex;
	}

	pobj = ORBit_POA_oid_to_obj (poa, &oid, FALSE, &env);
	if (!pobj)
		switch( poa->p_request_processing ) {
		case PortableServer_USE_ACTIVE_OBJECT_MAP_ONLY:
			CORBA_exception_set_system (&env, 
						    ex_CORBA_OBJECT_NOT_EXIST, 
						    CORBA_COMPLETED_NO);
			break;

		case PortableServer_USE_DEFAULT_SERVANT: /* drop through */
		case PortableServer_USE_SERVANT_MANAGER:
			pobj = ORBit_POA_create_object (poa, &oid, &env);
			break;

		default:
			g_assert_not_reached ();
			break;
		}

	g_assert ( pobj != NULL );

	opname = giop_recv_buffer_get_opname (recv_buffer);

	ORBit_POAObject_handle_request (pobj, opname, NULL, NULL, 
					NULL, recv_buffer, &env);

send_sys_ex:
	if (env._major == CORBA_SYSTEM_EXCEPTION) {
		GIOPSendBuffer *reply_buf;

		reply_buf = giop_send_buffer_use_reply ( 
					recv_buffer->connection->giop_version,
					giop_recv_buffer_get_request_id (recv_buffer),
					CORBA_SYSTEM_EXCEPTION);
		ORBit_send_system_exception (reply_buf, &env);
		giop_send_buffer_write (reply_buf, recv_buffer->connection);
		giop_send_buffer_unuse (reply_buf);
	}
	else
		g_assert (env._major == CORBA_NO_EXCEPTION);

	CORBA_exception_free(&env);
}

/***************************************************************************
 *
 *		POAObject invocation
 *		Code for invoking requests on POAObject and its servant.
 *
 ***************************************************************************/

static PortableServer_Servant
ORBit_POA_ServantManager_use_servant( PortableServer_POA poa,
				      ORBit_POAObject pobj,
				      CORBA_Identifier opname,
				      PortableServer_ServantLocator_Cookie *the_cookie,
				      PortableServer_ObjectId *oid,
				      CORBA_Environment *ev )
{
  PortableServer_ServantBase *retval;
  GSList                     **obj_list;

  if(poa->p_servant_retention == PortableServer_RETAIN)
    {
      POA_PortableServer_ServantActivator *sm;
      POA_PortableServer_ServantActivator__epv *epv;
		
      sm = (POA_PortableServer_ServantActivator *)poa->servant_manager;
      epv = sm->vepv->PortableServer_ServantActivator_epv;
      retval = epv->incarnate(sm, oid, poa, ev);
      if ( retval )
	{

	  /* XXX: two POAs sharing servant and having
	   *      different uniqueness policies ??
	   *  see note 11.3.5.1
	   */
	  if((poa->p_id_uniqueness==PortableServer_UNIQUE_ID) &&
             (ORBIT_SERVANT_TO_POAOBJECT_LIST(retval) != NULL))
	    {
	      CORBA_exception_set_system(ev,
					 ex_CORBA_OBJ_ADAPTER,
					 CORBA_COMPLETED_NO);
	      return NULL;
            }

	  obj_list = ORBIT_SERVANT_TO_POAOBJECT_LIST_ADDR(retval);
	  *obj_list = g_slist_append(*obj_list,pobj);

          /* released by ORBit_POA_deactivate_object */
	  ORBit_RootObject_duplicate(pobj);
	  pobj->servant = retval;
	}
    }
  else
    {
      POA_PortableServer_ServantLocator *sm;
      POA_PortableServer_ServantLocator__epv *epv;

      sm = (POA_PortableServer_ServantLocator *)poa->servant_manager;
      epv = sm->vepv->PortableServer_ServantLocator_epv;
      retval = epv->preinvoke(sm, oid, poa, opname, the_cookie, ev);

      if ( retval )
	{
	  /* XXX:  Is this right?
	   *       Is it the same as above? 
	   */
	  if((poa->p_id_uniqueness==PortableServer_UNIQUE_ID) &&
             (ORBIT_SERVANT_TO_POAOBJECT_LIST(retval) != NULL))
	    {
	      CORBA_exception_set_system(ev,
					 ex_CORBA_OBJ_ADAPTER,
					 CORBA_COMPLETED_NO);
	      return NULL;
            }

	  obj_list = ORBIT_SERVANT_TO_POAOBJECT_LIST_ADDR(retval);
	  *obj_list = g_slist_append(*obj_list,pobj);

          /* released by ORBit_POA_ServantManager_unuse_servant */
	  ORBit_RootObject_duplicate(pobj);
	  pobj->servant = retval;
	}
    }

  return retval;
}

static void
ORBit_POA_ServantManager_unuse_servant( PortableServer_POA poa,
					ORBit_POAObject pobj,
					CORBA_Identifier opname,
					PortableServer_ServantLocator_Cookie cookie,
					PortableServer_ObjectId *oid,
					PortableServer_Servant servant,
					CORBA_Environment *ev )
{
  POA_PortableServer_ServantLocator *sm;
  POA_PortableServer_ServantLocator__epv *epv;
  PortableServer_ServantBase *serv = servant;
  GSList **obj_list;

  g_assert(poa->p_servant_retention == PortableServer_NON_RETAIN);

  sm = (POA_PortableServer_ServantLocator *)poa->servant_manager;
  epv = sm->vepv->PortableServer_ServantLocator_epv;

  pobj->servant = NULL;
  obj_list = ORBIT_SERVANT_TO_POAOBJECT_LIST_ADDR(serv);	
  *obj_list = g_slist_remove(*obj_list,pobj);
  ORBit_RootObject_release(pobj);
  epv->postinvoke(sm, oid, poa, opname, cookie, servant, ev);
}

static gboolean
ORBit_POA_okey_to_objid (PortableServer_POA          poa,
			 ORBit_ObjectKey            *objkey,
			 PortableServer_ObjectId    *objid)
{
	CORBA_sequence_CORBA_octet *adaptor_key;

	adaptor_key = &((ORBit_ObjectAdaptor)poa)->adaptor_key;

	if (objkey->_length < adaptor_key->_length ||
	    memcmp (objkey->_buffer, adaptor_key->_buffer, adaptor_key->_length))
		return FALSE;

	objid->_buffer  = objkey->_buffer + adaptor_key->_length;
	objid->_length  = objkey->_length - adaptor_key->_length; 
	objid->_maximum = objid->_length;
	objid->_release = CORBA_FALSE;

	return TRUE;
}

/**
    This function is invoked from the generated stub code, after
    invoking the servant method.
**/
void
ORBit_POAObject_post_invoke(ORBit_POAObject pobj)
{
    if ( pobj->use_cnt > 0 )
    	return;
    if ( pobj->life_flags & ORBit_LifeF_DeactivateDo )
      {
	/* NOTE that the "desired" values of etherealize and cleanup
	 * are stored in pobj->life_flags and they dont need
	 * to be passed in again!
	 */
	ORBit_POA_deactivate_object(pobj->poa, pobj, /*ether*/0, /*cleanup*/0);
    	/* WATCHOUT: pobj may not exist anymore! */
    }
}

/*
 * C Language Mapping Specific Methods.
 * Section 1.26.2 (C Language Mapping Specification).
 */

CORBA_char*
PortableServer_ObjectId_to_string (PortableServer_ObjectId *id, 
				   CORBA_Environment       *ev)
{
	CORBA_char *str;

	if (memchr (id->_buffer, 0, id->_length)) {
		CORBA_exception_set_system (ev, ex_CORBA_BAD_PARAM,
					    CORBA_COMPLETED_NO);
		return NULL;
	}

	str = CORBA_string_alloc (id->_length);
	memcpy (str, id->_buffer, id->_length);
	str[id->_length] = 0;

	return str;
}

CORBA_wchar*
PortableServer_ObjectId_to_wstring (PortableServer_ObjectId *id,
				    CORBA_Environment       *ev)
{
	CORBA_wchar *retval;
	int          i;
  
	if (memchr (id->_buffer, 0, id->_length)) {
		CORBA_exception_set_system (ev, ex_CORBA_BAD_PARAM,
					    CORBA_COMPLETED_NO);
		return NULL;
	}

	retval = CORBA_wstring_alloc (id->_length);
	for (i = 0; i < id->_length; i++)
		retval[i] = id->_buffer[i];
	retval[id->_length] = 0;

	return retval;
}

PortableServer_ObjectId*
PortableServer_string_to_ObjectId (CORBA_char        *str,
				   CORBA_Environment *env)
{
	PortableServer_ObjectId tmp;

	tmp._length  = strlen (str);
	tmp._buffer  = str;
  
	return (PortableServer_ObjectId *)ORBit_sequence_CORBA_octet_dup (&tmp);
}

PortableServer_ObjectId*
PortableServer_wstring_to_ObjectId (CORBA_wchar       *str,
				    CORBA_Environment *env)
{
	PortableServer_ObjectId tmp;
	int                     i;

	for(i = 0; str[i]; i++);

	tmp._length = i*2;
	tmp._buffer = g_alloca(tmp._length);

	for(i = 0; str[i]; i++)
		tmp._buffer[i] = str[i];

	return (PortableServer_ObjectId *)ORBit_sequence_CORBA_octet_dup (&tmp);
}

/*
 * Current Operations.
 * Section 11.3.9
 */

PortableServer_POA
PortableServer_Current_get_POA (PortableServer_Current  obj,
				CORBA_Environment      *ev)
{
	ORBit_POAInvocation *iframe;

	iframe = ORBit_POACurrent_get_invocation(obj, ev);

	if (!iframe) 
		return CORBA_OBJECT_NIL;

	return (PortableServer_POA)
			CORBA_Object_duplicate (obj->orb->poa_current, ev);
}

PortableServer_ObjectId *
PortableServer_Current_get_object_id (PortableServer_Current  obj,
				      CORBA_Environment      *ev)
{
	ORBit_POAInvocation *iframe;

	iframe = ORBit_POACurrent_get_invocation(obj, ev);

	if (!iframe) 
		return NULL;

	return (PortableServer_ObjectId *)
			ORBit_sequence_CORBA_octet_dup (iframe->pobj->object_id);
}

/*
 * PortableServer::POA interface
 * Section 11.3.8
 */

PortableServer_POA
PortableServer_POA_create_POA (PortableServer_POA               poa,
			       const CORBA_char                *adapter_name,
			       const PortableServer_POAManager  a_POAManager,
			       const CORBA_PolicyList          *policies,
			       CORBA_Environment               *ev)
{
	PortableServer_POA retval;

	if (g_hash_table_lookup(poa->child_poas, adapter_name)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_AdapterAlreadyExists,
				     NULL);
		return CORBA_OBJECT_NIL;
		}

	retval = ORBit_POA_new (poa->orb, adapter_name, a_POAManager, policies, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;

	ORBit_POA_add_child (poa, retval);

	CORBA_Object_duplicate ((CORBA_Object)retval, ev);

	return retval;
}

PortableServer_POA
PortableServer_POA_find_POA (PortableServer_POA   poa,
			     const CORBA_char    *adapter_name,
			     const CORBA_boolean  activate_it,
			     CORBA_Environment   *ev)
{
	PortableServer_POA child_poa = NULL;

	if (poa->child_poas)
		child_poa = g_hash_table_lookup (poa->child_poas, adapter_name);

	if (activate_it)
		g_warning ("Don't yet know how to activate POA named \"%s\"",
			   adapter_name);

	if (child_poa)
		CORBA_Object_duplicate ((CORBA_Object)child_poa, ev);
	else
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_AdapterNonExistent,
				     NULL);	

	return child_poa;
}

void
PortableServer_POA_destroy (PortableServer_POA   poa,
			    const CORBA_boolean  etherealize_objects,
			    const CORBA_boolean  wait_for_completion,
			    CORBA_Environment   *ev)
{
	gboolean done;

	if (poa->life_flags & ORBit_LifeF_Destroyed)
		return;

	if (wait_for_completion && ORBit_POA_is_inuse (poa, CORBA_TRUE, ev)) {
		CORBA_exception_set_system (ev, ex_CORBA_BAD_INV_ORDER,
					    CORBA_COMPLETED_NO);
		return;
	}

	done = ORBit_POA_destroy (poa, etherealize_objects, ev);

	g_assert (done || !wait_for_completion);
}

CORBA_string
PortableServer_POA__get_the_name (PortableServer_POA  poa,
				  CORBA_Environment  *ev)
{
	return CORBA_string_dup (poa->name);
}

PortableServer_POA
PortableServer_POA__get_the_parent (PortableServer_POA  poa,
				    CORBA_Environment  *ev)
{
	return (PortableServer_POA)
			CORBA_Object_duplicate ((CORBA_Object)poa->parent_poa, ev);
}

PortableServer_POAList*
PortableServer_POA__get_the_children (PortableServer_POA  poa,
				      CORBA_Environment  *ev)
{
	PortableServer_POAList *retval;

	retval           = PortableServer_POAList__alloc();
	retval->_length  = g_hash_table_size (poa->child_poas);
	retval->_maximum = 0; /* FIXME: needed in ORBit_POAList__add_child */
	retval->_buffer  = CORBA_sequence_PortableServer_POA_allocbuf (retval->_length);
	retval->_release = CORBA_TRUE;

	g_hash_table_foreach (poa->child_poas, ORBit_POAList_add_child, retval);

	return retval;
}

PortableServer_POAManager
PortableServer_POA__get_the_POAManager (PortableServer_POA  poa,
					CORBA_Environment  *ev)
{
	return (PortableServer_POAManager)
			CORBA_Object_duplicate ((CORBA_Object)poa->poa_manager, ev);
}

PortableServer_AdapterActivator
PortableServer_POA__get_the_activator (PortableServer_POA  poa,
				       CORBA_Environment  *ev)
{
	return (PortableServer_AdapterActivator)
			CORBA_Object_duplicate ((CORBA_Object)poa->the_activator, ev);
}

void
PortableServer_POA__set_the_activator (PortableServer_POA                    poa,
				       const PortableServer_AdapterActivator activator,
				       CORBA_Environment * ev)
{
	if (poa->the_activator)
		ORBit_RootObject_release (poa->the_activator);

	poa->the_activator = (PortableServer_AdapterActivator)
					ORBit_RootObject_duplicate (activator);
}

PortableServer_ServantManager
PortableServer_POA_get_servant_manager (PortableServer_POA  poa,
					CORBA_Environment  *ev)
{
	return (PortableServer_ServantManager)
			CORBA_Object_duplicate ((CORBA_Object)poa->servant_manager, ev);
}

void
PortableServer_POA_set_servant_manager (PortableServer_POA                   poa,
					const PortableServer_ServantManager  imgr,
					CORBA_Environment                   *ev)
{
	if (poa->servant_manager)
		ORBit_RootObject_release (poa->servant_manager);

	poa->servant_manager = (PortableServer_ServantManager)
					ORBit_RootObject_duplicate (imgr);
}

PortableServer_Servant
PortableServer_POA_get_servant (PortableServer_POA  poa,
				CORBA_Environment  *ev)
{
	return poa->default_servant;
}

void
PortableServer_POA_set_servant (PortableServer_POA           poa,
			       const PortableServer_Servant  p_servant,
			       CORBA_Environment            *ev)
{
	poa->default_servant = p_servant;
}

PortableServer_ObjectId*
PortableServer_POA_activate_object (PortableServer_POA            poa,
				    const PortableServer_Servant  p_servant,
				    CORBA_Environment            *ev)
{
	PortableServer_ServantBase *servant = p_servant;
	ORBit_POAObject             newobj;

	if (poa->p_servant_retention != PortableServer_RETAIN || 
	    poa->p_id_assignment != PortableServer_SYSTEM_ID) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_WrongPolicy,
				     NULL);
		return NULL;
	}
	
	if (poa->p_id_uniqueness == PortableServer_UNIQUE_ID &&
	    ORBIT_SERVANT_TO_POAOBJECT_LIST (servant) != NULL) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_ServantAlreadyActive,
				     NULL);
		return NULL;
	}

	newobj = ORBit_POA_create_object (poa, NULL, ev);
	ORBit_POA_activate_object (poa, newobj, servant, ev);

	return (PortableServer_ObjectId *)
			ORBit_sequence_CORBA_octet_dup (newobj->object_id);
}

void
PortableServer_POA_activate_object_with_id (PortableServer_POA             poa,
					    const PortableServer_ObjectId *oid,
					    const PortableServer_Servant   p_servant,
					    CORBA_Environment             *ev)
{
	PortableServer_ServantBase *servant = p_servant;
	ORBit_POAObject             newobj;

	/*
	 * FIXME: If we're going to do this
	 *        we should always do it.
 	 */
	if(!poa || !oid || !p_servant) {
		CORBA_exception_set_system (ev, ex_CORBA_BAD_PARAM,
					    CORBA_COMPLETED_NO);
		return;
	}

	newobj = ORBit_POA_oid_to_obj (poa, oid, FALSE, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return;

	if (newobj && newobj->servant != NULL) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_ObjectAlreadyActive, 
				     NULL);
		return;
	}

	if (poa->p_id_uniqueness == PortableServer_UNIQUE_ID &&
	    ORBIT_SERVANT_TO_POAOBJECT_LIST (servant) != 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_ServantAlreadyActive,
				     NULL);
		return;
	}

	if (!newobj)
		newobj = ORBit_POA_create_object(poa, oid, ev);

	ORBit_POA_activate_object (poa, newobj, servant, ev);
}

void
PortableServer_POA_deactivate_object (PortableServer_POA             poa,
				      const PortableServer_ObjectId *oid,
				      CORBA_Environment             *ev)
{
	ORBit_POAObject oldobj;

	/*
	 * FIXME: If we're going to do this
	 *        we should always do it.
 	 */
	if (!poa || !oid) {
		CORBA_exception_set_system (ev, ex_CORBA_BAD_PARAM,
					    CORBA_COMPLETED_NO);
		return;
	}

	oldobj = ORBit_POA_oid_to_obj (poa, oid, TRUE, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return;

	ORBit_POA_deactivate_object (poa, oldobj, CORBA_TRUE, CORBA_FALSE);
}

CORBA_Object
PortableServer_POA_create_reference (PortableServer_POA        poa,
				     const CORBA_RepositoryId  intf,
				     CORBA_Environment        *ev)
{
	ORBit_POAObject pobj;

	if (poa->p_id_assignment != PortableServer_SYSTEM_ID) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_WrongPolicy, NULL);
		return CORBA_OBJECT_NIL;
	}

	pobj = ORBit_POA_create_object (poa, NULL, ev);

	return ORBit_POA_obj_to_ref (poa, pobj, intf, ev);
}

CORBA_Object
PortableServer_POA_create_reference_with_id (PortableServer_POA             poa,
					     const PortableServer_ObjectId *oid,
					     const CORBA_RepositoryId       intf,
					     CORBA_Environment             *ev)
{
	ORBit_POAObject	pobj;

	pobj = ORBit_POA_oid_to_obj (poa, oid, FALSE, NULL);
	if (!pobj)
		pobj = ORBit_POA_create_object(poa, oid, ev);

	return ORBit_POA_obj_to_ref(poa, pobj, intf, ev);
}

PortableServer_ObjectId*
PortableServer_POA_servant_to_id (PortableServer_POA            poa,
				  const PortableServer_Servant  p_servant,
				  CORBA_Environment            *ev)
{
	PortableServer_ServantBase *servant = p_servant;
	ORBit_POAObject             pobj = ORBIT_SERVANT_TO_FIRST_POAOBJECT (servant);
	gboolean                    defserv, retain, implicit, unique;

	g_return_val_if_fail (p_servant, NULL);

	defserv   = (poa->p_request_processing  == PortableServer_USE_DEFAULT_SERVANT);
	retain    = (poa->p_servant_retention   == PortableServer_RETAIN);
	implicit  = (poa->p_implicit_activation == PortableServer_IMPLICIT_ACTIVATION);
	unique    = (poa->p_id_uniqueness       == PortableServer_UNIQUE_ID);

	if (!(defserv || (retain && (unique || implicit)))) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_WrongPolicy,
				     NULL);
		return NULL;
	}

	if (retain && unique && pobj && pobj->servant == p_servant)
		return ORBit_sequence_CORBA_octet_dup (pobj->object_id);

	else if (retain && implicit && (!unique || !pobj)) {
		pobj = ORBit_POA_create_object (poa, NULL, ev);
		ORBit_POA_activate_object (poa, pobj, servant, ev);

		return ORBit_sequence_CORBA_octet_dup (pobj->object_id);
	}
	else {
		/*
		 * FIXME:
		 * This handles case 3 of the spec; but is broader:
		 * it matches invokations on any type of servant, not
		 * just the default servant.
		 * The stricter form could be implemented, 
		 * but it would only add more code...
		 */
		ORBit_POAInvocation	*irec = poa->orb->poa_current_invocations;

		for ( ; irec; irec = irec->prev)
			if (irec->pobj->servant == p_servant)
				return ORBit_sequence_CORBA_octet_dup (irec->pobj->object_id);
	}

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_PortableServer_POA_ServantNotActive,
			     NULL);

	return NULL;
}

CORBA_Object
PortableServer_POA_servant_to_reference (PortableServer_POA            poa,
					 const PortableServer_Servant  p_servant,
					 CORBA_Environment            *ev)
{
	PortableServer_ServantBase *servant = p_servant;
	ORBit_POAObject             pobj = ORBIT_SERVANT_TO_FIRST_POAOBJECT (servant);
	gboolean                    retain, implicit, unique;

	retain   = (poa->p_servant_retention   == PortableServer_RETAIN);
	implicit = (poa->p_implicit_activation == PortableServer_IMPLICIT_ACTIVATION);
	unique   = (poa->p_id_uniqueness       == PortableServer_UNIQUE_ID);

	if (!(retain && (unique || implicit))) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_WrongPolicy,
				     NULL);
		return CORBA_OBJECT_NIL;
	}

	if (retain && unique && pobj)
		if (((ORBit_OAObject)pobj)->objref)
			return ((ORBit_OAObject)pobj)->objref;
		else
			return ORBit_POA_obj_to_ref (poa, pobj, NULL, ev);

	else if (retain && implicit && (!unique || !pobj)) {
		pobj = ORBit_POA_create_object (poa, NULL, ev);
		ORBit_POA_activate_object (poa, pobj, servant, ev);

		return ORBit_POA_obj_to_ref (poa, pobj, NULL, ev);
	}
	else {
		/*
		 * FIXME:
		 * This case deals with "invoked in the context of
		 * executing a request." Note that there are no policy
		 * restrictions for this case. We must do a forward search
		 * looking for matching {servant}. If unique, we could 
		 * go backward from servant to pobj to use_cnt, but we
		 * dont do this since forward search is more general 
		 */
		ORBit_POAInvocation	*irec = poa->orb->poa_current_invocations;
		for ( ; irec; irec = irec->prev)
			if (irec->pobj->servant == p_servant)
				return ORBit_POA_obj_to_ref (poa, pobj, NULL, ev);
	}

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_PortableServer_POA_ServantNotActive,
			     NULL);

	return NULL;
}

PortableServer_Servant
PortableServer_POA_reference_to_servant (PortableServer_POA  poa,
					 const CORBA_Object  reference,
					 CORBA_Environment  *ev)
{
	if (reference == CORBA_OBJECT_NIL)
		return NULL;

	if (poa->p_request_processing != PortableServer_USE_DEFAULT_SERVANT && 
	    poa->p_servant_retention  != PortableServer_RETAIN) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_WrongPolicy,
				     NULL);
		return NULL;
	}

	/*
	 * FIXME: Is this correct
	 */
	if (poa->p_servant_retention == PortableServer_RETAIN ) {
		if (!reference->adaptor_obj) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_PortableServer_POA_WrongAdapter,
					     NULL);
			return NULL;
		}

		if (((ORBit_POAObject)reference->adaptor_obj)->servant)
			return ((ORBit_POAObject)reference->adaptor_obj)->servant;
	}

	if (poa->p_request_processing == PortableServer_USE_DEFAULT_SERVANT &&
	    poa->default_servant)
		return poa->default_servant;

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_PortableServer_POA_ObjectNotActive,
			     NULL);

	return NULL;
}

PortableServer_ObjectId*
PortableServer_POA_reference_to_id (PortableServer_POA  poa,
				    const CORBA_Object  reference,
				    CORBA_Environment  *ev)
{
	ORBit_POAObject pobj;

	if (reference == CORBA_OBJECT_NIL)
		return NULL;

	pobj = (ORBit_POAObject)reference->adaptor_obj;
	if (pobj)
		return (PortableServer_ObjectId *)
				ORBit_sequence_CORBA_octet_dup (pobj->object_id);

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_PortableServer_POA_WrongAdapter,
			     NULL);

	return NULL;
}

PortableServer_Servant
PortableServer_POA_id_to_servant (PortableServer_POA             poa,
				  const PortableServer_ObjectId *oid,
				  CORBA_Environment             *ev)
{
	if (poa->p_request_processing != PortableServer_USE_DEFAULT_SERVANT &&
	    poa->p_servant_retention  != PortableServer_RETAIN) {

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POA_WrongPolicy,
				     NULL);
		return NULL;
	}

	if (poa->p_servant_retention == PortableServer_RETAIN) {
		ORBit_POAObject pobj = ORBit_POA_oid_to_obj (poa, oid, TRUE, NULL);

		if (pobj && pobj->servant)
			return pobj->servant;
	}

	if (poa->p_request_processing == PortableServer_USE_DEFAULT_SERVANT &&
	    poa->default_servant)
		return poa->default_servant;

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_PortableServer_POA_ObjectNotActive,
			     NULL);

	return NULL;
}

CORBA_Object
PortableServer_POA_id_to_reference (PortableServer_POA             poa,
				    const PortableServer_ObjectId *oid,
				    CORBA_Environment             *ev)
{
	ORBit_POAObject pobj;

	pobj = ORBit_POA_oid_to_obj (poa, oid, TRUE, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return NULL;

	if (((ORBit_OAObject)pobj)->objref != CORBA_OBJECT_NIL)
		return ((ORBit_OAObject)pobj)->objref;

	return ORBit_POA_obj_to_ref (poa, pobj, NULL, ev);
}
