#include "config.h"
#include <orbit/orbit.h>
#include <string.h>
#include <stdio.h>
#include "corba-ops.h"
#include "orb-core-private.h"

#undef DEBUG

/*
 * HashTable of POA generated refs that have been
 * externalised and refs that we have received.
 */
static GHashTable *objrefs = NULL;

static guint
g_CORBA_Object_hash(gconstpointer key)
{
  guint retval;
  CORBA_Object _obj = (gpointer)key;

  retval = g_str_hash(_obj->type_id);
  g_slist_foreach(_obj->profile_list, IOP_profile_hash, &retval);

  return retval;
}

static gboolean
g_CORBA_Object_equal(gconstpointer a, gconstpointer b)
{
  GSList *cur1, *cur2;
  CORBA_Object _obj = (CORBA_Object)a;
  CORBA_Object other_object = (CORBA_Object)b;

  if(_obj == other_object)
    return TRUE;
  if(!(_obj && other_object))
    return FALSE;

  for(cur1 = _obj->profile_list; cur1; cur1 = cur1->next)
    {
      for(cur2 = other_object->profile_list; cur2; cur2 = cur2->next)
	{
	  if(IOP_profile_equal(_obj, other_object,
			       cur1->data, cur2->data))
	    {
#if 0
		char *a, *b;
		a = IOP_profile_dump (_obj, cur1->data);
		b = IOP_profile_dump (other_object, cur2->data);
		fprintf (stderr, "Profiles match:\n'%s':%s\n'%s':%s\n",
		_obj->type_id, a, other_object->type_id, b);
#endif
		return TRUE;
	    }
	}
    }

  return FALSE;
}

void
ORBit_register_objref( CORBA_Object obj )
{
  if( objrefs == NULL )
    objrefs = g_hash_table_new(g_CORBA_Object_hash, g_CORBA_Object_equal);

  g_assert (obj->profile_list != NULL);

  g_hash_table_insert (objrefs, obj, obj);
}

static CORBA_Object
ORBit_lookup_objref( CORBA_Object obj )
{
  if( objrefs == NULL ) {
    objrefs = g_hash_table_new(g_CORBA_Object_hash, g_CORBA_Object_equal);
    return NULL;
  }

  if (!obj->profile_list)
    return NULL;

  return g_hash_table_lookup(objrefs, obj);
}

static void
CORBA_Object_release_cb(ORBit_RootObject robj)
{
  CORBA_Object obj = (CORBA_Object) robj;

  if ( obj->profile_list != NULL )
    g_hash_table_remove (objrefs, obj);

  if (obj->connection) {
/*    g_warning("Release object '%p's connection", obj); */
    giop_connection_close (obj->connection);
    giop_connection_unref (obj->connection);
  }

  g_free (obj->type_id);

  IOP_delete_profiles (&obj->profile_list);
  IOP_delete_profiles (&obj->forward_locations);

  if ( obj->pobj != NULL )
    ORBit_RootObject_release_T (obj->pobj);

  g_free (obj);
}

static ORBit_RootObject_Interface objref_if = {
  ORBIT_ROT_OBJREF,
  CORBA_Object_release_cb
};

CORBA_Object
ORBit_objref_new(CORBA_ORB orb, const char *type_id)
{
  CORBA_Object retval;
  retval = g_new0(struct CORBA_Object_type, 1);

  ORBit_RootObject_init((ORBit_RootObject)retval, &objref_if);
  retval->type_id = g_strdup(type_id);
  retval->orb = orb;

  return retval;
}

static CORBA_Object
ORBit_objref_find(CORBA_ORB orb, const char *type_id, GSList *profiles)
{
  CORBA_Object retval = CORBA_OBJECT_NIL;
  struct CORBA_Object_type fakeme = {{0}};

  fakeme.type_id = (char *)type_id;
  fakeme.profile_list = profiles;

  O_MUTEX_LOCK(ORBit_RootObject_lifecycle_lock);
  retval = ORBit_lookup_objref( &fakeme );

  if(retval == NULL) {
    retval = ORBit_objref_new(orb, type_id);
    retval->profile_list = profiles;
    ORBit_register_objref( retval );
  }
  else
    IOP_delete_profiles (&profiles);

  retval = CORBA_Object_duplicate(retval, NULL);

  O_MUTEX_UNLOCK(ORBit_RootObject_lifecycle_lock);

  return retval;
}

