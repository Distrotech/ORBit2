#include "config.h"
#include <orbit/orbit.h>
#include <string.h>
#include "corba-ops.h"

static void ORBit_profile_free(IOP_Profile_info *p);
static guint g_CORBA_Object_hash(gconstpointer key);
static gboolean g_CORBA_Object_equal(gconstpointer a, gconstpointer b);
static IOP_Profile_info *IOP_profile_find(GSList *list, IOP_ProfileId type,
					  GSList **pos);
static IOP_Component_info *IOP_component_find(GSList *list, IOP_ComponentId type,
					      GSList **pos);
static void IOP_components_free(GSList *components);

static GHashTable *objrefs = NULL;

static void IOP_components_free(GSList *components)
{
}

static CORBA_unsigned_long
ORBit_classinfo_lookup_id(const char *type_id)
{
  return 0;
}

#define ORBIT_SERVANT_TO_CLASSINFO(x) NULL

static void
CORBA_Object_release_cb(ORBit_RootObject robj)
{
  CORBA_Object obj = (CORBA_Object)robj;

  g_hash_table_remove(objrefs, obj);
  giop_connection_unref(obj->connection);
  g_slist_foreach(obj->profile_list, (GFunc)ORBit_profile_free, NULL);
  g_slist_free(obj->profile_list);
  g_free(obj->type_id);
  /* XXX finish me */
  g_free(obj);
}

static ORBit_RootObject_Interface objref_if = {
  ORBIT_ROT_OBJREF,
  CORBA_Object_release_cb
};

static CORBA_Object
ORBit_objref_new(CORBA_ORB orb, const char *type_id, GSList *profiles)
{
  CORBA_Object retval;
  retval = g_new0(struct CORBA_Object_type, 1);

  ORBit_RootObject_init((ORBit_RootObject)retval, &objref_if);
  retval->type_id = g_strdup(type_id);
  retval->profile_list = profiles;
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
  if(!retval)
    retval = ORBit_objref_new(orb, type_id, profiles);
  else
    g_slist_foreach(profiles, (GFunc)ORBit_profile_free, NULL);

  retval = CORBA_Object_duplicate(retval, NULL);

  O_MUTEX_UNLOCK(ORBit_RootObject_lifecycle_lock);

  return retval;
}

static gboolean
ORBit_try_connection(CORBA_Object obj)
{
  while(1)
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

  if(obj->connection && ORBit_try_connection(obj))
    return obj->connection;
  
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
	    return obj->connection;
	}
    }

  return NULL;
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
CORBA_Object_is_a(CORBA_Object _obj,
		  const CORBA_char * logical_type_id,
		  CORBA_Environment * ev)
{
  if(!strcmp(logical_type_id, "IDL:CORBA/Object:1.0"))
    return CORBA_TRUE;

  if(!_obj)
    return CORBA_FALSE;

  if(!strcmp(logical_type_id, _obj->type_id))
    return CORBA_TRUE;

  if(_obj->bypass_obj)
    {
      CORBA_unsigned_long clsid
	= ORBit_classinfo_lookup_id(logical_type_id);
      PortableServer_ClassInfo *ci 
	= ORBIT_SERVANT_TO_CLASSINFO(_obj->bypass_obj->servant);
      return (clsid && (clsid < ci->vepvlen) && ci->vepvmap[clsid]);
  
      return CORBA_FALSE;
    }
  else
    return ORBit_ObjectImpl__is_a(_obj, logical_type_id, ev);
}

CORBA_boolean
CORBA_Object_non_existent(CORBA_Object _obj,
			  CORBA_Environment * ev)
{
  if(_obj)
    return ORBit_object_get_connection(_obj)?CORBA_TRUE:CORBA_FALSE;
  return CORBA_FALSE;
}

