#include <ctype.h>
#include <orbit/orbit.h>
#include <string.h>
#include "../orbit-init.h"
#include "../poa/orbit-poa-export.h"
#include "orbhttp.h"

static void
CORBA_ORB_release_fn(ORBit_RootObject robj)
{
  CORBA_ORB orb = (CORBA_ORB)robj;

  g_ptr_array_free(orb->poas, TRUE);

  g_free(orb);
}


static void
ORBit_locks_initialize(void)
{
  O_MUTEX_INIT(ORBit_RootObject_lifecycle_lock);
}

CORBA_ORB
CORBA_ORB_init(int *argc, char **argv, CORBA_ORBid orb_identifier,
	       CORBA_Environment *ev)
{
  static CORBA_ORB retval = NULL;
  LINCProtocolInfo *info;
  static ORBit_RootObject_Interface orb_if = {
    ORBIT_ROT_ORB,
    CORBA_ORB_release_fn
  };

  if(retval)
    return (CORBA_ORB)CORBA_Object_duplicate((CORBA_Object)retval, ev);

  giop_init();

#ifdef ORBIT_THREADSAFE
  ORBit_locks_initialize();
#endif  

  retval = g_new0(struct CORBA_ORB_type, 1);

  ORBit_RootObject_init((ORBit_RootObject)retval, &orb_if);

  ORBit_genrand_init(&retval->genrand);
  retval->default_giop_version = GIOP_LATEST;

  for(info = linc_protocol_all(); info->name; info++)
    {
      GIOPServer *server;
      LINCConnectionOptions options = 0;

#ifndef ORBIT_THREADED
      options |= LINC_CONNECTION_NONBLOCKING;
#endif
      server = giop_server_new(retval->default_giop_version,
			       info->name, NULL, NULL,
			       options, retval);
      if(server)
	{
	  retval->servers = g_slist_prepend(retval->servers, server);
	  if(!(info->flags & LINC_PROTOCOL_SECURE))
	    {
	      server = giop_server_new(retval->default_giop_version, info->name,
				       NULL, NULL, LINC_CONNECTION_SSL, retval);
	      if(server)
		retval->servers = g_slist_prepend(retval->servers, server);
	    }
	}
    }

  retval->poas = g_ptr_array_new();
  ORBit_init_internals(retval, ev);

  return retval;
}

CORBA_char *
CORBA_ORB_object_to_string(CORBA_ORB _obj,
			   const CORBA_Object obj,
			   CORBA_Environment * ev)
{
  GIOPSendBuffer *buf;
  CORBA_octet endianness = GIOP_FLAG_ENDIANNESS;
  CORBA_char *out;
  int i, j, k;

  g_return_val_if_fail(ev, NULL);

  if(!obj || !_obj
     || ORBIT_ROOT_OBJECT_TYPE(obj) != ORBIT_ROT_OBJREF)
    {
      CORBA_exception_set_system(ev,
				 ex_CORBA_BAD_PARAM,
				 CORBA_COMPLETED_NO);
      return NULL;
    }

  buf = giop_send_buffer_use(_obj->default_giop_version);
  buf->header_size = 0;
  g_assert(buf->num_used == 1);
  buf->num_used = 0; /* we don't want the header in there */
  giop_send_buffer_append(buf, &endianness, 1);
  ORBit_marshal_object(buf, obj);
  out = CORBA_string_alloc(4 + (buf->msg.header.message_size * 2) + 1);

  strcpy(out, "IOR:");
  k = 4;
  for(i = 0; i < buf->num_used; i++)
    {
      struct iovec *curvec = &buf->iovecs[i];
      guchar *ptr = curvec->iov_base;

      for(j = 0; j < curvec->iov_len; j++, ptr++)
	{
	  int n1 = (*ptr & 0xF0) >> 4, n2 = (*ptr & 0xF);
	  out[k++] = num2hexdigit(n1);
	  out[k++] = num2hexdigit(n2);
	}
    }
  out[k++] = '\0';
  
  giop_send_buffer_unuse(buf);

  return out;
}

