#include "config.h"

#include <orbit/orb-core/corba-environment.h>

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
      CORBA_Environment subev = {CORBA_NO_EXCEPTION};

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