static gboolean
IOP_Profile_equal(gpointer d1, gpointer d2)
{
  IOP_TAG_INTERNET_IOP_info *iiop1, *iiop2;
  IOP_TAG_GENERIC_IOP_info *giop1, *giop2;
  IOP_ProfileId t1, t2;

  t1 = ((IOP_Profile_info *)d1)->profile_type;
  t2 = ((IOP_Profile_info *)d2)->profile_type;
  if(t1 != t2)
    return FALSE;

  switch(t1)
    {
    case IOP_TAG_INTERNET_IOP:
      iiop1 = d1; iiop2 = d2;
      if(iiop1->oki->object_key._length != iiop2->oki->object_key._length)
	return FALSE;
      if(iiop1->port != iiop2->port)
	return FALSE;
      if(memcmp(iiop1->oki->object_key._buffer,
		iiop2->oki->object_key._buffer,
		iiop1->oki->object_key._length))
	return FALSE;
      break;
    case IOP_TAG_GENERIC_IOP:
      giop1 = d1; giop2 = d2;
      if(strcmp(giop1->proto, giop2->proto))
	return FALSE;
      if(strcmp(giop1->host, giop2->host))
	return FALSE;
      if(strcmp(giop1->service, giop2->service))
	return FALSE;
      if(memcmp(iiop1->oki->object_key._buffer,
		iiop2->oki->object_key._buffer,
		iiop1->oki->object_key._length))
	return FALSE;
      break;
    default:
      return FALSE;
      break;
    }

  return TRUE;
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
	  if(IOP_Profile_equal(cur1->data, cur2->data))
	    return TRUE;
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
  CORBA_Object _obj;

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
IOP_ObjectKey_marshal(GIOPSendBuffer *buf, IOP_ObjectKey_info *oki)
{
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append(buf, oki->object_key_vec.iov_base,
			  oki->object_key._length + 4 /* cant use iov_len because it includes tail alignment */);
}

static void
IOP_TAG_GENERIC_SSL_SEC_TRANS_marshal(GIOPSendBuffer *buf,
				      IOP_Component_info *ci)
{
  CORBA_unsigned_long len;
  IOP_TAG_GENERIC_SSL_SEC_TRANS_info *ssli = (IOP_TAG_GENERIC_SSL_SEC_TRANS_info *)ci;

  len = strlen(ssli->service) + 1;
  giop_send_buffer_append_indirect(buf, &len, 4);
  giop_send_buffer_append(buf, ssli->service, 4);
}

static void
IOP_TAG_COMPLETE_OBJECT_KEY_marshal(GIOPSendBuffer *buf,
				    IOP_Component_info *ci)
{
  IOP_TAG_COMPLETE_OBJECT_KEY_info *coki = (IOP_TAG_COMPLETE_OBJECT_KEY_info *)ci;

  IOP_ObjectKey_marshal(buf, coki->oki);
}

static void
IOP_UnknownComponent_marshal(GIOPSendBuffer *buf, IOP_Component_info *ci)
{
  IOP_UnknownComponent_info *uci = (IOP_UnknownComponent_info *)ci;

  giop_send_buffer_append(buf, uci->data._buffer, uci->data._length);
}

static void
IOP_components_marshal(GIOPSendBuffer *buf, GSList *components)
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
      len = buf->msg.header.message_size;
      lenptr = (CORBA_unsigned_long *)
	giop_send_buffer_append_indirect(buf, &len, 4);

      switch(ci->component_type)
	{
	case IOP_TAG_GENERIC_SSL_SEC_TRANS:
	  IOP_TAG_GENERIC_SSL_SEC_TRANS_marshal(buf, ci);
	  break;
	case IOP_TAG_COMPLETE_OBJECT_KEY:
	  IOP_TAG_COMPLETE_OBJECT_KEY_marshal(buf, ci);
	  break;
	default:
	  IOP_UnknownComponent_marshal(buf, ci);
	  break;
	}

      *lenptr = buf->msg.header.message_size - len;
    }
}

static void
IOP_TAG_INTERNET_IOP_marshal(GIOPSendBuffer *buf, IOP_Profile_info *profile)
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

  IOP_ObjectKey_marshal(buf, iiop->oki);

  IOP_components_marshal(buf, iiop->components);
}

static void
IOP_TAG_GENERIC_IOP_marshal(GIOPSendBuffer *buf, IOP_Profile_info *profile)
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

  IOP_components_marshal(buf, giop->components);
}

static void
IOP_TAG_MULTIPLE_COMPONENTS_marshal(GIOPSendBuffer *buf,
				    IOP_Profile_info *profile)
{
  IOP_TAG_MULTIPLE_COMPONENTS_info *mci = (IOP_TAG_MULTIPLE_COMPONENTS_info*)
    profile;

  IOP_components_marshal(buf, mci->components);
}

