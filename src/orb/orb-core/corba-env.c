#include "config.h"

#include <orbit/orb-core/corba-environment.h>

void
CORBA_exception_set_system(CORBA_Environment *ev,
			   CORBA_char *except_repos_id,
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
		    CORBA_char *except_repos_id,
		    void *param)
{
  CORBA_exception_free(ev);
  
  ev->_major = major;
  if(major != CORBA_NO_EXCEPTION)
    {
      ev->_id = CORBA_string_dup(except_repos_id);
      ev->_any._type = TC_null; /* CORBA sucks */
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

void
CORBA_exception_free(CORBA_Environment *ev)
{
  if(ev->_major != CORBA_NO_EXCEPTION)
    {
      CORBA_free(ev->_id);
      CORBA_any__freekids(&ev->_any, NULL);
      ev->_any._type = TC_null;
      ev->_any._value = NULL;
      ev->_any._release = CORBA_FALSE;
    }
}

CORBA_any *
CORBA_exception_as_any(CORBA_Environment *ev)
{
  return &ev->_any;
}
