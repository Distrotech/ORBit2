#include "config.h"
#include <orbit/orbit.h>
#include <string.h>
#include <stdio.h>
#include "../poa/orbit-poa-export.h"
#include "corba-ops.h"

#undef DEBUG

static gboolean g_CORBA_Object_equal(gconstpointer a, gconstpointer b);
static guint    g_CORBA_Object_hash(gconstpointer key);
static void     ORBit_delete_profiles(GSList **profiles);
static void     ORBit_profile_free(IOP_Profile_info *p);
static gboolean ORBit_demarshal_IOR(CORBA_ORB orb, GIOPRecvBuffer *buf,
		                    char **ret_type_id, GSList **ret_profiles);

static GHashTable *objrefs = NULL;

static void IOP_component_free(IOP_Component_info *c)
{
  switch(c->component_type)
    {
    case IOP_TAG_GENERIC_SSL_SEC_TRANS:
      g_free(((IOP_TAG_GENERIC_SSL_SEC_TRANS_info*)c)->service);
      break;
    case IOP_TAG_COMPLETE_OBJECT_KEY:
      g_free(((IOP_TAG_COMPLETE_OBJECT_KEY_info*)c)->oki);
      break;
    case IOP_TAG_SSL_SEC_TRANS:
      break;
    default:
      g_free(((IOP_UnknownProfile_info*)c)->data._buffer);
      break;
    }
  g_free(c);
}