void
ORBit_start_servers( CORBA_ORB orb )
{
  LINCProtocolInfo *info;

  for(info = linc_protocol_all(); info->name; info++)
    {
      GIOPServer *server;
      LINCConnectionOptions options = 0;

#ifndef ORBIT_THREADED
      options |= LINC_CONNECTION_NONBLOCKING;
#endif
      server = giop_server_new(orb->default_giop_version,
                               info->name, NULL, NULL,
                               options, orb);
      if(server)
        {
          orb->servers = g_slist_prepend(orb->servers, server);
          if(!(info->flags & LINC_PROTOCOL_SECURE))
            {
              server = giop_server_new(orb->default_giop_version, info->name,
                                       NULL, NULL, LINC_CONNECTION_SSL, orb);
              if(server)
                orb->servers = g_slist_prepend(orb->servers, server);
            }
#ifdef DEBUG
          fprintf (stderr, "ORB created giop server '%s'\n", info->name);
#endif
        }
#ifdef DEBUG
      else
        fprintf (stderr, "ORB failed to create giop server '%s'\n", info->name);
#endif
    }
}

static gboolean
ORBit_try_connection(CORBA_Object obj)
{
  while(obj->connection)
    switch(LINC_CONNECTION(obj->connection)->status)
      {
      case LINC_CONNECTING:
	g_main_iteration(TRUE);
	break;
      case LINC_CONNECTED:
	return TRUE;
	break;
      case LINC_DISCONNECTED:
	giop_connection_unref(obj->connection); obj->connection = NULL;
	return FALSE;
	break;
      }

  return FALSE;
}

GIOPConnection *
_ORBit_object_get_connection(CORBA_Object obj)
{
  GSList *plist, *cur;
  char tbuf[20];

  /* Information we have to come up with */
  IOP_ObjectKey_info *oki;
  char *proto = NULL, *host, *service;
  gboolean is_ssl = FALSE;
  GIOPVersion iiop_version = GIOP_1_2;

  plist = obj->forward_locations;
  if(!plist)
    plist = obj->profile_list;

  if(ORBit_try_connection(obj))
    return obj->connection;
  
  g_assert (obj->connection == NULL);

  for(cur = plist; cur; cur = cur->next)
    {
      gpointer *pinfo = cur->data;
      if(IOP_profile_get_info(obj, pinfo, &iiop_version, &proto,
			      &host, &service, &is_ssl, &oki, tbuf))
	{
	  obj->connection =
	    giop_connection_initiate(proto, host, service,
				     is_ssl?LINC_CONNECTION_SSL:0, iiop_version);

	  if(ORBit_try_connection(obj))
	    {
	      obj->oki = oki;
	      obj->connection->orb_data = obj->orb;
	      return obj->connection;
	    }
	}
    }

  return NULL;
}

GIOPConnection *
ORBit_handle_location_forward(GIOPRecvBuffer *buf,
			      CORBA_Object obj)
{
  GIOPConnection *retval = NULL;
  GSList *profiles;

  if(ORBit_demarshal_IOR(obj->orb, buf, NULL, &profiles))
    goto out;

  IOP_delete_profiles(&obj->forward_locations);

  obj->forward_locations = profiles;

  retval = _ORBit_object_get_connection(obj);

 out:
  giop_recv_buffer_unuse(buf);

  return retval;
}

CORBA_InterfaceDef
CORBA_Object_get_interface(CORBA_Object _obj,
			   CORBA_Environment * ev)
{
  /* XXX fixme */
  return CORBA_OBJECT_NIL;
}

CORBA_boolean
CORBA_Object_is_nil(CORBA_Object _obj,
		    CORBA_Environment * ev)
{
  return _obj?CORBA_FALSE:CORBA_TRUE;
}

CORBA_Object
CORBA_Object_duplicate(CORBA_Object _obj,
		       CORBA_Environment * ev)
{
  return ORBit_RootObject_duplicate(_obj);
}

void
CORBA_Object_release(CORBA_Object _obj, CORBA_Environment * ev)
{
  ORBit_RootObject_release(_obj);
}

