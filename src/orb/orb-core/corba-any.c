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

gpointer ORBit_demarshal_arg(GIOPRecvBuffer *buf,
                             CORBA_TypeCode tc,
                             gboolean dup_strings,
                             CORBA_ORB orb)
{
}

gboolean ORBit_demarshal_any(GIOPRecvBuffer *buf, CORBA_any *retval,
			     gboolean dup_strings,
			     CORBA_ORB orb)
{
}

gpointer
CORBA_any__freekids(gpointer mem, gpointer dat)
{
}