CORBA_Object
CORBA_ORB_string_to_object(CORBA_ORB _obj,
			  const CORBA_char * str,
			  CORBA_Environment * ev)
{
  CORBA_Object retval = CORBA_OBJECT_NIL;
  GIOPRecvBuffer *buf;
  CORBA_unsigned_long len;
  int i;
  char *tbuf;
  char *realstr = NULL;

  if(strstr(str, "://"))
    {
      realstr = orb_http_resolve(str);
      if(!realstr)
	goto out;
      str = realstr;
    }
  if(strncmp(str, "IOR:", 4))
    {
      CORBA_exception_set_system(ev,
				 ex_CORBA_BAD_PARAM,
				 CORBA_COMPLETED_NO);
      goto out;
    }

  str += 4;
  len = strlen(str);
  while(len > 0 && !isxdigit(str[len-1])) len--;
  if(len % 2)
    goto out;

  tbuf = g_alloca(len / 2);
#define HEXDIGIT(c) (isdigit((guchar)(c))?(c)-'0':tolower((guchar)(c))-'a'+10)
#define HEXOCTET(a,b) ((HEXDIGIT((a)) << 4) | HEXDIGIT((b)))

  for(i = 0; i < len; i += 2)
    tbuf[i/2] = HEXOCTET(str[i],str[i+1]);

  buf = giop_recv_buffer_use_encaps(tbuf, len / 2);

  if(ORBit_demarshal_object(&retval, buf, _obj))
    {
      CORBA_exception_set_system(ev,
				 ex_CORBA_MARSHAL,
				 CORBA_COMPLETED_NO);
      retval = CORBA_OBJECT_NIL;
    }

  giop_recv_buffer_unuse(buf);

 out:
  if(realstr)
    g_free(realstr);

  return retval;
}

void
CORBA_ORB_create_list(CORBA_ORB _obj, const CORBA_long count,
		      CORBA_NVList * new_list,
		      CORBA_Environment * ev)
{
}

void
CORBA_ORB_create_operation_list(CORBA_ORB _obj,
				const CORBA_OperationDef oper,
				CORBA_NVList * new_list,
				CORBA_Environment * ev)
{
}

void
CORBA_ORB_send_multiple_requests_oneway(CORBA_ORB _obj,
					const CORBA_RequestSeq * req,
					CORBA_Environment * ev)
{
}

void
CORBA_ORB_send_multiple_requests_deferred(CORBA_ORB _obj,
					  const CORBA_RequestSeq *
					  req,
					  CORBA_Environment * ev)
{
}

CORBA_boolean
CORBA_ORB_poll_next_response(CORBA_ORB _obj,
			     CORBA_Environment * ev)
{
  return CORBA_FALSE;
}

void
CORBA_ORB_get_next_response(CORBA_ORB _obj, CORBA_Request * req,
			    CORBA_Environment * ev)
{
  *req = NULL;
}

CORBA_boolean
CORBA_ORB_get_service_information(CORBA_ORB _obj,
				  const CORBA_ServiceType service_type,
				  CORBA_ServiceInformation **service_information,
				  CORBA_Environment * ev)
{
  return CORBA_FALSE;
}

static void
servlist_add_id(gpointer key, gpointer value, gpointer data)
{
  CORBA_ORB_ObjectIdList *retval = data;

  retval->_buffer[retval->_maximum++] = CORBA_string_dup(key);
}

CORBA_ORB_ObjectIdList *
CORBA_ORB_list_initial_services(CORBA_ORB _obj,
				CORBA_Environment *ev)
{
  CORBA_ORB_ObjectIdList *retval;
  retval = CORBA_ORB_ObjectIdList__alloc();
  if(_obj->initial_refs)
    {
      retval->_length = g_hash_table_size(_obj->initial_refs);
      retval->_maximum = 0;
      retval->_buffer =
	CORBA_sequence_CORBA_ORB_ObjectId_allocbuf(retval->_length);
      g_hash_table_foreach(_obj->initial_refs, servlist_add_id, retval);
    }
  else
    {
      retval->_length = 0;
      retval->_buffer = NULL;
    }
  return retval;
}

CORBA_Object
CORBA_ORB_resolve_initial_references(CORBA_ORB _obj,
				     const CORBA_ORB_ObjectId identifier,
				     CORBA_Environment * ev)
{
  ORBit_InitialReference *val;
  if(!_obj->initial_refs)
    return CORBA_OBJECT_NIL;
  val = g_hash_table_lookup(_obj->initial_refs, identifier);

  if(val)
    return CORBA_Object_duplicate(val->objref, ev);

  return CORBA_OBJECT_NIL;
}