CORBA_boolean
CORBA_Object_non_existent(CORBA_Object _obj,
			  CORBA_Environment * ev)
{
  if(_obj == CORBA_OBJECT_NIL)
    return TRUE;
  return ORBit_object_get_connection(_obj)?CORBA_FALSE:CORBA_TRUE;
}

CORBA_boolean
CORBA_Object_is_equivalent(CORBA_Object _obj,
			   CORBA_Object other_object,
			   CORBA_Environment * ev)
{
  return g_CORBA_Object_equal(_obj, other_object);
}

CORBA_unsigned_long
CORBA_Object_hash(CORBA_Object _obj,
		  const CORBA_unsigned_long maximum,
		  CORBA_Environment * ev)
{
  CORBA_unsigned_long retval;

  retval = g_CORBA_Object_hash(_obj);

  return maximum?(retval%maximum):retval;
}

void
CORBA_Object_create_request(CORBA_Object _obj,
			    const CORBA_Context ctx,
			    const CORBA_Identifier operation,
			    const CORBA_NVList arg_list,
			    CORBA_NamedValue * result,
			    CORBA_Request * request,
			    const CORBA_Flags req_flag,
			    CORBA_Environment * ev)
{
  CORBA_exception_set_system(ev, ex_CORBA_NO_IMPLEMENT, CORBA_COMPLETED_NO);
}

CORBA_Policy
CORBA_Object_get_policy(CORBA_Object _obj,
			const CORBA_PolicyType policy_type,
			CORBA_Environment * ev)
{
  CORBA_exception_set_system(ev, ex_CORBA_NO_IMPLEMENT, CORBA_COMPLETED_NO);
  return CORBA_OBJECT_NIL;
}

CORBA_DomainManagersList *
CORBA_Object_get_domain_managers(CORBA_Object _obj,
				 CORBA_Environment * ev)
{
  CORBA_exception_set_system(ev, ex_CORBA_NO_IMPLEMENT, CORBA_COMPLETED_NO);
  return NULL;
}

CORBA_Object
CORBA_Object_set_policy_overrides(CORBA_Object _obj,
				  const CORBA_PolicyList *policies,
				  const CORBA_SetOverrideType set_add,
				  CORBA_Environment * ev)
{
  CORBA_exception_set_system(ev, ex_CORBA_NO_IMPLEMENT, CORBA_COMPLETED_NO);
  return CORBA_OBJECT_NIL;
}

void
ORBit_marshal_object(GIOPSendBuffer *buf, CORBA_Object obj)
{
  CORBA_unsigned_long type_len, num_profiles;
  GSList *cur;
  char *typeid;

  giop_send_buffer_align(buf, 4);
  if(obj)
    typeid = obj->type_id;
  else
    typeid = "";
  type_len = strlen(typeid) + 1;
  giop_send_buffer_append_indirect(buf, &type_len, 4);
  giop_send_buffer_append(buf, typeid, type_len);
  if(obj)
    {
      if ( obj->profile_list == NULL )
	  IOP_generate_profiles( obj );
      num_profiles = g_slist_length(obj->profile_list);
      g_assert (num_profiles > 0);
    }
  else
    num_profiles = 0;
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append_indirect(buf, &num_profiles, 4);

#ifdef DEBUG
  fprintf (stderr, "Marshal object '%p'\n", obj);
#endif
  if(obj)
    for(cur = obj->profile_list; cur; cur = cur->next)
      {
#ifdef DEBUG
        fprintf (stderr, "%s\n", IOP_profile_dump (obj, cur->data));
#endif
	IOP_profile_marshal(obj, buf, cur->data);
      }
}

gboolean
ORBit_demarshal_object(CORBA_Object *obj, GIOPRecvBuffer *buf,
		       CORBA_ORB orb)
{
  GSList *profiles;
  char *type_id;

  if(ORBit_demarshal_IOR(orb, buf, &type_id, &profiles))
    return TRUE;

  if(type_id)
    *obj = ORBit_objref_find(orb, type_id, profiles);
  else
    *obj = CORBA_OBJECT_NIL;

  return FALSE;
}

gpointer
CORBA_Object__freekids(gpointer mem, gpointer dat)
{
  return mem + sizeof(CORBA_Object);
}

