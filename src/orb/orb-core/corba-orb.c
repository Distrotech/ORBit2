#include <ctype.h>
#include <orbit/orbit.h>
#include <string.h>

static void
CORBA_ORB_release_fn(ORBit_RootObject robj)
{
  CORBA_ORB orb = (CORBA_ORB)robj;

  g_list_foreach(orb->servers, (GFunc)g_object_unref, NULL);
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

  linc_init();
#ifdef ORBIT_THREADSAFE
  ORBit_locks_initialize();
#endif  

  retval = g_new0(struct CORBA_ORB_type, 1);

  ORBit_RootObject_init((ORBit_RootObject)retval, &orb_if);

  retval->default_giop_version = GIOP_LATEST;

  for(info = linc_protocol_all(); info->name; info++)
    {
      GIOPServer *server;

      server = giop_server_new(retval->default_giop_version,
			       info->name, NULL, NULL, 0);
      if(server)
	{
	  retval->servers = g_list_prepend(retval->servers, server);
	  if(!(info->flags & LINC_PROTOCOL_SECURE))
	    {
	      server = giop_server_new(retval->default_giop_version, info->name,
				       NULL, NULL, LINC_CONNECTION_SSL);
	      if(server)
		retval->servers = g_list_prepend(retval->servers, server);
	    }
	}
    }

  retval->poas = g_ptr_array_new();

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

  if(!obj
     || !_obj
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
  CORBA_Object retval;
  GIOPRecvBuffer *buf;
  CORBA_unsigned_long len;
  int i;

  if(!strncmp(str, "IOR:", 4))
    /* XXX raise ex_CORBA_MARSHAL */
    return CORBA_OBJECT_NIL;

  str += 4;
  len = strlen(str);
  while(len > 0 && !isxdigit(str[len-1])) len--;
  if(len % 2)
    /* XXX raise ex_CORBA_MARSHAL */
    return CORBA_OBJECT_NIL;
  buf = giop_recv_buffer_use_encaps(g_alloca(len / 2), len / 2);
  
  buf->msg.header.flags = *(buf->cur++);

#define HEXDIGIT(c) (isdigit((guchar)(c))?(c)-'0':tolower((guchar)(c))-'a'+10)
#define HEXOCTET(a,b) ((HEXDIGIT((a)) << 4) | HEXDIGIT((b)))

  for(i = 0; i < len; i += 2)
    buf->message_body[i/2] = HEXOCTET(str[i],str[i+1]);
  
  if(!ORBit_demarshal_object(&retval, buf, _obj))
    {
      /* XXX raise ex_CORBA_MARSHAL */
      retval = CORBA_OBJECT_NIL;
    }
  giop_recv_buffer_unuse(buf);

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
CORBA_ORB_get_default_context(CORBA_ORB _obj, CORBA_Context * ctx,
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
}

CORBA_boolean
CORBA_ORB_get_service_information(CORBA_ORB _obj,
				  const CORBA_ServiceType
				  service_type,
				  CORBA_ServiceInformation **
				  service_information,
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

CORBA_TypeCode
CORBA_ORB_create_struct_tc(CORBA_ORB _obj,
			   const CORBA_RepositoryId id,
			   const CORBA_Identifier name,
			   const CORBA_StructMemberSeq *members,
			   CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
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
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_enum_tc(CORBA_ORB _obj,
			 const CORBA_RepositoryId id,
			 const CORBA_Identifier name,
			 const CORBA_EnumMemberSeq *
			 members, CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_alias_tc(CORBA_ORB _obj,
			  const CORBA_RepositoryId id,
			  const CORBA_Identifier name,
			  const CORBA_TypeCode
			  original_type,
			  CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_exception_tc(CORBA_ORB _obj,
			      const CORBA_RepositoryId id,
			      const CORBA_Identifier name,
			      const CORBA_StructMemberSeq *
			      members,
			      CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_interface_tc(CORBA_ORB _obj,
			      const CORBA_RepositoryId id,
			      const CORBA_Identifier name,
			      CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_string_tc(CORBA_ORB _obj,
			   const CORBA_unsigned_long bound,
			   CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_wstring_tc(CORBA_ORB _obj,
			    const CORBA_unsigned_long bound,
			    CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_fixed_tc(CORBA_ORB _obj,
			  const CORBA_unsigned_short digits,
			  const CORBA_short scale,
			  CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_sequence_tc(CORBA_ORB _obj,
			     const CORBA_unsigned_long
			     bound,
			     const CORBA_TypeCode
			     element_type,
			     CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_recursive_sequence_tc(CORBA_ORB _obj,
				       const CORBA_unsigned_long bound,
				       const CORBA_unsigned_long offset,
				       CORBA_Environment *ev)
{
  return CORBA_OBJECT_NIL;
}

CORBA_TypeCode
CORBA_ORB_create_array_tc(CORBA_ORB _obj,
			  const CORBA_unsigned_long length,
			  const CORBA_TypeCode element_type,
			  CORBA_Environment * ev)
{
  return CORBA_OBJECT_NIL;
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
  return CORBA_FALSE;
}

void
CORBA_ORB_perform_work(CORBA_ORB _obj, CORBA_Environment * ev)
{
}

void
CORBA_ORB_run(CORBA_ORB _obj, CORBA_Environment * ev)
{
}

void
CORBA_ORB_shutdown(CORBA_ORB _obj,
		   const CORBA_boolean wait_for_completion,
		   CORBA_Environment * ev)
{
}

void
CORBA_ORB_destroy(CORBA_ORB _obj, CORBA_Environment * ev)
{
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
				  &findkey, &findval))
    {
      if(!findval->free_name)
	findkey = NULL;
    }

  g_hash_table_insert(orb->initial_refs,
		      val->free_name?g_strdup(identifier):identifier,
		      val);
  g_free(findkey);
}