static CORBA_TypeCode
CORBA_TypeCode_allocate(void)
{
  CORBA_TypeCode tc = g_new0(struct CORBA_TypeCode_struct,1);
  ORBit_RootObject_init(&tc->parent, &ORBit_TypeCode_epv);
  return ORBit_RootObject_duplicate(tc);
}

CORBA_TypeCode
CORBA_ORB_create_struct_tc(CORBA_ORB _obj,
			   const CORBA_RepositoryId id,
			   const CORBA_Identifier name,
			   const CORBA_StructMemberSeq *members,
			   CORBA_Environment * ev)
{
  CORBA_TypeCode tc;
  int i;

  tc=CORBA_TypeCode_allocate();
  if(tc == NULL)
    goto tc_alloc_failed;

  tc->subtypes = g_new0(CORBA_TypeCode, members->_length);
  if(tc->subtypes == NULL)
    goto subtypes_alloc_failed;

  tc->subnames = g_new0(const char *, members->_length);
  if(tc->subnames == NULL)
    goto subnames_alloc_failed;

  tc->kind = CORBA_tk_struct;
  tc->name = g_strdup(name);
  tc->repo_id = g_strdup(id);
  tc->sub_parts = members->_length;
  tc->length = members->_length;

  for(i = 0; i<members->_length; i++) {
    CORBA_StructMember *mem = (CORBA_StructMember *)&(members->_buffer[i]);

    g_assert(&(mem->type) != NULL);

    tc->subtypes[i] = ORBit_RootObject_duplicate(mem->type);
    tc->subnames[i] = g_strdup(mem->name);
  }

  return(tc);

 subnames_alloc_failed:
  g_free(tc->subtypes);
 subtypes_alloc_failed:
  ORBit_RootObject_release(tc);
 tc_alloc_failed:
  CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
  return NULL;
}

CORBA_TypeCode
CORBA_ORB_create_union_tc(CORBA_ORB _obj,
			  const CORBA_RepositoryId id,
			  const CORBA_Identifier name,
			  const CORBA_TypeCode
			  discriminator_type,
			  const CORBA_UnionMemberSeq *
			  members, CORBA_Environment * ev)
{
  CORBA_TypeCode tc;
  int i;

  tc=CORBA_TypeCode_allocate();

  if(tc == NULL)
    goto tc_alloc_failed;

  tc->discriminator = ORBit_RootObject_duplicate(discriminator_type);
		
  tc->subtypes=g_new0(CORBA_TypeCode, members->_length);
  if(tc->subtypes==NULL)
    goto subtypes_alloc_failed;

  tc->subnames=g_new0(const char *, members->_length);
  if(tc->subnames==NULL)
    goto subnames_alloc_failed;

  tc->sublabels=g_new0(CORBA_any, members->_length);
  if(tc->sublabels == NULL)
    goto sublabels_alloc_failed;

  tc->kind=CORBA_tk_union;
  tc->name=g_strdup(name);
  tc->repo_id=g_strdup(id);
  tc->sub_parts=members->_length;
  tc->length=members->_length;
  tc->default_index=-1;

  for(i=0;i<members->_length;i++) {
    CORBA_UnionMember *mem=(CORBA_UnionMember *)&(members->_buffer[i]);

    g_assert(&(mem->label)!=NULL);
    memcpy(&(tc->sublabels[i]), &(mem->label), (size_t)sizeof(CORBA_any));
    g_assert(&(mem->type)!=NULL);
    tc->subtypes[i] = ORBit_RootObject_duplicate(mem->type);
    tc->subnames[i]=g_strdup(mem->name);

    if(mem->label._type->kind==CORBA_tk_octet) {
      tc->default_index=i;
    }
  }

  return(tc);

 sublabels_alloc_failed:
  g_free(tc->sublabels);
 subnames_alloc_failed:
  g_free(tc->subtypes);
 subtypes_alloc_failed:
  ORBit_free(tc->discriminator);
  ORBit_RootObject_release(tc);
 tc_alloc_failed:
  CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
  return NULL;
}