/* 
 * Arguments' Definitions.
 */

static ORBit_IArg CORBA_Object_is_a__arginfo[] = {
   {
   TC_CORBA_string, ORBit_I_ARG_IN, "logical_type_id"
   },
   {NULL, 0, NULL}
};

static ORBit_IArg CORBA_Object_is_equivalent__arginfo[] = {
   {
    TC_CORBA_Object, ORBit_I_ARG_IN, "other_object"
   },
   {NULL, 0, NULL}
};

static ORBit_IArg CORBA_Object_hash__arginfo[] = {
   {
    TC_CORBA_unsigned_long, 
    ORBit_I_ARG_IN | ORBit_I_COMMON_FIXED_SIZE,
    "maximum"
   },
   {NULL, 0, NULL}
};

static ORBit_IArg CORBA_Object_create_request__arginfo[] = {
   {
    TC_CORBA_Context, ORBit_I_ARG_IN, "ctx"
   },
   {
    TC_CORBA_Identifier, ORBit_I_ARG_IN, "operation"
   },
   {
    TC_CORBA_NVList, ORBit_I_ARG_IN, "arg_list"
   },
   {
    TC_CORBA_NamedValue, ORBit_I_ARG_INOUT, "result"
   },
   {
    TC_CORBA_Request, ORBit_I_ARG_OUT, "request"
   },
   {
    TC_CORBA_Flags, 
    ORBit_I_ARG_IN | ORBit_I_COMMON_FIXED_SIZE, 
    "req_flag"
   },
   {NULL, 0, NULL}
};

static ORBit_IArg CORBA_Object_get_policy__arginfo[] = {
   {
    TC_CORBA_PolicyType,
    ORBit_I_ARG_IN | ORBit_I_COMMON_FIXED_SIZE,
    "policy_type"
   },
   {NULL, 0, NULL}
};

static ORBit_IArg CORBA_Object_set_policy_overrides__arginfo[] = {
   {
    TC_CORBA_PolicyList, ORBit_I_ARG_IN, "policies"
   },
   {
    TC_CORBA_SetOverrideType, 
    ORBit_I_ARG_IN | ORBit_I_COMMON_FIXED_SIZE,
    "set_add"
   },
   {NULL, 0, NULL}
};

/* 
 * Methods' Definitions.
 */

ORBit_IMethod CORBA_Object__imethods[] = {
   {
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_CORBA_InterfaceDef, "get_interface", 14, 0
   },
   {
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_CORBA_boolean, "is_nil", 7, 
    0 | ORBit_I_COMMON_FIXED_SIZE
   },
   {
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_CORBA_Object, "duplicate", 10, 0
   },
   {
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    CORBA_OBJECT_NIL, "release", 8, 0
   },
   {
    {1, 1, CORBA_Object_is_a__arginfo, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_CORBA_boolean, "is_a", 5,
    0 | ORBit_I_COMMON_FIXED_SIZE
   },
   {
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_CORBA_boolean, "non_existent", 13,
    0 | ORBit_I_COMMON_FIXED_SIZE
   },
   {
    {1, 1, CORBA_Object_is_equivalent__arginfo, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_CORBA_boolean, "is_equivalent", 14,
    0 | ORBit_I_COMMON_FIXED_SIZE
   },
   {
    {1, 1, CORBA_Object_hash__arginfo, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_CORBA_unsigned_long, "hash", 5,
    0 | ORBit_I_COMMON_FIXED_SIZE
   },
   {
    {6, 6, CORBA_Object_create_request__arginfo, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    CORBA_OBJECT_NIL, "create_request", 15, 0
   },
   {
    {1, 1, CORBA_Object_get_policy__arginfo, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_CORBA_Policy, "get_policy", 11, 0
   },
   {
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_CORBA_DomainManagersList, "get_domain_managers", 20, 0
   },
   {
    {2, 2, CORBA_Object_set_policy_overrides__arginfo, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_CORBA_Object, "set_policy_overrides", 21, 0
   }
};

/* 
 * Interface Definition.
 */

ORBit_IInterface CORBA_Object__itype = {
  "CORBA_Object", 
  {12, 12, CORBA_Object__imethods, FALSE},
  {0, 0, NULL, FALSE}
};
