#include "IIOP-private.h"

G_LOCK_DEFINE_STATIC (request_id_counter);
static CORBA_unsigned_long request_id_counter = 1;


CORBA_unsigned_long
giop_get_request_id(void)
{
  CORBA_unsigned_long retval;
  G_LOCK (request_id_counter);
  retval = request_id_counter++;
  G_UNLOCK (request_id_counter);
  return retval;
}