static void
IOP_TAG_ORBIT_SPECIFIC_marshal(GIOPSendBuffer *buf,
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
  IOP_ObjectKey_marshal(buf, osi->oki);
}

static void
ORBit_marshal_profile(GIOPSendBuffer *buf, IOP_Profile_info *profile)
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
      IOP_TAG_INTERNET_IOP_marshal(buf, profile);
      break;
    case IOP_TAG_ORBIT_SPECIFIC:
      IOP_TAG_ORBIT_SPECIFIC_marshal(buf, profile);
      break;
    case IOP_TAG_GENERIC_IOP:
      IOP_TAG_GENERIC_IOP_marshal(buf, profile);
      break;
    case IOP_TAG_MULTIPLE_COMPONENTS:
      IOP_TAG_MULTIPLE_COMPONENTS_marshal(buf, profile);
      break;
    }
  *seqlen = buf->msg.header.message_size - dumb;
}

void
ORBit_marshal_object(GIOPSendBuffer *buf, CORBA_Object obj)
{
  CORBA_unsigned_long type_len, num_profiles;
  GSList *cur;

  giop_send_buffer_align(buf, 4);
  type_len = strlen(obj->type_id) + 1;
  giop_send_buffer_append(buf, &type_len, 4);
  giop_send_buffer_append(buf, obj->type_id, type_len);
  num_profiles = g_slist_length(obj->profile_list);
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append_indirect(buf, &num_profiles, 4);
  
  for(cur = obj->profile_list; cur; cur = cur->next)
    ORBit_marshal_profile(buf, cur->data);
}

static void
ORBit_profile_free(IOP_Profile_info *p)
{
  g_free(p);
}