CORBA_TypeCode
CORBA_ORB_create_enum_tc(CORBA_ORB _obj,
			 const CORBA_RepositoryId id,
			 const CORBA_Identifier name,
			 const CORBA_EnumMemberSeq *
			 members, CORBA_Environment * ev)
{
  CORBA_TypeCode tc;
  int i;

  tc = CORBA_TypeCode_allocate();
  if(tc == NULL)
    goto tc_alloc_failed;

  tc->subnames=g_new0(const char *, members->_length);
  if(tc->subnames==NULL)
    goto subnames_alloc_failed;

  tc->kind = CORBA_tk_enum;
  tc->name = g_strdup(name);
  tc->repo_id = g_strdup(id);
  tc->sub_parts = members->_length;
  tc->length = members->_length;

  for(i=0;i<members->_length;i++)
    tc->subnames[i]=g_strdup(members->_buffer[i]);

  return(tc);

 subnames_alloc_failed:
  ORBit_RootObject_release(tc);
 tc_alloc_failed:
  CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
  return(NULL);
}

CORBA_TypeCode
CORBA_ORB_create_alias_tc(CORBA_ORB _obj,
			  const CORBA_RepositoryId id,
			  const CORBA_Identifier name,
			  const CORBA_TypeCode
			  original_type,
			  CORBA_Environment * ev)
{
  CORBA_TypeCode tc;

  tc = CORBA_TypeCode_allocate();
  if(tc==NULL)
    goto tc_alloc_failed;
	
	/* Can't use chunks here, because it's sometimes an array. Doh! */
  tc->subtypes=g_new0(CORBA_TypeCode, 1);
  if(tc->subtypes==NULL)
    goto subtypes_alloc_failed;

  tc->kind=CORBA_tk_alias;
  tc->name=g_strdup(name);
  tc->repo_id=g_strdup(id);
  tc->sub_parts=1;
  tc->length=1;

  tc->subtypes[0] = ORBit_RootObject_duplicate(original_type);

  return(tc);
 subtypes_alloc_failed:
  ORBit_RootObject_release(tc);
 tc_alloc_failed:
  CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
  return NULL;
}

CORBA_TypeCode
CORBA_ORB_create_exception_tc(CORBA_ORB _obj,
			      const CORBA_RepositoryId id,
			      const CORBA_Identifier name,
			      const CORBA_StructMemberSeq *
			      members,
			      CORBA_Environment * ev)
{
  CORBA_TypeCode tc;
  int i;

  tc=CORBA_TypeCode_allocate();
  if(tc==NULL)
    goto tc_alloc_failed;

  if (members->_length == 0)
    {
      tc->subtypes = NULL;
      tc->subnames = NULL;
    }
  else
    {
      tc->subtypes=g_new0(CORBA_TypeCode, members->_length);
      if(tc->subtypes==NULL)
	goto subtypes_alloc_failed;

      tc->subnames=g_new0(const char *, members->_length);
      if(tc->subnames==NULL)
	goto subnames_alloc_failed;
    }

  tc->kind=CORBA_tk_except;
  tc->name=g_strdup(name);
  tc->repo_id=g_strdup(id);
  tc->sub_parts=members->_length;
  tc->length=members->_length;

  for(i=0;i<members->_length;i++) {
    CORBA_StructMember *mem=(CORBA_StructMember *)&(members->_buffer[i]);

    g_assert(mem->type != NULL);
    tc->subtypes[i] = ORBit_RootObject_duplicate(mem->type);
    tc->subnames[i]=g_strdup(mem->name);
  }

  return(tc);

 subnames_alloc_failed:
  g_free(tc->subtypes);
 subtypes_alloc_failed:
  ORBit_RootObject_release(tc);
 tc_alloc_failed:
  CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
  return(NULL);
}

CORBA_TypeCode
CORBA_ORB_create_interface_tc(CORBA_ORB _obj,
			      const CORBA_RepositoryId id,
			      const CORBA_Identifier name,
			      CORBA_Environment * ev)
{
	CORBA_TypeCode tc;

	tc=CORBA_TypeCode_allocate();
	if(tc==NULL) {
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY,
					   CORBA_COMPLETED_NO);
		return(NULL);
	}

	tc->kind=CORBA_tk_objref;
	tc->name=g_strdup(name);
	tc->repo_id=g_strdup(id);

	return(tc);
}

CORBA_TypeCode
CORBA_ORB_create_string_tc(CORBA_ORB _obj,
			   const CORBA_unsigned_long bound,
			   CORBA_Environment * ev)
{
  CORBA_TypeCode tc;

  tc=CORBA_TypeCode_allocate();
  if(tc==NULL)
    {
      CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
      return(NULL);
    }

  tc->kind=CORBA_tk_string;
  tc->length=bound;

  return(tc);
}

