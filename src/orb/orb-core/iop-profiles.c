#include <orbit/orbit.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "iop-profiles.h"
#include "orb-core-private.h"
#include "../poa/orbit-poa-export.h"

#undef DEBUG

/*
 * common set of profiles for Ojbect Adaptor generated refs
 */
static GSList *common_profiles = NULL;

static void IOP_profile_free (IOP_Profile_info *p);

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
IOP_ObjectKey_dump (ORBit_ObjectKey *objkey)
{
	int i;
	GString *str = g_string_sized_new (objkey->_length * 2 + 4);

	for (i = 0; i < objkey->_length; i++)
		g_string_printfa (str, "%2x", objkey->_buffer [i]);

	return g_string_free (str, FALSE);
}

G_GNUC_UNUSED gchar * 
IOP_profile_dump(CORBA_Object obj, gpointer p)
{
	IOP_ProfileId t;
	char         *key = NULL;
	GString      *str = g_string_sized_new (64);

	t = ((IOP_Profile_info *)p)->profile_type;

	switch (t) {
	case IOP_TAG_INTERNET_IOP: {
		IOP_TAG_INTERNET_IOP_info  *iiop = p;
		ORBit_ObjectKey            *objkey;

		objkey = iiop->object_key ? iiop->object_key : obj->object_key;
		
		key = IOP_ObjectKey_dump (objkey);
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
		ORBit_ObjectKey             *objkey;
		
		objkey = os->object_key ? os->object_key : obj->object_key;

		key = IOP_ObjectKey_dump (objkey);

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

gboolean
IOP_profile_get_info(CORBA_Object obj, gpointer *pinfo,
		     GIOPVersion *iiop_version, char **proto,
		     char **host, char **service, gboolean *ssl,
		     ORBit_ObjectKey **objkey, char *tmpbuf)
{
  IOP_TAG_ORBIT_SPECIFIC_info *osi;
  IOP_TAG_INTERNET_IOP_info *iiop;
  IOP_TAG_GENERIC_IOP_info *giop;
  IOP_Profile_info         *pi = (IOP_Profile_info *)pinfo;

  *ssl = FALSE;

#ifdef DEBUG
  {
    char *str;
    fprintf (stderr, "profile for object '%p' '%s'\n",
	     obj, (str = IOP_profile_dump (obj, pi)));
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
      *objkey = iiop->object_key;
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
	*objkey = NULL;
	if(mci)
	  {
	    IOP_TAG_COMPLETE_OBJECT_KEY_info *coki;
	    coki = (IOP_TAG_COMPLETE_OBJECT_KEY_info *)
	      IOP_component_find(mci->components, IOP_TAG_COMPLETE_OBJECT_KEY,
				 NULL);
	    if(coki)
	      *objkey = coki->object_key ? coki->object_key : obj->object_key;
	  }
	return *objkey?TRUE:FALSE;
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
	  *objkey = osi->object_key;
	  return TRUE;
	}
      break;
    default:
      break;
    }

  return FALSE;
}

static IOP_TAG_MULTIPLE_COMPONENTS_info *
IOP_get_mci (GSList *p)
{
  for (; p; p = p->next) {
    if (((IOP_Profile_info *)p->data)->profile_type ==
	IOP_TAG_MULTIPLE_COMPONENTS)
	    return p->data;
  }
  return NULL;
}

static inline gboolean
IOP_ObjectKey_equal (ORBit_ObjectKey *a,
		     ORBit_ObjectKey *b)
{
	if (a->_length != b->_length)
		return FALSE;

	if (memcmp (a->_buffer, b->_buffer, a->_length))
		return FALSE;

	return TRUE;
}

gboolean
IOP_profile_equal (CORBA_Object obj1, CORBA_Object obj2,
		   gpointer d1, gpointer d2)
{
	IOP_TAG_MULTIPLE_COMPONENTS_info *mci1, *mci2;
	IOP_ProfileId                    t1, t2;

	mci1 = IOP_get_mci(obj1->profile_list);
	mci2 = IOP_get_mci(obj2->profile_list);

	t1 = ((IOP_Profile_info *)d1)->profile_type;
	t2 = ((IOP_Profile_info *)d2)->profile_type;

	if(t1 != t2)
		return FALSE;

	switch (t1) {
	case IOP_TAG_INTERNET_IOP: {
		IOP_TAG_INTERNET_IOP_info  *iiop1 = d1;
		IOP_TAG_INTERNET_IOP_info  *iiop2 = d2;
		ORBit_ObjectKey            *objkey1, *objkey2;

		if (iiop1->port != iiop2->port)
			return FALSE;

		objkey1 = iiop1->object_key ? iiop1->object_key : obj1->object_key;
		objkey2 = iiop2->object_key ? iiop2->object_key : obj2->object_key;

		if (!IOP_ObjectKey_equal (objkey1, objkey2))
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
			ORBit_ObjectKey                  *objkey1, *objkey2;

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

			objkey1 = c1->object_key ? c1->object_key : obj1->object_key;
			objkey2 = c2->object_key ? c2->object_key : obj2->object_key;

			if (!IOP_ObjectKey_equal (objkey1, objkey2))
				return FALSE;
		}
		break;
	}

	case IOP_TAG_ORBIT_SPECIFIC: {
		IOP_TAG_ORBIT_SPECIFIC_info *os1 = d1;
		IOP_TAG_ORBIT_SPECIFIC_info *os2 = d2;
		ORBit_ObjectKey             *objkey1, *objkey2;

		if (os1->ipv6_port != os2->ipv6_port)
			return FALSE;

		objkey1 = os1->object_key ? os1->object_key : obj1->object_key;
		objkey2 = os2->object_key ? os2->object_key : obj2->object_key;

		if (!IOP_ObjectKey_equal (objkey1, objkey2))
			return FALSE;
		if (strcmp (os1->unix_sock_path, os2->unix_sock_path))
			return FALSE;
		break;
	}
	case IOP_TAG_MULTIPLE_COMPONENTS: {
		static int warned = 0;
		if (!(warned++)) /* FIXME: */
			g_warning ("IOP_profile_equal: no multiple "
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

static guint
IOP_mem_hash (gconstpointer key, gulong len)
{
  const char *p, *pend;
  guint h = 0;

  for(p = key, pend = p + len; p < pend; p++)
    h = (h << 5) - h + *p;

  return h;
}

void
IOP_profile_hash(gpointer item, gpointer data)
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
      *h ^= IOP_mem_hash(upi->data._buffer, upi->data._length);
      break;
    }
}

void
IOP_delete_profiles (GSList **profiles)
{
  if (profiles && *profiles && (*profiles != common_profiles)) {
    g_slist_foreach (*profiles, (GFunc)IOP_profile_free, NULL);
    g_slist_free (*profiles);
    *profiles = NULL;
  }
}

void
IOP_generate_profiles (CORBA_Object obj)
{
  IOP_TAG_MULTIPLE_COMPONENTS_info *mci = NULL;
  IOP_TAG_ORBIT_SPECIFIC_info      *osi = NULL;
  IOP_TAG_INTERNET_IOP_info        *iiop = NULL;
  gboolean                          need_objkey_component;
  ORBit_OAObject                    adaptor_obj = obj->adaptor_obj;
  GSList                           *ltmp;

  g_assert( obj && (obj->profile_list == NULL) );

  /*
   * no need to have any listening sockets until now.
   * if the ORB has been shutdown and restarted,
   * the profiles must be regenerated.
   */
  if (!obj->orb->servers) {
    ORBit_start_servers (obj->orb);
    if (common_profiles) {
      g_slist_foreach (common_profiles, (GFunc)IOP_profile_free, NULL);
      g_slist_free (common_profiles);
      common_profiles = NULL;
    }
  }

  if (!obj->object_key && adaptor_obj)
    obj->object_key = ORBit_OAObject_object_to_objkey (adaptor_obj);

  /* share the profiles between everyone. */
  if (common_profiles) {
    obj->profile_list = common_profiles;
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
              common_profiles = g_slist_append(common_profiles, iiop);
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

          for(giop = NULL, ltmp2 = common_profiles; ltmp2; ltmp2 = ltmp2->next)
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
              common_profiles = g_slist_append(common_profiles, giop);
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
        }
      need_objkey_component = TRUE;
    }

  if (osi)
    common_profiles = g_slist_append (common_profiles, osi);

  /* We always create this to marshal the TAG_CODE_SET component */
  mci = g_new0 (IOP_TAG_MULTIPLE_COMPONENTS_info, 1);
  mci->parent.profile_type = IOP_TAG_MULTIPLE_COMPONENTS;

  if (need_objkey_component) {
    IOP_TAG_COMPLETE_OBJECT_KEY_info *coki;

    coki = g_new0 (IOP_TAG_COMPLETE_OBJECT_KEY_info, 1);
    coki->parent.component_type = IOP_TAG_COMPLETE_OBJECT_KEY;
    coki->object_key = NULL; /* Sucks in the object's key at marshal time */
    mci->components = g_slist_append (mci->components, coki);
  }

  {
    IOP_TAG_CODE_SETS_info *csets;

    csets = g_new0 (IOP_TAG_CODE_SETS_info, 1);
    csets->parent.component_type = IOP_TAG_CODE_SETS;
    mci->components = g_slist_append (mci->components, csets);
  }

  common_profiles = g_slist_append (common_profiles, mci);

  obj->profile_list = common_profiles;
  ORBit_register_objref (obj);
}

/*
 * 'freedom' routines.
 */

static void
IOP_component_free (IOP_Component_info *c)
{
	switch (c->component_type) {
	case IOP_TAG_GENERIC_SSL_SEC_TRANS:
		g_free (((IOP_TAG_GENERIC_SSL_SEC_TRANS_info *)c)->service);
		break;
	case IOP_TAG_COMPLETE_OBJECT_KEY: {
		IOP_TAG_COMPLETE_OBJECT_KEY_info *coki = 
			(IOP_TAG_COMPLETE_OBJECT_KEY_info *)c;

		if (coki->object_key)
			ORBit_free_T (coki->object_key);
		coki->object_key = NULL;

		break;
		}
	case IOP_TAG_SSL_SEC_TRANS:
		break;
	case IOP_TAG_CODE_SETS:
		break;
	default:
		g_free (((IOP_UnknownProfile_info*)c)->data._buffer);
		break;
	}

	g_free(c);
}

static void
IOP_components_free (GSList *components)
{
	g_slist_foreach (components, (GFunc)IOP_component_free, NULL);
}

static void
IOP_TAG_MULTIPLE_COMPONENTS_free (IOP_Profile_info *p)
{
	IOP_TAG_MULTIPLE_COMPONENTS_info *info = (IOP_TAG_MULTIPLE_COMPONENTS_info *)p;

	IOP_components_free (info->components);
}

static void
IOP_TAG_INTERNET_IOP_free (IOP_Profile_info *p)
{
	IOP_TAG_INTERNET_IOP_info *info = (IOP_TAG_INTERNET_IOP_info *)p;

	IOP_components_free (info->components);
	g_free (info->host);

	if (info->object_key)
		ORBit_free_T (info->object_key);
	info->object_key = NULL;
}

static void
IOP_TAG_GENERIC_IOP_free (IOP_Profile_info *p)
{
	IOP_TAG_GENERIC_IOP_info *info = (IOP_TAG_GENERIC_IOP_info *)p;

	IOP_components_free (info->components);
	g_free (info->proto);
	g_free (info->host);
	g_free (info->service);
}

static void
IOP_TAG_ORBIT_SPECIFIC_free (IOP_Profile_info *p)
{
	IOP_TAG_ORBIT_SPECIFIC_info *info = (IOP_TAG_ORBIT_SPECIFIC_info *)p;

	g_free (info->unix_sock_path);

	if (info->object_key)
		ORBit_free_T (info->object_key);
	info->object_key = NULL;
}

static void
IOP_UnknownProfile_free (IOP_Profile_info *p)
{
	IOP_UnknownProfile_info *info = (IOP_UnknownProfile_info *)p;

	g_free (info->data._buffer);
}

static void
IOP_profile_free (IOP_Profile_info *p)
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

/*
 * marshalling routines.
 */

static void
IOP_ObjectKey_marshal (CORBA_Object                obj,
		       GIOPSendBuffer             *buf,
		       ORBit_ObjectKey            *objkey)
{
	if (!objkey)
		objkey = obj->object_key;

	giop_send_buffer_append_aligned (buf, &objkey->_length, 4);

	giop_send_buffer_append (buf, objkey->_buffer, objkey->_length);
}

static void
IOP_TAG_GENERIC_SSL_SEC_TRANS_marshal(CORBA_Object obj, GIOPSendBuffer *buf,
				      IOP_Component_info *ci)
{
  CORBA_unsigned_long len;
  IOP_TAG_GENERIC_SSL_SEC_TRANS_info *ssli = (IOP_TAG_GENERIC_SSL_SEC_TRANS_info *)ci;

  len = strlen(ssli->service) + 1;
  giop_send_buffer_append_aligned(buf, &len, 4);
  giop_send_buffer_append(buf, ssli->service, 4);
}

static void
IOP_TAG_SSL_SEC_TRANS_marshal(CORBA_Object obj, GIOPSendBuffer *buf,
			      IOP_Component_info *ci)
{
  IOP_TAG_SSL_SEC_TRANS_info *ssli = (IOP_TAG_SSL_SEC_TRANS_info *) ci;

  giop_send_buffer_align (buf, 4);
  giop_send_buffer_append (buf, &ssli->target_supports, 10);
}

static void
IOP_TAG_COMPLETE_OBJECT_KEY_marshal(CORBA_Object obj, GIOPSendBuffer *buf,
				    IOP_Component_info *ci)
{
  IOP_TAG_COMPLETE_OBJECT_KEY_info *coki = (IOP_TAG_COMPLETE_OBJECT_KEY_info *) ci;

  IOP_ObjectKey_marshal (obj, buf, coki->object_key);
}

static void
CodeSetComponent_marshal (GIOPSendBuffer *buf,
			  CORBA_unsigned_long native_code_set,
			  CORBA_sequence_CORBA_unsigned_long *opt_conversion_code_sets)
{
	/* native_code_set */
	giop_send_buffer_append_aligned (buf, &native_code_set, 4);

	if (opt_conversion_code_sets)
		g_error ("Unimplemented as yet");
	else {
		CORBA_unsigned_long length = 0;
		giop_send_buffer_append_aligned (buf, &length, 4);
	}	
}

/* we always marshal the same thing: see 13.7.2.4 */
static void
IOP_TAG_CODE_SETS_marshal(CORBA_Object obj, GIOPSendBuffer *buf,
			  IOP_Component_info *ci)
{
	/* To get these magic numbers see the 'OSF Character
	   and Codeset Registry'; ftp.opengroup.org/pub/code_set_registry */
	CORBA_unsigned_long utf8_key  = 0x05010001;
	CORBA_unsigned_long utf16_key = 0x00010109;

	/* Marshal a CodeSetComponentInfo structure */
	CodeSetComponent_marshal (buf, utf8_key, NULL);
	CodeSetComponent_marshal (buf, utf16_key, NULL);
}

static void
IOP_UnknownComponent_marshal(CORBA_Object obj, GIOPSendBuffer *buf, 
			     IOP_Component_info *ci)
{
  IOP_UnknownComponent_info *uci = (IOP_UnknownComponent_info *)ci;

  giop_send_buffer_append(buf, uci->data._buffer, uci->data._length);
}

static void
IOP_components_marshal (CORBA_Object obj, 
			GIOPSendBuffer *buf,
			GSList *components)
{
	CORBA_unsigned_long  len;
	GSList              *cur;
	guchar              *marker;

	len = g_slist_length (components);
	giop_send_buffer_append_aligned (buf, &len, 4);

	for (cur = components; cur; cur = cur->next) {
		IOP_Component_info *ci = cur->data;

		giop_send_buffer_align (buf, 4);
		giop_send_buffer_append (buf, &ci->component_type, 4);

		switch (ci->component_type) {
		case IOP_TAG_GENERIC_SSL_SEC_TRANS:
		case IOP_TAG_SSL_SEC_TRANS:
			marker = giop_send_buffer_append_aligned (buf, &len, 4);
			len = buf->msg.header.message_size;
			giop_send_buffer_append (buf, &buf->msg.header.flags, 1);
			break;
		default:
			marker = NULL;
			break;
		}

		switch (ci->component_type) {
		case IOP_TAG_GENERIC_SSL_SEC_TRANS:
			IOP_TAG_GENERIC_SSL_SEC_TRANS_marshal (obj, buf, ci);
			break;
		case IOP_TAG_SSL_SEC_TRANS:
			IOP_TAG_SSL_SEC_TRANS_marshal (obj, buf, ci);
			break;
		case IOP_TAG_COMPLETE_OBJECT_KEY:
			IOP_TAG_COMPLETE_OBJECT_KEY_marshal (obj, buf, ci);
			break;
		case IOP_TAG_CODE_SETS:
			IOP_TAG_CODE_SETS_marshal (obj, buf, ci);
			break;
		default:
			IOP_UnknownComponent_marshal (obj, buf, ci);
			break;
		}

		if (marker) {
			len = buf->msg.header.message_size - len;
			memcpy (marker, &len, 4);
		}
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
  giop_send_buffer_append_aligned(buf, &len, 4);
  giop_send_buffer_append(buf, iiop->host, len);

  giop_send_buffer_align(buf, 2);
  giop_send_buffer_append(buf, &iiop->port, 2);

  IOP_ObjectKey_marshal(obj, buf, iiop->object_key ? iiop->object_key : obj->object_key);

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
  giop_send_buffer_append_aligned(buf, &len, 4);
  giop_send_buffer_append(buf, giop->proto, len);

  len = strlen(giop->host) + 1;
  giop_send_buffer_append_aligned(buf, &len, 4);
  giop_send_buffer_append(buf, giop->host, len);

  len = strlen(giop->service) + 1;
  giop_send_buffer_append_aligned(buf, &len, 4);
  giop_send_buffer_append(buf, giop->service, len);

  IOP_components_marshal(obj, buf, giop->components);
}

static void
IOP_TAG_MULTIPLE_COMPONENTS_marshal (CORBA_Object      obj,
				     GIOPSendBuffer   *buf,
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
  giop_send_buffer_append_aligned(buf, &len, 4);
  giop_send_buffer_append(buf, osi->unix_sock_path, len);
  giop_send_buffer_align(buf, 2);
  giop_send_buffer_append(buf, &osi->ipv6_port, 2);
  IOP_ObjectKey_marshal(obj, buf, osi->object_key ? osi->object_key : obj->object_key);
}

static void
IOP_UnknownProfile_marshal(CORBA_Object obj, GIOPSendBuffer *buf, 
			   IOP_Profile_info *pi)
{
  IOP_UnknownProfile_info *upi = (IOP_UnknownProfile_info *)pi;

  giop_send_buffer_append(buf, upi->data._buffer, upi->data._length);
}

void
IOP_profile_marshal (CORBA_Object obj, GIOPSendBuffer *buf, gpointer *p)
{
	IOP_Profile_info    *profile = (IOP_Profile_info *)p;
	CORBA_unsigned_long  seqlen, msgsz;
	guchar              *marker;

	giop_send_buffer_append_aligned (buf, &profile->profile_type, 4);
	marker = giop_send_buffer_append_aligned (buf, NULL, 4);

	msgsz = buf->msg.header.message_size;

	switch (profile->profile_type) {
	case IOP_TAG_INTERNET_IOP:
		giop_send_buffer_append (buf, &buf->msg.header.flags, 1);
		IOP_TAG_INTERNET_IOP_marshal (obj, buf, profile);
		break;
	case IOP_TAG_ORBIT_SPECIFIC:
		giop_send_buffer_append (buf, &buf->msg.header.flags, 1);
		IOP_TAG_ORBIT_SPECIFIC_marshal (obj, buf, profile);
		break;
	case IOP_TAG_GENERIC_IOP:
		giop_send_buffer_append (buf, &buf->msg.header.flags, 1);
		IOP_TAG_GENERIC_IOP_marshal (obj, buf, profile);
		break;
	case IOP_TAG_MULTIPLE_COMPONENTS:
		giop_send_buffer_append (buf, &buf->msg.header.flags, 1);
		IOP_TAG_MULTIPLE_COMPONENTS_marshal (obj, buf, profile);
		break;
	default:
		IOP_UnknownProfile_marshal (obj, buf, profile);
		break;
	}

	seqlen = buf->msg.header.message_size - msgsz;
	memcpy (marker, &seqlen, 4);
}

/*
 * demarshalling routines.
 */

/* FIXME: WTF ! */
static GIOPRecvBuffer *gbuf = NULL;

static ORBit_ObjectKey*
IOP_ObjectKey_demarshal (GIOPRecvBuffer *buf)
{
	ORBit_ObjectKey     *objkey;
	CORBA_unsigned_long  len;

	buf->cur = ALIGN_ADDRESS (buf->cur, 4);
	if ((buf->cur + 4) > buf->end)
		return NULL;

	len = *(CORBA_unsigned_long *)buf->cur;

	if (giop_msg_conversion_needed (buf))
		len = GUINT32_SWAP_LE_BE (len);

	buf->cur += 4;

	if ((buf->cur + len) > buf->end ||
	    (buf->cur + len) < buf->cur)
		return NULL;

	len = ALIGN_VALUE(len, 4);

	objkey           = CORBA_sequence_CORBA_octet__alloc ();
	objkey->_length  = objkey->_maximum = len;
	objkey->_buffer  = CORBA_sequence_CORBA_octet_allocbuf (objkey->_length);
	objkey->_release = CORBA_TRUE;

	memcpy (objkey->_buffer, buf->cur, len);

	buf->cur += len;

	return objkey;
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
IOP_TAG_COMPLETE_OBJECT_KEY_demarshal (IOP_ComponentId id,
				       GIOPRecvBuffer *buf)
{
	IOP_TAG_COMPLETE_OBJECT_KEY_info *retval;
	ORBit_ObjectKey                  *objkey;
  
	objkey = IOP_ObjectKey_demarshal (buf);
	if(!objkey)
		return NULL;

	retval = g_new (IOP_TAG_COMPLETE_OBJECT_KEY_info, 1);
	retval->parent.component_type = id;
	retval->object_key            = objkey;

	return (IOP_Component_info *)retval;
}

static gboolean
CodeSetComponent_demarshal (GIOPRecvBuffer *buf,
			    CORBA_unsigned_long *native_code_set,
			    CORBA_sequence_CORBA_unsigned_long **opt_conversion_code_sets)
{
	CORBA_unsigned_long sequence_length;

	buf->cur = ALIGN_ADDRESS (buf->cur, 4);

	if (buf->cur + 8 > buf->end)
		return FALSE;

	*native_code_set = *(CORBA_unsigned_long *)buf->cur;
	buf->cur += 4;
	sequence_length = *(CORBA_unsigned_long *)buf->cur;
	buf->cur += 4;

	if (sequence_length > 0) {
		static int warned = 0;
		if (!(warned++))
			g_warning ("Ignoring incoming code_sets component");

		if (buf->cur + sequence_length * 4 < buf->end)
			return FALSE;
		else
			buf->cur += sequence_length * 4;
	}
	
	return TRUE;
}

static IOP_Component_info *
IOP_TAG_CODE_SETS_demarshal(IOP_ComponentId id, GIOPRecvBuffer *buf)
{
  IOP_TAG_CODE_SETS_info *retval;
  CORBA_unsigned_long    dummy;
  
  retval = g_new (IOP_TAG_CODE_SETS_info, 1);
  retval->parent.component_type = id;

  /* We don't care about the data much */
  if (!CodeSetComponent_demarshal (buf, &dummy, NULL) ||
      !CodeSetComponent_demarshal (buf, &dummy, NULL)) {
	  g_free (retval);
	  return NULL;
  }

  return (IOP_Component_info *) retval;
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
	case IOP_TAG_CODE_SETS:
          c = IOP_TAG_CODE_SETS_demarshal(cid, buf);
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
  retval->object_key = IOP_ObjectKey_demarshal(buf);
  if(!retval->object_key)
    goto errout;

  giop_recv_buffer_unuse(buf);
  return (IOP_Profile_info *)retval;

 errout:
  if(retval)
    {
      CORBA_free(retval->object_key);
      g_free(retval->unix_sock_path);
      g_free(retval);
    }
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
  if(buf && !IOP_components_demarshal(buf, &components))
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
  retval->object_key = IOP_ObjectKey_demarshal(buf);
  if(!retval->object_key)
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
      CORBA_free(retval->object_key);

      g_free(retval);
    }

  eo2:
  giop_recv_buffer_unuse(buf);
  return NULL;
}

static IOP_Profile_info *
IOP_profile_demarshal(GIOPRecvBuffer *buf, CORBA_ORB orb)
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

gboolean
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
      profile = IOP_profile_demarshal(buf, orb);
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
  IOP_delete_profiles(&profiles);
  return TRUE;
}
