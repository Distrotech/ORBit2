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
ORBit_handle_exception(GIOPRecvBuffer *buf, CORBA_Environment *ev,
		       ORBit_exception_demarshal_info *ex_info, CORBA_ORB orb)
{
  
}

GIOPConnection *
ORBit_handle_location_forward(GIOPRecvBuffer *buf,
			      CORBA_Object obj)
{
  return NULL;
}

void
ORBit_send_system_exception(GIOPSendBuffer *buf, CORBA_Environment *ev)
{
  g_assert(ev->_major == CORBA_SYSTEM_EXCEPTION);
}
