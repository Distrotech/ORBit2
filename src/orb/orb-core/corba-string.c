#include "config.h"
#include <string.h>
#include <orbit/orb-core/corba-string.h>

CORBA_char *
CORBA_string_alloc(CORBA_unsigned_long len)
{
  return ORBit_alloc_simple(len+1);
}

CORBA_char *
CORBA_string_dup(CORBA_char *str)
{
  CORBA_char *retval;
  CORBA_unsigned_long len;

  if(!str)
    return NULL;

  len = strlen(str) + 1;

  retval = ORBit_alloc_simple(len);
  memcpy(retval, str, len);

  return retval;
}

gpointer
CORBA_string__freekids(gpointer mem, gpointer data)
{
  CORBA_char **pstr = mem;
  CORBA_free(*pstr);
  return pstr + 1; 
}

CORBA_wchar *
CORBA_wstring_alloc(CORBA_unsigned_long len)
{
  return ORBit_alloc_simple((len+1)*2);
}
