#include "giop-private.h"

const char giop_version_ids[GIOP_NUM_VERSIONS][2] = {
  {1,0},
  {1,1},
  {1,2}
};

O_MUTEX_DEFINE(giop_queued_messages_lock);
GList *giop_queued_messages = NULL;

void
giop_init(void)
{
  O_MUTEX_INIT(giop_queued_messages_lock);
  giop_send_buffer_init();
  giop_recv_buffer_init();
  linc_init();
}
