#include "config.h"
#include <orbit/orbit.h>
#include <string.h>

gpointer
CORBA_sequence__freekids(gpointer mem, gpointer dat)
{
  CORBA_sequence_CORBA_octet *seq = mem;

  if(seq->_release)
    ORBit_free_T(seq->_buffer);

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
      retval->_release = TRUE;
    }

  return retval;
}
