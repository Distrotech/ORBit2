#include "config.h"
#include <orbit/orbit.h>

gpointer
CORBA_sequence__freekids(gpointer mem, gpointer dat)
{
  CORBA_sequence_CORBA_octet *seq = mem;

  if(seq->_release)
    CORBA_free(seq->_buffer);

  return seq + 1;
}

CORBA_sequence_octet*
ORBit_sequence_octet_dup(const CORBA_sequence_octet *in)
{
  return NULL;
}

CORBA_sequence_PortableServer_POA*
CORBA_sequence_PortableServer_POA__alloc(void)
{
  return NULL;
}
PortableServer_POA*
CORBA_sequence_PortableServer_POA_allocbuf(CORBA_unsigned_long len)
{
  return NULL;
}
