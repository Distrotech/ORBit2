#include "giop-private.h"

const char giop_version_ids[GIOP_NUM_VERSIONS][2] = {
  {1,0},
  {1,1},
  {1,2}
};

void
giop_init(void)
{
  giop_send_buffer_init();
}
