#include <orbit/orbit.h>

CORBA_InterfaceDef
CORBA_Object_get_interface(CORBA_Object _obj,
			   CORBA_Environment * ev)
{
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
}

CORBA_boolean
CORBA_Object_non_existent(CORBA_Object _obj,
			  CORBA_Environment * ev)
{
}

CORBA_boolean
CORBA_Object_is_equivalent(CORBA_Object _obj,
			   const CORBA_Object other_object,
			   CORBA_Environment * ev)
{
}

CORBA_unsigned_long
CORBA_Object_hash(CORBA_Object _obj,
		  const CORBA_unsigned_long maximum,
		  CORBA_Environment * ev)
{
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
}

CORBA_Policy
CORBA_Object_get_policy(CORBA_Object _obj,
			const CORBA_PolicyType policy_type,
			CORBA_Environment * ev)
{
}

CORBA_DomainManagersList *
CORBA_Object_get_domain_managers(CORBA_Object _obj,
				 CORBA_Environment * ev)
{
}

CORBA_Object
CORBA_Object_set_policy_overrides(CORBA_Object _obj,
				  const CORBA_PolicyList *policies,
				  const CORBA_SetOverrideType set_add,
				  CORBA_Environment * ev)
{
}

static void
ORBit_marshal_profile(GIOPSendBuffer *buf, IOP_Profile_info *profile)
{
  CORBA_unsigned_long *seqlen, dumb;
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append(buf, profile->profile_type);
  seqlen = giop_send_buffer_append_indirect(buf, &dumb, 4);
  *seqlen = 0;

  dumb = buf->msg.header.message_size;
  switch(profile->profile_type)
    {
    case IOP_TAG_INTERNET_IOP:
      break;
    case IOP_TAG_ORBIT_SPECIFIC:
      break;
    case IOP_TAG_GENERIC_IOP:
      break;
    case IOP_TAG_MULTIPLE_COMPONENTS:
      break;
    }
  *seqlen = buf->msg.header.message_size - dumb;
}

void
ORBit_marshal_object(GIOPSendBuffer *buf, CORBA_Object obj)
{
  CORBA_unsigned_long type_len, num_profiles;

  giop_send_buffer_align(buf, 4);
  type_len = strlen(obj->type_id) + 1;
  giop_send_buffer_append(buf, &type_len, 4);
  giop_send_buffer_append(buf, obj->type_id, type_len);
  num_profiles = g_slist_length(obj->profile_list);
  giop_send_buffer_align(buf, 4);
  giop_send_buffer_append(buf, &num_profiles, 4);
  
  for(cur = obj->profile_list; cur; cur = cur->next)
    ORBit_marshal_profile(buf, cur->data);
}

static void
ORBit_profile_free(IOP_Profile_info *p)
{
  g_free(p);
}

IOP_Profile_info *
ORBit_demarshal_profile(GIOPRecvBuffer *buf, CORBA_ORB orb)
{
  return NULL;
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
  if((buf->cur + 4) >= buf->end)
    goto errout;
  len = *(CORBA_unsigned_long *)buf->cur;
  buf->cur += 4;
  if((buf->cur + len) >= buf->end
     || (buf->cur + len) < buf->cur)
    goto errout;
  type_id = buf->cur;
  buf->cur += len;
  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) >= buf->end)
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
    }

  return FALSE;

 errout:
  g_slist_foreach(profiles, (GFunc)ORBit_profile_free, NULL);
}
