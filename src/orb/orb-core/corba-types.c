#include "config.h"
#include <orbit/orbit.h>
#include <string.h>

gpointer
CORBA_sequence__freekids(gpointer mem, gpointer dat)
{
  CORBA_sequence_CORBA_octet *seq = mem;

  if(seq->_release)
    CORBA_free(seq->_buffer);

  return seq + 1;
}

CORBA_sequence_CORBA_octet*
ORBit_sequence_CORBA_octet_dup(const CORBA_sequence_CORBA_octet *in)
{
  CORBA_sequence_CORBA_octet *retval;
  retval = CORBA_sequence_CORBA_octet__alloc();
  *retval = *in;
  if(in->_buffer)
    {
      retval->_buffer = CORBA_sequence_CORBA_octet_allocbuf(in->_length);
      memcpy(retval->_buffer, in->_buffer, in->_length);
    }

  return retval;
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