CORBA_TypeCode
CORBA_ORB_create_wstring_tc(CORBA_ORB _obj,
			    const CORBA_unsigned_long bound,
			    CORBA_Environment * ev)
{
  CORBA_TypeCode tc;

  tc=CORBA_TypeCode_allocate();
  if(tc==NULL)
    {
      CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
      return(NULL);
    }

  tc->kind=CORBA_tk_wstring;
  tc->length=bound;

  return(tc);
}

CORBA_TypeCode
CORBA_ORB_create_fixed_tc(CORBA_ORB _obj,
			  const CORBA_unsigned_short digits,
			  const CORBA_short scale,
			  CORBA_Environment * ev)
{
  CORBA_TypeCode tc;

  tc=CORBA_TypeCode_allocate();
  if(tc==NULL)
    {
      CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
      return(NULL);
    }

  tc->kind=CORBA_tk_fixed;
  tc->digits=digits;
  tc->scale=scale;

  return(tc);
}

CORBA_TypeCode
CORBA_ORB_create_sequence_tc(CORBA_ORB _obj,
			     const CORBA_unsigned_long
			     bound,
			     const CORBA_TypeCode
			     element_type,
			     CORBA_Environment * ev)
{
  CORBA_TypeCode tc;

  tc=CORBA_TypeCode_allocate();
  if(tc==NULL)
    goto tc_alloc_failed;

  tc->subtypes=g_new0(CORBA_TypeCode, 1);
  if(tc->subtypes==NULL)
    goto subtypes_alloc_failed;

  tc->kind=CORBA_tk_sequence;
  tc->sub_parts=1;
  tc->length=bound;

  tc->subtypes[0] = ORBit_RootObject_duplicate(element_type);

  return(tc);

 subtypes_alloc_failed:
  ORBit_RootObject_release(tc);
 tc_alloc_failed:
  CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
  return(NULL);
}

CORBA_TypeCode
CORBA_ORB_create_recursive_sequence_tc(CORBA_ORB _obj,
				       const CORBA_unsigned_long bound,
				       const CORBA_unsigned_long offset,
				       CORBA_Environment *ev)
{
  CORBA_TypeCode tc;

  tc=CORBA_TypeCode_allocate();
  if(tc==NULL)
    goto tc_alloc_failed;

  tc->subtypes=g_new0(CORBA_TypeCode, 1);
  if(tc->subtypes==NULL)
    goto subtypes_alloc_failed;

  tc->kind=CORBA_tk_sequence;
  tc->sub_parts=1;
  tc->length=bound;

  tc->subtypes[0] = CORBA_TypeCode_allocate();
  tc->subtypes[0]->kind=CORBA_tk_recursive;
  tc->subtypes[0]->recurse_depth=offset;

  return(tc);

 subtypes_alloc_failed:
  ORBit_RootObject_release(tc);
 tc_alloc_failed:
  CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
  return(NULL);
}

CORBA_TypeCode
CORBA_ORB_create_array_tc(CORBA_ORB _obj,
			  const CORBA_unsigned_long length,
			  const CORBA_TypeCode element_type,
			  CORBA_Environment * ev)
{
  CORBA_TypeCode tc;

  tc=CORBA_TypeCode_allocate();
  if(tc==NULL)
    goto tc_alloc_failed;

  tc->subtypes=g_new0(CORBA_TypeCode, 1);
  if(tc->subtypes==NULL)
    goto subtypes_alloc_failed;

  tc->kind=CORBA_tk_array;
  tc->sub_parts=1;
  tc->length=length;

  tc->subtypes[0] = ORBit_RootObject_duplicate(element_type);

  return(tc);

 subtypes_alloc_failed:
  ORBit_RootObject_release(tc);
 tc_alloc_failed:
  CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
  return(NULL);
}