static IOP_ObjectKey_info *
IOP_ObjectKey_demarshal(GIOPRecvBuffer *buf)
{
  CORBA_unsigned_long len, rlen;
  IOP_ObjectKey_info *retval;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    return NULL;
  len = *(CORBA_unsigned_long *)buf->cur;
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    return NULL;

  rlen = ALIGN_VALUE(len, 4);
  retval = g_malloc(G_STRUCT_OFFSET(IOP_ObjectKey_info, object_key_data._buffer)
		    + rlen);
  retval->object_key_data._length = len;
  retval->object_key._length = len;
  retval->object_key._buffer = retval->object_key_data._buffer;
  memcpy(retval->object_key_data._buffer, buf->cur, len);
  retval->object_key_vec.iov_base = &retval->object_key_data;
  retval->object_key_vec.iov_len = 4 + rlen;

  return retval;
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
  sub->cur += 4;
  retval->target_requires = *(CORBA_unsigned_long *)sub->cur;
  sub->cur += 4;
  retval->port = *(CORBA_unsigned_short *)sub->cur;
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

  if((buf->cur + 4) > buf->end)
    return NULL;
  len = *(CORBA_unsigned_long *)buf->cur;
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

  *components = retval = NULL;
  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    return TRUE;
  len = *(CORBA_unsigned_long*)buf->cur;
  buf->cur += 4;
  for(i = 0; i < len; i++)
    {
      IOP_ComponentId cid;
      IOP_Component_info *c;

      buf->cur = ALIGN_ADDRESS(buf->cur, 4);
      if((buf->cur + 4) > buf->end)
	goto errout;
      cid = *(CORBA_unsigned_long *)buf->cur;
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

  if((buf->cur + 4) > buf->end)
    return NULL;
  len = *(CORBA_unsigned_long*)buf->cur;
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    return NULL;
  retval = g_new(IOP_UnknownProfile_info, 1);
  retval->parent.profile_type = p;
  retval->data._length = len;
  retval->data._buffer = g_memdup(buf->cur, len);
  retval->data._release = CORBA_FALSE; /* We free this manually */
  buf->cur += len;
  
  return (IOP_Profile_info *)retval;
}

static IOP_Profile_info *
IOP_TAG_ORBIT_SPECIFIC_demarshal(IOP_ProfileId p, GIOPRecvBuffer *pbuf,
				 CORBA_ORB orb)
{
  IOP_TAG_ORBIT_SPECIFIC_info *retval;
  CORBA_unsigned_long len;
  GIOPRecvBuffer *buf;

  buf = giop_recv_buffer_use_encaps_buf(pbuf);
  if(!buf)
    return NULL;

  if((buf->cur + 4) > buf->end)
    return NULL;
  len = *(CORBA_unsigned_long *)buf->cur;
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    return NULL;

  retval = g_new0(IOP_TAG_ORBIT_SPECIFIC_info, 1);
  retval->parent.profile_type = p;
  retval->unix_sock_path = g_malloc(len-1);
  memcpy(retval->unix_sock_path, buf->cur, len);
  buf->cur += len;
  buf->cur = ALIGN_ADDRESS(buf->cur, 2);
  if((buf->cur + 2) > buf->end)
    goto errout;
  retval->ipv6_port = *(CORBA_unsigned_short *)buf->cur;
  buf->cur += 2;
  retval->oki = IOP_ObjectKey_demarshal(buf);
  if(!retval->oki)
    goto errout;

  giop_recv_buffer_unuse(buf);
  return (IOP_Profile_info *)retval;

 errout:
  g_free(retval->oki);
  g_free(retval->unix_sock_path);
  g_free(retval);
  giop_recv_buffer_unuse(buf);
  return NULL;
}

static IOP_Profile_info *
IOP_TAG_MULTIPLE_COMPONENTS_demarshal(IOP_ProfileId p, GIOPRecvBuffer *pbuf,
				      CORBA_ORB orb)
{
  IOP_TAG_MULTIPLE_COMPONENTS_info *retval = NULL;
  GIOPRecvBuffer *buf;
  GSList *components;

  buf = giop_recv_buffer_use_encaps_buf(pbuf);
  if(!IOP_components_demarshal(buf, &components))
    {
      retval = g_new(IOP_TAG_MULTIPLE_COMPONENTS_info, 1);
      retval->parent.profile_type = p;
      retval->components = components;
    }
  giop_recv_buffer_unuse(buf);
  return (IOP_Profile_info*)retval;
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
    return NULL;

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
  buf->cur += 4;

  retval = g_new0(IOP_TAG_GENERIC_IOP_info, 1);
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
    return NULL;

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
  buf->cur += 4;

  retval = g_new0(IOP_TAG_INTERNET_IOP_info, 1);
  retval->iiop_version = version;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    goto errout;
  retval->host = g_memdup(buf->cur, len);
  buf->cur += len;
  buf->cur = ALIGN_ADDRESS(buf->cur, 2);
  if((buf->cur + 2) > buf->end)
    goto errout;
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

  giop_recv_buffer_unuse(buf);
  return (IOP_Profile_info *)retval;

 errout:
  if(retval)
    {
      IOP_components_free(retval->components);
      g_free(retval->host);
      g_free(retval->oki);
    }

  g_free(retval);
  eo2:
  giop_recv_buffer_unuse(buf);
  return NULL;
}

static IOP_Profile_info *
ORBit_demarshal_profile(GIOPRecvBuffer *buf, CORBA_ORB orb)
{
  IOP_ProfileId p;
  GIOPRecvBuffer *subbuf;
  IOP_Profile_info *retval;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);

  if((buf->cur + 4) > buf->end)
    return NULL;
  p = *(CORBA_unsigned_long *)buf->cur;
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
  giop_recv_buffer_unuse(subbuf);

  return retval;
}

gboolean
ORBit_demarshal_object(CORBA_Object *obj, GIOPRecvBuffer *buf,
		       CORBA_ORB orb)
{
  GSList *profiles = NULL;
  char *type_id;
  CORBA_unsigned_long len, num_profiles;
  int i;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    goto errout;
  len = *(CORBA_unsigned_long *)buf->cur;
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
  buf->cur += 4;
  if(!strcmp(type_id, "Null") && num_profiles == 0)
    {
      *obj = CORBA_OBJECT_NIL;
      return FALSE;
    }
  for(i = 0; i < num_profiles; i++)
    {
      IOP_Profile_info *profile;
      profile = ORBit_demarshal_profile(buf, orb);
      if(profile)
	profiles = g_slist_append(profiles, profile);
      else
	goto errout;
    }

  *obj = ORBit_objref_find(orb, type_id, profiles);

  return FALSE;

 errout:
  g_slist_foreach(profiles, (GFunc)ORBit_profile_free, NULL);
  return TRUE;
}

gpointer
CORBA_Object__freekids(gpointer mem, gpointer dat)
{
}