static void IOP_components_free(GSList *components)
{
  g_slist_foreach(components, (GFunc)IOP_component_free, NULL);
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
  
  ORBit_delete_profiles (&obj->forward_locations);

  /* obj->pobj != NULL, then obj is a POA generated reference */
  if ( obj->pobj != NULL ) {
    ORBit_RootObject_release_T (obj->pobj);
    obj->profile_list = NULL;
  } else 
    ORBit_delete_profiles (&obj->profile_list);

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
  if(!objrefs)
    objrefs = g_hash_table_new(g_CORBA_Object_hash, g_CORBA_Object_equal);
  retval = g_hash_table_lookup(objrefs, &fakeme);

  if(retval == NULL) {
    retval = ORBit_objref_new(orb, type_id);
    retval->profile_list = profiles;
    g_hash_table_insert (objrefs, retval, retval);
  }
  else
    ORBit_delete_profiles (&profiles);

  retval = CORBA_Object_duplicate(retval, NULL);

  O_MUTEX_UNLOCK(ORBit_RootObject_lifecycle_lock);

  return retval;
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

static IOP_Profile_info *
IOP_profile_find(GSList *list, IOP_ProfileId type, GSList **pos)
{
  for(; list; list = list->next)
    {
      IOP_Profile_info *pi = list->data;
      if(pi->profile_type == type)
	{
	  if(pos)
	    *pos = list;
	  return pi;
	}
    }

  return NULL;
}

static IOP_Component_info *
IOP_component_find(GSList *list, IOP_ComponentId type, GSList **pos)
{
  for(; list; list = list->next)
    {
      IOP_Component_info *pi = list->data;
      if(pi->component_type == type)
	return pi;
    }

  return NULL;
}

static gchar *
IOP_ObjectKey_dump (IOP_ObjectKey_info *oki)
{
	int i;
	GString *str = g_string_sized_new (oki->object_key._length * 2 + 4);

	for (i = 0; i < oki->object_key._length; i++)
		g_string_printfa (str, "%2x", oki->object_key._buffer [i]);

	return g_string_free (str, FALSE);
}

static G_GNUC_UNUSED gchar * 
IOP_Profile_dump(CORBA_Object obj, gpointer p)
{
	IOP_ProfileId t;
	char         *key = NULL;
	GString      *str = g_string_sized_new (64);

	t = ((IOP_Profile_info *)p)->profile_type;

	switch (t) {
	case IOP_TAG_INTERNET_IOP: {
		IOP_TAG_INTERNET_IOP_info *iiop = p;
		IOP_ObjectKey_info        *oki;

		oki = iiop->oki != NULL ? iiop->oki : obj->oki;
		
		key = IOP_ObjectKey_dump (oki);
		g_string_printf (str, "P-IIOP %s:0x%x '%s'",
				 iiop->host, iiop->port, key);
		break;
	}
	
	case IOP_TAG_GENERIC_IOP: {
		IOP_TAG_GENERIC_IOP_info *giop = p;
		
		g_string_printf (str, "P-GIOP %s:%s:%s",
				 giop->proto, giop->service,
				 giop->host);
		break;
	}
	
	case IOP_TAG_ORBIT_SPECIFIC: {
		IOP_TAG_ORBIT_SPECIFIC_info *os = p;
		IOP_ObjectKey_info          *oki;
		
		oki = os->oki != NULL ? os->oki : obj->oki;

		key = IOP_ObjectKey_dump (os->oki);
		g_string_printf (str, "P-OS %s:0x%x '%s'",
				 os->unix_sock_path, os->ipv6_port,
				 key);
		break;
	}
	case IOP_TAG_MULTIPLE_COMPONENTS:
	default:
		g_string_printf (str, "P-<None>");
		break;
	}

	g_free (key);
	
	return g_string_free (str, FALSE);
}

static gboolean
IOP_profile_get_info(CORBA_Object obj, IOP_Profile_info *pi,
		     GIOPVersion *iiop_version, char **proto,
		     char **host, char **service, gboolean *ssl,
		     IOP_ObjectKey_info **oki, char *tmpbuf)
{
  IOP_TAG_ORBIT_SPECIFIC_info *osi;
  IOP_TAG_INTERNET_IOP_info *iiop;
  IOP_TAG_GENERIC_IOP_info *giop;

  *ssl = FALSE;

#ifdef DEBUG
  {
    char *str;
    fprintf (stderr, "profile for object '%p' '%s'\n",
	     obj, (str = IOP_Profile_dump (obj, pi)));
    g_free (str);
  }
#endif

  switch(pi->profile_type)
    {
    case IOP_TAG_INTERNET_IOP:
      iiop = (IOP_TAG_INTERNET_IOP_info *)pi;
      *iiop_version = iiop->iiop_version;
      *proto = "IPv4";
      *host = iiop->host;
      *service = tmpbuf;
      g_snprintf(tmpbuf, 8, "%d", iiop->port);
      *oki = iiop->oki;
#ifdef LINC_SSL_SUPPORT
      {
	IOP_TAG_SSL_SEC_TRANS_info *ssli;
	ssli = (IOP_TAG_SSL_SEC_TRANS_info *)
	  IOP_component_find(iiop->components, IOP_TAG_SSL_SEC_TRANS, NULL);
	if(ssli)
	  {
	    g_snprintf(tmpbuf, 8, "%d", ssli->port);
	    *ssl = TRUE;
	  }
      }
#endif
      return TRUE;
      break;
    case IOP_TAG_GENERIC_IOP:
      giop = (IOP_TAG_GENERIC_IOP_info *)pi;
      *iiop_version = giop->iiop_version;
      *proto = giop->proto;
      *host = giop->host;
      *service = giop->service;
#ifdef LINC_SSL_SUPPORT
      {
	IOP_TAG_GENERIC_SSL_SEC_TRANS_info *ssli;
	ssli = (IOP_TAG_GENERIC_SSL_SEC_TRANS_info *)
	  IOP_component_find(giop->components, IOP_TAG_GENERIC_SSL_SEC_TRANS,
			     NULL);
	if(ssli)
	  {
	    *service = ssli->service;
	    *ssl = TRUE;
	  }
      }
#endif
      {
	IOP_TAG_MULTIPLE_COMPONENTS_info *mci;
	mci = (IOP_TAG_MULTIPLE_COMPONENTS_info *)
	  IOP_profile_find(obj->profile_list, IOP_TAG_MULTIPLE_COMPONENTS, NULL);
	*oki = NULL;
	if(mci)
	  {
	    IOP_TAG_COMPLETE_OBJECT_KEY_info *coki;
	    coki = (IOP_TAG_COMPLETE_OBJECT_KEY_info *)
	      IOP_component_find(mci->components, IOP_TAG_COMPLETE_OBJECT_KEY,
				 NULL);
	    if(coki)
	      *oki = coki->oki;
	  }
	return *oki?TRUE:FALSE;
      }
      break;
    case IOP_TAG_ORBIT_SPECIFIC:
      /* Due to (a) my brain deadness in putting multiple protocols in
	 one profile in ORBit 0.[3-5].x (b) the inability of this current
	 code to support multiple protocols per profile,
	 this only works to pull out the UNIX socket path OR IPv6. It's not
	 like anyone used IPv6 with old ORBit, anyways - most people
	 just want UNIX sockets. */
      osi = (IOP_TAG_ORBIT_SPECIFIC_info *)pi;
      if(osi->unix_sock_path && *osi->unix_sock_path)
	{
	  *iiop_version = GIOP_1_0;
	  *proto = "UNIX";
	  *host = "";
	  *service = osi->unix_sock_path;
	  *oki = osi->oki;
	  return TRUE;
	}
      break;
    default:
      break;
    }

  return FALSE;
}

static void
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
      IOP_Profile_info *pi = cur->data;
      if(IOP_profile_get_info(obj, pi, &iiop_version, &proto,
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

  ORBit_delete_profiles(&obj->forward_locations);

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

static inline gboolean
IOP_ObjectKey_equal(IOP_ObjectKey_info *a, IOP_ObjectKey_info *b)
{
	if (a->object_key._length !=
	    b->object_key._length)
		return FALSE;
	if (memcmp (a->object_key._buffer,
		    b->object_key._buffer,
		    a->object_key._length))
		return FALSE;
	return TRUE;
}

static gboolean
IOP_Profile_equal(CORBA_Object obj1, CORBA_Object obj2,
		  gpointer d1, gpointer d2,
		  IOP_TAG_MULTIPLE_COMPONENTS_info *mci1,
		  IOP_TAG_MULTIPLE_COMPONENTS_info *mci2)
{
	IOP_ProfileId t1, t2;

	t1 = ((IOP_Profile_info *)d1)->profile_type;
	t2 = ((IOP_Profile_info *)d2)->profile_type;

	if(t1 != t2)
		return FALSE;

	switch (t1) {
	case IOP_TAG_INTERNET_IOP: {
		IOP_TAG_INTERNET_IOP_info *iiop1 = d1;
		IOP_TAG_INTERNET_IOP_info *iiop2 = d2;
		IOP_ObjectKey_info        *oki1, *oki2;

		if (iiop1->port != iiop2->port)
			return FALSE;

		oki1 = iiop1->oki != NULL ? iiop1->oki : obj1->oki;
		oki2 = iiop2->oki != NULL ? iiop2->oki : obj2->oki;

		if (!IOP_ObjectKey_equal (oki1, oki2))
			return FALSE;
		if (strcmp (iiop1->host, iiop2->host))
			return FALSE;
		break;
	}

	case IOP_TAG_GENERIC_IOP: {
		IOP_TAG_GENERIC_IOP_info *giop1 = d1;
		IOP_TAG_GENERIC_IOP_info *giop2 = d2;

		if (!(mci1 || mci2))
			return FALSE;

		if (strcmp (giop1->service, giop2->service))
			return FALSE;
		if (strcmp (giop1->host, giop2->host))
			return FALSE;
		if (strcmp (giop1->proto, giop2->proto))
			return FALSE;

		{ /* Oh, the ugliness */
			IOP_TAG_COMPLETE_OBJECT_KEY_info *c1, *c2;
			IOP_ObjectKey_info               *oki1, *oki2;

			c1 = (IOP_TAG_COMPLETE_OBJECT_KEY_info *)
				IOP_component_find(mci1->components,
						   IOP_TAG_COMPLETE_OBJECT_KEY,
						   NULL);
			c2 = (IOP_TAG_COMPLETE_OBJECT_KEY_info *)
				IOP_component_find(mci2->components,
						   IOP_TAG_COMPLETE_OBJECT_KEY,
						   NULL);

			if (!(c1 || c2))
				return FALSE;

			oki1 = c1->oki != NULL ? c1->oki : obj1->oki;
			oki2 = c2->oki != NULL ? c2->oki : obj2->oki;

			if (!IOP_ObjectKey_equal (oki1, oki2))
				return FALSE;
		}
		break;
	}

	case IOP_TAG_ORBIT_SPECIFIC: {
		IOP_TAG_ORBIT_SPECIFIC_info *os1 = d1;
		IOP_TAG_ORBIT_SPECIFIC_info *os2 = d2;
		IOP_ObjectKey_info               *oki1, *oki2;

		if (os1->ipv6_port != os2->ipv6_port)
			return FALSE;

		oki1 = os1->oki != NULL ? os1->oki : obj1->oki;
		oki2 = os2->oki != NULL ? os2->oki : obj2->oki;

		if (!IOP_ObjectKey_equal (oki1, oki2))
			return FALSE;
		if (strcmp (os1->unix_sock_path, os2->unix_sock_path))
			return FALSE;
		break;
	}
	case IOP_TAG_MULTIPLE_COMPONENTS: {
		static int warned = 0;
		if (!(warned++)) /* FIXME: */
			g_warning ("IOP_Profile_equal: no multiple "
				   "components support");
		return FALSE;
		break;
	}
	default:
		g_warning ("No IOP_Profile_match for component");
		return FALSE;
		break;
	}

	return TRUE;
}

static IOP_TAG_MULTIPLE_COMPONENTS_info *
get_mci (GSList *p)
{
  for (; p; p = p->next) {
    if (((IOP_Profile_info *)p->data)->profile_type ==
	IOP_TAG_MULTIPLE_COMPONENTS)
	    return p->data;
  }
  return NULL;
}

static gboolean
g_CORBA_Object_equal(gconstpointer a, gconstpointer b)
{
  GSList *cur1, *cur2;
  CORBA_Object _obj = (CORBA_Object)a;
  CORBA_Object other_object = (CORBA_Object)b;
  IOP_TAG_MULTIPLE_COMPONENTS_info *mci1, *mci2;

  if(_obj == other_object)
    return TRUE;
  if(!(_obj && other_object))
    return FALSE;

  mci1 = get_mci (_obj->profile_list);
  mci2 = get_mci (other_object->profile_list);

  for(cur1 = _obj->profile_list; cur1; cur1 = cur1->next)
    {
      for(cur2 = other_object->profile_list; cur2; cur2 = cur2->next)
	{
           if(IOP_Profile_equal(_obj, other_object,
				cur1->data, cur2->data,
				mci1, mci2))
	     {
/*                char *a, *b;
		a = IOP_Profile_dump (_obj, cur1->data);
		b = IOP_Profile_dump (other_object, cur2->data);
		fprintf (stderr, "Profiles match:\n'%s':%s\n'%s':%s\n",
		_obj->type_id, a, other_object->type_id, b);*/
		return TRUE;
	     }
	}
    }
  
  return FALSE;
}

CORBA_boolean
CORBA_Object_is_equivalent(CORBA_Object _obj,
			   CORBA_Object other_object,
			   CORBA_Environment * ev)
{
  return g_CORBA_Object_equal(_obj, other_object);
}

static guint
mem_hash (gconstpointer key, gulong len)
{
  const char *p, *pend;
  guint h = 0;

  for(p = key, pend = p + len; p < pend; p++)
    h = (h << 5) - h + *p;

  return h;
}

static void
profile_hash(gpointer item, gpointer data)
{
  IOP_Profile_info *p = item;
  guint *h = data;
  IOP_TAG_INTERNET_IOP_info *iiop;
  IOP_TAG_GENERIC_IOP_info *giop;
  IOP_TAG_ORBIT_SPECIFIC_info *osi;
  IOP_TAG_MULTIPLE_COMPONENTS_info *mci;
  IOP_UnknownProfile_info *upi;

  *h ^= p->profile_type;
  switch(p->profile_type)
    {
    case IOP_TAG_ORBIT_SPECIFIC:
      osi = item;
      *h ^= g_str_hash(osi->unix_sock_path);
      break;
    case IOP_TAG_INTERNET_IOP:
      iiop = item;
      *h ^= g_str_hash(iiop->host);
      *h ^= iiop->port;
      break;
    case IOP_TAG_GENERIC_IOP:
      giop = item;
      *h ^= g_str_hash(giop->proto);
      *h ^= g_str_hash(giop->host);
      *h ^= g_str_hash(giop->service);
      break;
    case IOP_TAG_MULTIPLE_COMPONENTS:
      mci = item;
      *h ^= g_slist_length(mci->components);
      break;
    default:
      upi = item;
      *h ^= mem_hash(upi->data._buffer, upi->data._length);
      break;
    }
}

static guint
g_CORBA_Object_hash(gconstpointer key)
{
  guint retval;
  CORBA_Object _obj = (gpointer)key;

  retval = g_str_hash(_obj->type_id);
  g_slist_foreach(_obj->profile_list, profile_hash, &retval);
  return retval;
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

static void
ORBit_delete_profiles (GSList **profiles)
{
  if (profiles && *profiles) {
    g_slist_foreach (*profiles, (GFunc) ORBit_profile_free, NULL);
    g_slist_free (*profiles);
    *profiles = NULL;
  }
}

static void
ORBit_generate_profiles( CORBA_Object obj )
  {
  IOP_TAG_MULTIPLE_COMPONENTS_info *mci = NULL;
  IOP_TAG_ORBIT_SPECIFIC_info      *osi = NULL;
  IOP_TAG_INTERNET_IOP_info        *iiop = NULL;
  gboolean                         need_objkey_component;
  GSList                           *ltmp;

  static GSList                    *profiles = NULL;

  g_assert( obj && (obj->profile_list == NULL) );

  /*
   * no need to have any listening sockets until now.
   * if the ORB has been shutdown and restarted,
   * the profiles must be regenerated.
   */
  if ( obj->orb->servers == NULL ) {
    ORBit_start_servers( obj->orb );
    if ( profiles != NULL )
      ORBit_delete_profiles( &profiles );
  }

  /* only carry around one copy of the object key. */
  if ( obj->oki == NULL )
    obj->oki = ORBit_POA_object_to_okey( obj->pobj );

  /* share the profiles between everyone. */
  if ( profiles != NULL ) {
    obj->profile_list = profiles;
    return;
  }

  need_objkey_component = FALSE;

  for(ltmp = obj->orb->servers ; ltmp != NULL ; ltmp = ltmp->next)
    {
      LINCServer *serv = ltmp->data;
      gboolean   ipv4, ipv6, uds, ssl;

      ipv4 = (strcmp(serv->proto->name, "IPv4") == 0)     ? TRUE : FALSE;
      ipv6 = (strcmp(serv->proto->name, "IPv6") == 0)     ? TRUE : FALSE;
      uds  = (strcmp(serv->proto->name, "UNIX") == 0)     ? TRUE : FALSE;
      ssl  = (serv->create_options & LINC_CONNECTION_SSL) ? TRUE : FALSE;

      if( osi == NULL && (uds || (ipv6 && !ssl)) )
        {
          osi = g_new0(IOP_TAG_ORBIT_SPECIFIC_info, 1);
          osi->parent.profile_type = IOP_TAG_ORBIT_SPECIFIC;
        }

      if(uds && osi->unix_sock_path == NULL )
        osi->unix_sock_path = g_strdup(serv->local_serv_info);

      if(ipv6 && !ssl)
        osi->ipv6_port = atoi(serv->local_serv_info);

      if(ipv4)
        {
          if(iiop == NULL)
            {
              iiop = g_new0(IOP_TAG_INTERNET_IOP_info, 1);
              iiop->host = g_strdup(serv->local_host_info);
              profiles = g_slist_append(profiles, iiop);
            }

          if(ssl)
            {
              IOP_TAG_SSL_SEC_TRANS_info *sslsec;

              sslsec = g_new0(IOP_TAG_SSL_SEC_TRANS_info, 1);
              sslsec->parent.component_type = IOP_TAG_SSL_SEC_TRANS;
              /* integrity & confidentiality */
              sslsec->target_supports = sslsec->target_requires = 2|4;
              sslsec->port = atoi(serv->local_serv_info);
              iiop->components = g_slist_append(iiop->components, sslsec);
            }
          else
            {
              g_assert(!iiop->port);
              iiop->port = atoi(serv->local_serv_info);
              iiop->iiop_version = obj->orb->default_giop_version;
            }
        }
      else
        {
          GSList *ltmp2;
          IOP_TAG_GENERIC_IOP_info *giop;

          for(giop = NULL, ltmp2 = profiles; ltmp2; ltmp2 = ltmp2->next)
            {
              IOP_TAG_GENERIC_IOP_info *giopt;

              giopt = ltmp2->data;
              if(giopt->parent.profile_type == IOP_TAG_GENERIC_IOP
                 && strcmp(giopt->proto, serv->proto->name))
                {
                  giop = giopt;
                  break;
                }
            }

          if(giop == NULL)
            {
              giop = g_new0(IOP_TAG_GENERIC_IOP_info, 1);
              giop->parent.profile_type = IOP_TAG_GENERIC_IOP;
              giop->iiop_version = obj->orb->default_giop_version;
              giop->proto = g_strdup(serv->proto->name);
              giop->host = g_strdup(serv->local_host_info);
              profiles = g_slist_append(profiles, giop);
            }

          if(ssl)
            {
              IOP_TAG_GENERIC_SSL_SEC_TRANS_info *sslsec;

              sslsec = g_new0(IOP_TAG_GENERIC_SSL_SEC_TRANS_info, 1);
              sslsec->parent.component_type = IOP_TAG_GENERIC_SSL_SEC_TRANS;
              sslsec->service = g_strdup(serv->local_serv_info);
              giop->components = g_slist_append(giop->components, sslsec);
            }
          else
            {
              g_assert(!giop->service);
              giop->service = g_strdup(serv->local_serv_info);
            }

          need_objkey_component = TRUE;
        }
    }

  if(osi)
    profiles = g_slist_append(profiles, osi);

  if(need_objkey_component)
    {
      mci = g_new0(IOP_TAG_MULTIPLE_COMPONENTS_info, 1);
      mci->parent.profile_type = IOP_TAG_MULTIPLE_COMPONENTS;
      if(need_objkey_component)
        {
          IOP_TAG_COMPLETE_OBJECT_KEY_info *coki;
          coki = g_new0(IOP_TAG_COMPLETE_OBJECT_KEY_info, 1);
          coki->parent.component_type = IOP_TAG_COMPLETE_OBJECT_KEY;
          mci->components = g_slist_append(mci->components, coki);
        }
      profiles = g_slist_append(profiles, mci);
    }

 obj->profile_list = profiles;
 if(!objrefs)
   objrefs = g_hash_table_new(g_CORBA_Object_hash, g_CORBA_Object_equal);
 g_hash_table_insert(objrefs, obj, obj);
 }

static void
IOP_ObjectKey_marshal(CORBA_Object obj, GIOPSendBuffer *buf, 
		      IOP_ObjectKey_info *oki)
{
  if ( oki == NULL )
    oki = obj->oki;

  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append(buf, oki->object_key_vec.iov_base,
			  oki->object_key._length + 4 /* cant use iov_len because it includes tail alignment */);
}

static void
IOP_TAG_GENERIC_SSL_SEC_TRANS_marshal(CORBA_Object obj, GIOPSendBuffer *buf,
				      IOP_Component_info *ci)
{
  CORBA_unsigned_long len;
  IOP_TAG_GENERIC_SSL_SEC_TRANS_info *ssli = (IOP_TAG_GENERIC_SSL_SEC_TRANS_info *)ci;

  len = strlen(ssli->service) + 1;
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append_indirect(buf, &len, 4);
  giop_send_buffer_append(buf, ssli->service, 4);
}

static void
IOP_TAG_SSL_SEC_TRANS_marshal(CORBA_Object obj, GIOPSendBuffer *buf,
			      IOP_Component_info *ci)
{
  IOP_TAG_SSL_SEC_TRANS_info *ssli = (IOP_TAG_SSL_SEC_TRANS_info *)ci;

  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append(buf, &ssli->target_supports, 10);
}

static void
IOP_TAG_COMPLETE_OBJECT_KEY_marshal(CORBA_Object obj, GIOPSendBuffer *buf,
				    IOP_Component_info *ci)
{
  IOP_TAG_COMPLETE_OBJECT_KEY_info *coki = (IOP_TAG_COMPLETE_OBJECT_KEY_info *)ci;

  IOP_ObjectKey_marshal(obj, buf, coki->oki);
}

static void
IOP_UnknownComponent_marshal(CORBA_Object obj, GIOPSendBuffer *buf, 
			     IOP_Component_info *ci)
{
  IOP_UnknownComponent_info *uci = (IOP_UnknownComponent_info *)ci;

  giop_send_buffer_append(buf, uci->data._buffer, uci->data._length);
}

static void
IOP_components_marshal(CORBA_Object obj, GIOPSendBuffer *buf,
		       GSList *components)
{
  CORBA_unsigned_long len, *lenptr;
  GSList *cur;

  len = g_slist_length(components);
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append_indirect(buf, &len, 4);

  for(cur = components; cur; cur = cur->next)
    {
      IOP_Component_info *ci = cur->data;

      giop_send_buffer_align(buf, 4);
      giop_send_buffer_append(buf, &ci->component_type, 4);

      switch(ci->component_type)
	{
	case IOP_TAG_GENERIC_SSL_SEC_TRANS:
	case IOP_TAG_SSL_SEC_TRANS:
	  /* Help out with putting an encaps thing on the wire */
	  lenptr = (CORBA_unsigned_long *)
	    giop_send_buffer_append_indirect(buf, &len, 4);
	  len = buf->msg.header.message_size;
	  giop_send_buffer_append(buf, &buf->msg.header.flags, 1);
	  break;
	default:
	  lenptr = NULL;
	  break;
	}

      switch(ci->component_type)
	{
	case IOP_TAG_GENERIC_SSL_SEC_TRANS:
	  IOP_TAG_GENERIC_SSL_SEC_TRANS_marshal(obj, buf, ci);
	  break;
	case IOP_TAG_SSL_SEC_TRANS:
	  IOP_TAG_SSL_SEC_TRANS_marshal(obj, buf, ci);
	  break;
	case IOP_TAG_COMPLETE_OBJECT_KEY:
	  IOP_TAG_COMPLETE_OBJECT_KEY_marshal(obj, buf, ci);
	  break;
	default:
	  IOP_UnknownComponent_marshal(obj, buf, ci);
	  break;
	}

      if(lenptr)
	*lenptr = buf->msg.header.message_size - len;
    }
}

static void
IOP_TAG_INTERNET_IOP_marshal(CORBA_Object obj, GIOPSendBuffer *buf, 
			     IOP_Profile_info *profile)
{
  IOP_TAG_INTERNET_IOP_info *iiop = (IOP_TAG_INTERNET_IOP_info *)profile;
  CORBA_unsigned_long len;

  giop_send_buffer_append(buf, giop_version_ids[iiop->iiop_version], 2);
  len = strlen(iiop->host) + 1;
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append_indirect(buf, &len, 4);
  giop_send_buffer_append(buf, iiop->host, len);

  giop_send_buffer_align(buf, 2);
  giop_send_buffer_append(buf, &iiop->port, 2);

  IOP_ObjectKey_marshal(obj, buf, iiop->oki);

  IOP_components_marshal(obj, buf, iiop->components);
}

static void
IOP_TAG_GENERIC_IOP_marshal(CORBA_Object obj, GIOPSendBuffer *buf, 
			    IOP_Profile_info *profile)
{
  IOP_TAG_GENERIC_IOP_info *giop = (IOP_TAG_GENERIC_IOP_info *)profile;
  CORBA_unsigned_long len;

  giop_send_buffer_append(buf, giop_version_ids[giop->iiop_version], 2);
  len = strlen(giop->proto) + 1;
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append_indirect(buf, &len, 4);
  giop_send_buffer_append(buf, giop->proto, len);

  len = strlen(giop->host) + 1;
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append_indirect(buf, &len, 4);
  giop_send_buffer_append(buf, giop->host, len);

  len = strlen(giop->service) + 1;
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append_indirect(buf, &len, 4);
  giop_send_buffer_append(buf, giop->service, len);

  IOP_components_marshal(obj, buf, giop->components);
}

static void
IOP_TAG_MULTIPLE_COMPONENTS_marshal(CORBA_Object obj, GIOPSendBuffer *buf,
				    IOP_Profile_info *profile)
{
  IOP_TAG_MULTIPLE_COMPONENTS_info *mci = (IOP_TAG_MULTIPLE_COMPONENTS_info*)
    profile;

  IOP_components_marshal(obj, buf, mci->components);
}

static void
IOP_TAG_ORBIT_SPECIFIC_marshal(CORBA_Object obj, GIOPSendBuffer *buf,
			       IOP_Profile_info *profile)
{
  CORBA_unsigned_long len;
  IOP_TAG_ORBIT_SPECIFIC_info *osi = (IOP_TAG_ORBIT_SPECIFIC_info*)profile;

  len = strlen(osi->unix_sock_path) + 1;
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append_indirect(buf, &len, 4);
  giop_send_buffer_append(buf, osi->unix_sock_path, len);
  giop_send_buffer_align(buf, 2);
  giop_send_buffer_append(buf, &osi->ipv6_port, 2);
  IOP_ObjectKey_marshal(obj, buf, obj->oki);
}

static void
IOP_UnknownProfile_marshal(CORBA_Object obj, GIOPSendBuffer *buf, 
			   IOP_Profile_info *pi)
{
  IOP_UnknownProfile_info *upi = (IOP_UnknownProfile_info *)pi;

  giop_send_buffer_append(buf, upi->data._buffer, upi->data._length);
}

static void
ORBit_marshal_profile(CORBA_Object obj, GIOPSendBuffer *buf, 
		      IOP_Profile_info *profile)
{
  CORBA_unsigned_long *seqlen, dumb;
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append(buf, &profile->profile_type, 4);
  seqlen = (CORBA_unsigned_long *)
    giop_send_buffer_append_indirect(buf, &dumb, 4);
  *seqlen = 0;

  dumb = buf->msg.header.message_size;
  switch(profile->profile_type)
    {
    case IOP_TAG_INTERNET_IOP:
      giop_send_buffer_append(buf, &buf->msg.header.flags, 1);
      IOP_TAG_INTERNET_IOP_marshal(obj, buf, profile);
      break;
    case IOP_TAG_ORBIT_SPECIFIC:
      giop_send_buffer_append(buf, &buf->msg.header.flags, 1);
      IOP_TAG_ORBIT_SPECIFIC_marshal(obj, buf, profile);
      break;
    case IOP_TAG_GENERIC_IOP:
      giop_send_buffer_append(buf, &buf->msg.header.flags, 1);
      IOP_TAG_GENERIC_IOP_marshal(obj, buf, profile);
      break;
    case IOP_TAG_MULTIPLE_COMPONENTS:
      giop_send_buffer_append(buf, &buf->msg.header.flags, 1);
      IOP_TAG_MULTIPLE_COMPONENTS_marshal(obj, buf, profile);
      break;
    default:
      IOP_UnknownProfile_marshal(obj, buf, profile);
      break;
    }
  *seqlen = buf->msg.header.message_size - dumb;
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
	  ORBit_generate_profiles( obj );
      num_profiles = g_slist_length(obj->profile_list);
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
        fprintf (stderr, "%s\n", IOP_Profile_dump (obj, cur->data));
#endif
	ORBit_marshal_profile(obj, buf, cur->data);
      }
}

/* FIXME: WTF ! */
static GIOPRecvBuffer *gbuf = NULL;

static IOP_ObjectKey_info *
IOP_ObjectKey_demarshal(GIOPRecvBuffer *buf)
{
  CORBA_unsigned_long len, rlen;
  IOP_ObjectKey_info *retval;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    goto errout;
  len = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    goto errout;

  rlen = ALIGN_VALUE(len, 4);
  retval = g_malloc(G_STRUCT_OFFSET(IOP_ObjectKey_info, object_key_data._buffer)
		    + rlen);
  retval->object_key_data._length = len;
  retval->object_key._length = len;
  retval->object_key._release = CORBA_FALSE;
  retval->object_key._buffer = retval->object_key_data._buffer;
  memcpy(retval->object_key_data._buffer, buf->cur, len);
  buf->cur += len;
  retval->object_key_vec.iov_base = &retval->object_key_data;
  retval->object_key_vec.iov_len = 4 + rlen;

  return retval;

 errout:
  return NULL;
}

static IOP_Component_info *
IOP_TAG_SSL_SEC_TRANS_demarshal(IOP_ComponentId id, GIOPRecvBuffer *buf)
{
  IOP_TAG_SSL_SEC_TRANS_info *retval = NULL;
  GIOPRecvBuffer *sub;

  sub = giop_recv_buffer_use_encaps_buf(buf);
  if(!sub)
    return NULL;

  sub->cur = ALIGN_ADDRESS(sub->cur, 4);

  if((sub->cur + 10) > sub->end)
    {
      giop_recv_buffer_unuse(sub);
      return NULL;
    }
  retval = g_new(IOP_TAG_SSL_SEC_TRANS_info, 1);
  retval->parent.component_type = id;

  retval->target_supports = *(CORBA_unsigned_long *)sub->cur;
  if(giop_msg_conversion_needed(buf))
    retval->target_supports = GUINT32_SWAP_LE_BE(retval->target_supports);
  sub->cur += 4;
  retval->target_requires = *(CORBA_unsigned_long *)sub->cur;
  if(giop_msg_conversion_needed(buf))
    retval->target_requires = GUINT32_SWAP_LE_BE(retval->target_requires);
  sub->cur += 4;
  retval->port = *(CORBA_unsigned_short *)sub->cur;
  if(giop_msg_conversion_needed(buf))
    retval->port = GUINT16_SWAP_LE_BE(retval->port);
  sub->cur += 2;

  giop_recv_buffer_unuse(sub);

  return (IOP_Component_info *)retval;
}


static IOP_Component_info *
IOP_TAG_GENERIC_SSL_SEC_TRANS_demarshal(IOP_ComponentId id, GIOPRecvBuffer *buf)
{
  IOP_TAG_GENERIC_SSL_SEC_TRANS_info *retval = NULL;
  GIOPRecvBuffer *sub;
  CORBA_unsigned_long len;

  sub = giop_recv_buffer_use_encaps_buf(buf);
  if(!sub)
    return NULL;

  sub->cur = ALIGN_ADDRESS(sub->cur, 4);
  if((sub->cur + 4) > sub->end)
    goto errout;
  len = *(CORBA_unsigned_long *)sub->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  sub->cur += 4;
  if((sub->cur + len) > sub->end
     || (sub->cur + len) < sub->cur)
    goto errout;
  retval = g_new(IOP_TAG_GENERIC_SSL_SEC_TRANS_info, 1);
  retval->parent.component_type = id;
  retval->service = g_memdup(sub->cur, len);

  giop_recv_buffer_unuse(sub);
  return (IOP_Component_info *)retval;

 errout:
  g_free(retval);
  giop_recv_buffer_unuse(sub);
  return NULL;
}

static IOP_Component_info *
IOP_TAG_COMPLETE_OBJECT_KEY_demarshal(IOP_ComponentId id, GIOPRecvBuffer *buf)
{
  IOP_TAG_COMPLETE_OBJECT_KEY_info *retval;
  IOP_ObjectKey_info *oki;
  
  oki = IOP_ObjectKey_demarshal(buf);
  if(!oki)
    return NULL;
  retval = g_new(IOP_TAG_COMPLETE_OBJECT_KEY_info, 1);
  retval->parent.component_type = id;
  retval->oki = oki;
  return (IOP_Component_info *)retval;
}

static IOP_Component_info *
IOP_UnknownComponent_demarshal(IOP_ComponentId p, GIOPRecvBuffer *buf)
{
  IOP_UnknownComponent_info *retval;
  CORBA_unsigned_long len;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    return NULL;
  len = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    return NULL;
  retval = g_new(IOP_UnknownComponent_info, 1);
  retval->parent.component_type = p;
  retval->data._length = len;
  retval->data._buffer = g_memdup(buf->cur, len);
  retval->data._release = CORBA_FALSE; /* We free this manually */
  buf->cur += len;
  
  return (IOP_Component_info *)retval;
}

static gboolean
IOP_components_demarshal(GIOPRecvBuffer *buf, GSList **components)
{
  GSList *retval;
  CORBA_unsigned_long len, i;
  IOP_ComponentId cid;

  *components = retval = NULL;
  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    return TRUE;
  len = *(CORBA_unsigned_long*)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;
  for(i = 0; i < len; i++)
    {
      IOP_Component_info *c;

      buf->cur = ALIGN_ADDRESS(buf->cur, 4);
      if((buf->cur + 4) > buf->end)
	goto errout;
      cid = *(CORBA_unsigned_long *)buf->cur;
      if(giop_msg_conversion_needed(buf))
	cid = GUINT32_SWAP_LE_BE(cid);
      buf->cur += 4;
      switch(cid)
	{
	case IOP_TAG_COMPLETE_OBJECT_KEY:
	  c = IOP_TAG_COMPLETE_OBJECT_KEY_demarshal(cid, buf);
	  break;
	case IOP_TAG_GENERIC_SSL_SEC_TRANS:
	  c = IOP_TAG_GENERIC_SSL_SEC_TRANS_demarshal(cid, buf);
	  break;
	case IOP_TAG_SSL_SEC_TRANS:
	  c = IOP_TAG_SSL_SEC_TRANS_demarshal(cid, buf);
	  break;
	default:
	  c = IOP_UnknownComponent_demarshal(cid, buf);
	  break;
	}
      if(c)
	retval = g_slist_append(retval, c);
      else
	goto errout;
    }

  *components = retval;
  return FALSE;

 errout:
  IOP_components_free(retval);
  return TRUE;
}

static IOP_Profile_info *
IOP_UnknownProfile_demarshal(IOP_ProfileId p, GIOPRecvBuffer *buf,
			     CORBA_ORB orb)
{
  IOP_UnknownProfile_info *retval;
  CORBA_unsigned_long len;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    goto errout;
  len = *(CORBA_unsigned_long*)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    goto errout;
  retval = g_new(IOP_UnknownProfile_info, 1);
  retval->parent.profile_type = p;
  retval->data._length = len;
  retval->data._buffer = g_memdup(buf->cur, len);
  retval->data._release = CORBA_FALSE; /* We free this manually */
  buf->cur += len;
  
  return (IOP_Profile_info *)retval;

 errout:
  return NULL;
}

static void
IOP_UnknownProfile_free (IOP_Profile_info *p)
{
  IOP_UnknownProfile_info *info = (IOP_UnknownProfile_info *) p;

  g_free (info->data._buffer);
}

static IOP_Profile_info *
IOP_TAG_ORBIT_SPECIFIC_demarshal(IOP_ProfileId p, GIOPRecvBuffer *pbuf,
				 CORBA_ORB orb)
{
  IOP_TAG_ORBIT_SPECIFIC_info *retval = NULL;
  CORBA_unsigned_long len;
  GIOPRecvBuffer *buf;

  buf = giop_recv_buffer_use_encaps_buf(pbuf);
  if(!buf)
    goto errout;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    return NULL;
  len = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    goto errout;

  retval = g_new(IOP_TAG_ORBIT_SPECIFIC_info, 1);
  retval->parent.profile_type = p;
  retval->unix_sock_path = g_malloc(len);
  memcpy(retval->unix_sock_path, buf->cur, len);
  buf->cur += len;
  buf->cur = ALIGN_ADDRESS(buf->cur, 2);
  if((buf->cur + 2) > buf->end)
    goto errout;
  retval->ipv6_port = *(CORBA_unsigned_short *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    retval->ipv6_port = GUINT16_SWAP_LE_BE(retval->ipv6_port);
  buf->cur += 2;
  retval->oki = IOP_ObjectKey_demarshal(buf);
  if(!retval->oki)
    goto errout;

  giop_recv_buffer_unuse(buf);
  return (IOP_Profile_info *)retval;

 errout:
  if(retval)
    {
      g_free(retval->oki);
      g_free(retval->unix_sock_path);
      g_free(retval);
    }
  giop_recv_buffer_unuse(buf);
  return NULL;
}

static void
IOP_TAG_ORBIT_SPECIFIC_free (IOP_Profile_info *p)
{
  IOP_TAG_ORBIT_SPECIFIC_info *info = (IOP_TAG_ORBIT_SPECIFIC_info *) p;

  g_free(info->oki);
  g_free(info->unix_sock_path);
}

static IOP_Profile_info *
IOP_TAG_MULTIPLE_COMPONENTS_demarshal(IOP_ProfileId p, GIOPRecvBuffer *pbuf,
				      CORBA_ORB orb)
{
  IOP_TAG_MULTIPLE_COMPONENTS_info *retval = NULL;
  GIOPRecvBuffer *buf;
  GSList *components;

  buf = giop_recv_buffer_use_encaps_buf(pbuf);
  if(buf && !IOP_components_demarshal(buf, &components))
    {
      retval = g_new(IOP_TAG_MULTIPLE_COMPONENTS_info, 1);
      retval->parent.profile_type = p;
      retval->components = components;
    }
  giop_recv_buffer_unuse(buf);
  return (IOP_Profile_info*)retval;
}

static void
IOP_TAG_MULTIPLE_COMPONENTS_free (IOP_Profile_info *p)
{
  IOP_TAG_MULTIPLE_COMPONENTS_info *info = (IOP_TAG_MULTIPLE_COMPONENTS_info *) p;

  IOP_components_free (info->components);
}

static IOP_Profile_info *
IOP_TAG_GENERIC_IOP_demarshal(IOP_ProfileId p, GIOPRecvBuffer *pbuf,
			      CORBA_ORB orb)
{
  IOP_TAG_GENERIC_IOP_info *retval;
  CORBA_octet v1, v2;
  GIOPVersion version;
  CORBA_unsigned_long len;
  GIOPRecvBuffer *buf;

  buf = giop_recv_buffer_use_encaps_buf(pbuf);
  if(!buf)
    goto eo2;

  if((buf->cur + 2) > buf->end)
    goto eo2;
  v1 = *(buf->cur++);
  v2 = *(buf->cur++);
  switch(v1)
    {
    case 1:
      switch(v2)
	{
	case 0:
	  version = GIOP_1_0;
	  break;
	case 1:
	  version = GIOP_1_1;
	  break;
	case 2:
	  version = GIOP_1_2;
	  break;
	default:
	  goto eo2;
	  break;
	}
      break;
    default:
      goto eo2;
      break;
    }
  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    goto eo2;
  len = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;

  retval = g_new(IOP_TAG_GENERIC_IOP_info, 1);
  retval->parent.profile_type = IOP_TAG_GENERIC_IOP;
  retval->iiop_version = version;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    goto errout;
  retval->proto = g_memdup(buf->cur, len);
  buf->cur += len;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    goto errout;
  len = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    goto errout;
  retval->host = g_memdup(buf->cur, len);
  buf->cur += len;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    goto errout;
  len = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    goto errout;
  retval->service = g_memdup(buf->cur, len);
  buf->cur += len;

  if(IOP_components_demarshal(buf, &retval->components))
    goto errout;

  giop_recv_buffer_unuse(buf);
  return (IOP_Profile_info *)retval;

 errout:
  if(retval)
    {
      IOP_components_free(retval->components);
      g_free(retval->proto);
      g_free(retval->host);
      g_free(retval->service);
    }

  g_free(retval);
 eo2:
  giop_recv_buffer_unuse(buf);
  return NULL;
}

static void
IOP_TAG_GENERIC_IOP_free (IOP_Profile_info *p)
{
  IOP_TAG_GENERIC_IOP_info *info = (IOP_TAG_GENERIC_IOP_info *) p;

  IOP_components_free (info->components);
  g_free (info->proto);
  g_free (info->host);
  g_free (info->service);
}

static IOP_Profile_info *
IOP_TAG_INTERNET_IOP_demarshal(IOP_ProfileId p, GIOPRecvBuffer *pbuf,
			       CORBA_ORB orb)
{
  IOP_TAG_INTERNET_IOP_info *retval;
  CORBA_octet v1, v2;
  GIOPVersion version;
  CORBA_unsigned_long len;
  GIOPRecvBuffer *buf;

  buf = giop_recv_buffer_use_encaps_buf(pbuf);
  if(!buf)
    goto eo2;

  if((buf->cur + 2) > buf->end)
    goto eo2;
  v1 = *(buf->cur++);
  v2 = *(buf->cur++);
  switch(v1)
    {
    case 1:
      switch(v2)
	{
	case 0:
	  version = GIOP_1_0;
	  break;
	case 1:
	  version = GIOP_1_1;
	  break;
	case 2:
	  version = GIOP_1_2;
	  break;
	default:
	  goto eo2;
	  break;
	}
      break;
    default:
      goto eo2;
      break;
    }
  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    goto eo2;
  len = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;

  retval = g_new(IOP_TAG_INTERNET_IOP_info, 1);
  retval->parent.profile_type = IOP_TAG_INTERNET_IOP;
  retval->iiop_version = version;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    goto errout;
  retval->host = g_memdup(buf->cur, len);
  buf->cur += len;
  buf->cur = ALIGN_ADDRESS(buf->cur, 2);
  if((buf->cur + 2) > buf->end)
    goto errout;
  if(giop_msg_conversion_needed(buf))
    retval->port = GUINT16_SWAP_LE_BE(*(CORBA_unsigned_short *)buf->cur);
  else
    retval->port = *(CORBA_unsigned_short *)buf->cur;
  buf->cur += 2;
  retval->oki = IOP_ObjectKey_demarshal(buf);
  if(!retval->oki)
    goto errout;
  if(version > GIOP_1_0)
    {
      if(IOP_components_demarshal(buf, &retval->components))
	goto errout;
    }
  else
    retval->components = NULL;

  giop_recv_buffer_unuse(buf);
  return (IOP_Profile_info *)retval;

 errout:
  if(retval)
    {
      IOP_components_free(retval->components);
      g_free(retval->host);
      g_free(retval->oki);

      g_free(retval);
    }

  eo2:
  giop_recv_buffer_unuse(buf);
  return NULL;
}

static void
IOP_TAG_INTERNET_IOP_free (IOP_Profile_info *p)
{
  IOP_TAG_INTERNET_IOP_info *info = (IOP_TAG_INTERNET_IOP_info *) p;

  IOP_components_free (info->components);
  g_free (info->host);
  g_free (info->oki);
}

static void
ORBit_profile_free (IOP_Profile_info *p)
{
  switch (p->profile_type) {
    case IOP_TAG_INTERNET_IOP:
      IOP_TAG_INTERNET_IOP_free (p);
      break;
    case IOP_TAG_MULTIPLE_COMPONENTS:
      IOP_TAG_MULTIPLE_COMPONENTS_free (p);
      break;
    case IOP_TAG_GENERIC_IOP:
      IOP_TAG_GENERIC_IOP_free (p);
      break;
    case IOP_TAG_ORBIT_SPECIFIC:
      IOP_TAG_ORBIT_SPECIFIC_free (p);
      break;
    default:
      IOP_UnknownProfile_free (p);
      break;
  }
  g_free (p);
}

static IOP_Profile_info *
ORBit_demarshal_profile(GIOPRecvBuffer *buf, CORBA_ORB orb)
{
  IOP_ProfileId p;
  IOP_Profile_info *retval;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);

  if((buf->cur + 4) > buf->end)
    return NULL;
  p = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    p = GUINT32_SWAP_LE_BE(p);
  buf->cur += 4;

  switch(p)
    {
    case IOP_TAG_INTERNET_IOP:
      retval = IOP_TAG_INTERNET_IOP_demarshal(p, buf, orb);
      break;
    case IOP_TAG_MULTIPLE_COMPONENTS:
      retval = IOP_TAG_MULTIPLE_COMPONENTS_demarshal(p, buf, orb);
      break;
    case IOP_TAG_GENERIC_IOP:
      retval = IOP_TAG_GENERIC_IOP_demarshal(p, buf, orb);
      break;
    case IOP_TAG_ORBIT_SPECIFIC:
      retval = IOP_TAG_ORBIT_SPECIFIC_demarshal(p, buf, orb);
      break;
    default:
      retval = IOP_UnknownProfile_demarshal(p, buf, orb);
      break;
    }

  return retval;
}

static gboolean
ORBit_demarshal_IOR(CORBA_ORB orb, GIOPRecvBuffer *buf,
		    char **ret_type_id, GSList **ret_profiles)
{
  GSList *profiles = NULL;
  char *type_id;
  CORBA_unsigned_long len, num_profiles;
  int i;

  gbuf = buf;
  
  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    goto errout;
  len = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    goto errout;
  type_id = buf->cur;
  buf->cur += len;
  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    goto errout;
  num_profiles = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    num_profiles = GUINT32_SWAP_LE_BE(num_profiles);
  buf->cur += 4;
  if(!strcmp(type_id, "") && num_profiles == 0)
    type_id = NULL;

  for(i = 0; i < num_profiles; i++)
    {
      IOP_Profile_info *profile;
      profile = ORBit_demarshal_profile(buf, orb);
      if(profile)
	profiles = g_slist_append(profiles, profile);
      else
	  goto errout;
    }

  *ret_profiles = profiles;
  if(ret_type_id)
    *ret_type_id = type_id;
  return FALSE;

 errout:
  ORBit_delete_profiles(&profiles);
  return TRUE;
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