CORBA_TypeCode
CORBA_ORB_create_value_tc(CORBA_ORB _obj,
			  const CORBA_RepositoryId id,
			  const CORBA_Identifier name,
			  const CORBA_ValueModifier type_modifier,
			  const CORBA_TypeCode concrete_base,
			  const CORBA_ValueMemberSeq *members,
			  CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_value_box_tc(CORBA_ORB _obj,
			      const CORBA_RepositoryId id,
			      const CORBA_Identifier name,
			      const CORBA_TypeCode boxed_type,
			      CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_native_tc(CORBA_ORB _obj,
			   const CORBA_RepositoryId id,
			   const CORBA_Identifier name,
			   CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_recursive_tc(CORBA_ORB _obj,
			      const CORBA_RepositoryId id,
			      CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_abstract_interface_tc(CORBA_ORB _obj,
				       const CORBA_RepositoryId id,
				       const CORBA_Identifier name,
				       CORBA_Environment *ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_boolean
CORBA_ORB_work_pending(CORBA_ORB _obj,
		       CORBA_Environment * ev)
{
  return g_main_pending();
}

void
CORBA_ORB_perform_work(CORBA_ORB _obj, CORBA_Environment * ev)
{
  g_main_iteration(FALSE);
}

void
CORBA_ORB_run(CORBA_ORB _obj, CORBA_Environment * ev)
{
  while(1)
    {
      GIOPRecvBuffer *in_buf;

      in_buf = giop_recv_buffer_use();
      if(in_buf->msg.header.message_type == GIOP_REQUEST)
	ORBit_handle_request(_obj, in_buf);
      else
	giop_recv_buffer_unuse(in_buf);
    }
}

void
CORBA_ORB_shutdown(CORBA_ORB orb,
		   const CORBA_boolean wait_for_completion,
		   CORBA_Environment * ev)
{
  if ( g_ptr_array_index(orb->poas, 0) ) /* Root POA */
    {
      PortableServer_POA_destroy( g_ptr_array_index(orb->poas, 0), 
				  TRUE, wait_for_completion, ev);
      if (ev->_major)
	/* This is prob. an INV_ORDER exception */
	return;
    }

  g_slist_foreach(orb->servers, (GFunc)g_object_unref, NULL);
  g_slist_free(orb->servers); orb->servers = NULL;
  giop_connection_remove_by_orb(orb);
}

void
CORBA_ORB_destroy(CORBA_ORB orb, CORBA_Environment * ev)
{
  PortableServer_POA root_poa;

  if ( orb->life_flags & ORBit_LifeF_Destroyed )
    return;

  CORBA_ORB_shutdown(orb, TRUE, ev);
  if ( ev->_major )
    return;
  root_poa = g_ptr_array_index(orb->poas, 0);
  if ( root_poa )
    {
      if ( root_poa->parent.refs != 1 )
	{
	  g_warning("CORBA_ORB_destroy: Application still has %d refs to RootPOA.",
		    root_poa->parent.refs-1);
	}

      ORBit_RootObject_release(root_poa);
      g_ptr_array_index(orb->poas, 0) = NULL;
    }

#ifndef G_DISABLE_ASSERT
  {
    int pi;
    for (pi = 0; pi < orb->poas->len; pi++)
      {
	PortableServer_POA *poa = g_ptr_array_index(orb->poas,pi);
	g_assert(poa == NULL);
      }
  }
#endif
  orb->life_flags |= ORBit_LifeF_Destroyed;
  ORBit_RootObject_release(orb);
}

CORBA_Policy
CORBA_ORB_create_policy(CORBA_ORB _obj,
			const CORBA_PolicyType type,
			const CORBA_any * val,
			CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_ValueFactory
CORBA_ORB_register_value_factory(CORBA_ORB _obj,
				 const CORBA_RepositoryId id,
				 const CORBA_ValueFactory factory,
				 CORBA_Environment *ev)
{
  return CORBA_OBJECT_NIL;
}

void
CORBA_ORB_unregister_value_factory(CORBA_ORB _obj,
				   const CORBA_RepositoryId id,
				   CORBA_Environment * ev)
{
}

CORBA_ValueFactory
CORBA_ORB_lookup_value_factory(CORBA_ORB _obj,
			       const CORBA_RepositoryId id,
			       CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

/* ORBit extension */
void
CORBA_ORB_set_initial_reference(CORBA_ORB orb, CORBA_ORB_ObjectId identifier,
				ORBit_InitialReference *val, CORBA_Environment *ev)
{
  ORBit_InitialReference *findval;
  char *findkey = NULL;

  if(!orb->initial_refs)
    orb->initial_refs = g_hash_table_new(g_str_hash, g_str_equal);

  if(g_hash_table_lookup_extended(orb->initial_refs, identifier,
				  (gpointer *)&findkey, (gpointer *)&findval))
    {
      if(!findval->free_name)
	findkey = NULL;
    }

  g_hash_table_insert(orb->initial_refs,
		      val->free_name?g_strdup(identifier):identifier,
		      val);
  g_free(findkey);
}
