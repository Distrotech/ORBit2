#include "config.h"
#include <orbit/orbit.h>

void ORBit_marshal_arg(GIOPSendBuffer *buf,
                       gconstpointer val,
                       CORBA_TypeCode tc)
{
}

void ORBit_marshal_any(GIOPSendBuffer *buf, const CORBA_any *val)
{
}

gpointer
ORBit_demarshal_arg(GIOPRecvBuffer *buf,
		    CORBA_TypeCode tc,
		    gboolean dup_strings,
		    CORBA_ORB orb)
{
  return NULL;
}

gboolean
ORBit_demarshal_any(GIOPRecvBuffer *buf, CORBA_any *retval,
		    gboolean dup_strings,
		    CORBA_ORB orb)
{
  return FALSE;
}

gpointer
CORBA_any__freekids(gpointer mem, gpointer dat)
{
  CORBA_any *t;
  t = mem;
  if(t->_release)
    g_free(t->_value); /* XXX fixme */
  g_free(t);
  return t + 1;
}
