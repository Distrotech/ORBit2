#include "config.h"
#include <orbit/orbit.h>
#include <string.h>

void
ORBit_handle_system_exception(CORBA_Environment *ev,
			      const CORBA_char *nom,
			      CORBA_completion_status status,
			      GIOPRecvBuffer *buf,
			      GIOPSendBuffer *sendbuf)
{
  CORBA_exception_set_system(ev, nom, status);
  giop_recv_buffer_unuse(buf);
  giop_send_buffer_unuse(sendbuf);
}

void
CORBA_exception_set_system(CORBA_Environment *ev,
			   const CORBA_char *except_repos_id,
			   CORBA_completion_status completed)
{
  CORBA_SystemException *se;

  se = CORBA_SystemException__alloc();
  /* I have never seen a case where 'minor' is actually necessary */
  se->minor = 0 /* minor */;
  se->completed = completed;
  CORBA_exception_set(ev, CORBA_SYSTEM_EXCEPTION, except_repos_id, se);
}

void
CORBA_exception_set(CORBA_Environment *ev,
		    CORBA_exception_type major,
		    const CORBA_char *except_repos_id,
		    void *param)
{
  CORBA_exception_free(ev);
  
  ev->_major = major;
  if(major != CORBA_NO_EXCEPTION)
    {
      ev->_id = CORBA_string_dup(except_repos_id);
      ev->_any._type = NULL; /* CORBA sucks */
      ev->_any._value = param;
      ev->_any._release = CORBA_TRUE;
    }
}

CORBA_char *
CORBA_exception_id(CORBA_Environment *ev)
{
  if(ev->_major != CORBA_NO_EXCEPTION)
    return ev->_id;

  return NULL;
}

void *
CORBA_exception_value(CORBA_Environment *ev)
{
  if(ev->_major != CORBA_NO_EXCEPTION)
    return ev->_any._value;

  return NULL;
}

/* An ORBit extension that seems to be perpetuated */
void
CORBA_exception_init(CORBA_Environment *ev)
{
  memset(ev, 0, sizeof(CORBA_Environment));
  ev->_major = CORBA_NO_EXCEPTION;
}

void
CORBA_exception_free(CORBA_Environment *ev)
{
  if(ev->_major != CORBA_NO_EXCEPTION)
    {
      CORBA_free(ev->_id);
      ev->_major = CORBA_NO_EXCEPTION;
      CORBA_any__freekids(&ev->_any, NULL);
      ev->_any._type = NULL;
      ev->_any._value = NULL;
      ev->_any._release = CORBA_FALSE;
    }
}

CORBA_any *
CORBA_exception_as_any(CORBA_Environment *ev)
{
  return &ev->_any;
}

void
ORBit_handle_exception(GIOPRecvBuffer *rb, CORBA_Environment *ev,
		       const ORBit_exception_demarshal_info *ex_info,
		       CORBA_ORB orb)
{
  CORBA_SystemException *new;
  CORBA_unsigned_long len, completion_status, reply_status;
  CORBA_char *my_repoid;

  CORBA_exception_free(ev);

  rb->cur = ALIGN_ADDRESS(rb->cur, sizeof(len));
  if((rb->cur + 4) > rb->end)
    goto errout;
  len = *(CORBA_unsigned_long *)rb->cur;
  rb->cur += 4;
  if(giop_msg_conversion_needed(rb))
    len = GUINT32_SWAP_LE_BE(len);

  if(len)
    {
      my_repoid = rb->cur;
      rb->cur += len;
    }
  else
    my_repoid = NULL;

  reply_status = giop_recv_buffer_reply_status(rb);
  if(reply_status == CORBA_SYSTEM_EXCEPTION)
    {
      CORBA_unsigned_long minor;

      ev->_major = CORBA_SYSTEM_EXCEPTION;

      rb->cur = ALIGN_ADDRESS(rb->cur, sizeof(minor));
      if((rb->cur + sizeof(minor)) > rb->end)
	goto errout;
      minor = *(CORBA_unsigned_long*)rb->cur;
      rb->cur += 4;
      if(giop_msg_conversion_needed(rb))
	minor = GUINT32_SWAP_LE_BE(minor);

      rb->cur = ALIGN_ADDRESS(rb->cur, sizeof(completion_status));
      if((rb->cur + sizeof(completion_status)) > rb->end)
	goto errout;
      completion_status = *(CORBA_unsigned_long*)rb->cur;
      rb->cur += 4;
      if(giop_msg_conversion_needed(rb))
	completion_status = GUINT32_SWAP_LE_BE(completion_status);

      new = CORBA_SystemException__alloc();
      new->minor=minor;
      new->completed=completion_status;
			
      /* XXX what should the repo ID be? */
      CORBA_exception_set(ev, CORBA_SYSTEM_EXCEPTION,
			  my_repoid,
			  new);
    }
  else if(reply_status == CORBA_USER_EXCEPTION)
    {
      int i;

      if(!ex_info)
	{
	  /* weirdness; they raised an exception that we don't
	     know about */
	  CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
				     CORBA_COMPLETED_MAYBE);
	}
      else
	{
	  for(i = 0; ex_info[i].tc != CORBA_OBJECT_NIL;
	      i++)
	    if(!strcmp(ex_info[i].tc->repo_id,
		       my_repoid))
	      break;

	  if(ex_info[i].tc == CORBA_OBJECT_NIL)
				/* weirdness; they raised an exception
				   that we don't know about */
	    CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
				       CORBA_COMPLETED_MAYBE);
	  else
	    ex_info[i].demarshal(rb, ev);
	}
    };

  return;
  
  /* ignore LOCATION_FORWARD here, that gets handled in the stub */
 errout:
  CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
			     CORBA_COMPLETED_MAYBE);
}

void
ORBit_send_system_exception(GIOPSendBuffer *buf, CORBA_Environment *ev)
{
  CORBA_unsigned_long len;
  CORBA_SystemException *se = ev->_any._value;

  g_assert(ev->_major == CORBA_SYSTEM_EXCEPTION);

  len = strlen(ev->_id) + 1;
  giop_send_buffer_append_aligned (buf, &len, 4);
  giop_send_buffer_append(buf, ev->_id, len);
  
  giop_send_buffer_append_aligned(buf, &se->minor, 4);
  giop_send_buffer_append_aligned(buf, &se->completed, 4);
}

void
ORBit_send_user_exception(GIOPSendBuffer *send_buffer,
			  CORBA_Environment *ev,
			  const ORBit_exception_marshal_info *user_exceptions)
{
  int i;

  for(i = 0; user_exceptions[i].tc != CORBA_OBJECT_NIL; i++)
    {
      if(!strcmp(user_exceptions[i].tc->repo_id, ev->_id))
	break;
    }

  if(user_exceptions[i].tc == CORBA_OBJECT_NIL)
    {
      CORBA_Environment fakeev;
      CORBA_exception_init(&fakeev);
      CORBA_exception_set_system(&fakeev, ex_CORBA_UNKNOWN,
				 CORBA_COMPLETED_MAYBE);
      ORBit_send_system_exception(send_buffer, &fakeev);
      CORBA_exception_free(&fakeev);
    }
  else
    {
      CORBA_unsigned_long len;
      len = strlen(ev->_id) + 1;
      giop_send_buffer_append_aligned(send_buffer, &len, 4);
      giop_send_buffer_append(send_buffer, ev->_id, len);

      if(user_exceptions[i].marshal && ev->_any._value)
	user_exceptions[i].marshal(send_buffer, ev);
    }
}
