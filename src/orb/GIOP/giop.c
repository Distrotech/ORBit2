#include "giop-private.h"

const char giop_version_ids[GIOP_NUM_VERSIONS][2] = {
  {1,0},
  {1,1},
  {1,2}
};

void
giop_init(void)
{
  char ctbuf[1024];
  linc_init();

  g_snprintf(ctbuf, sizeof(ctbuf), "/tmp/orbit-%s", g_get_user_name());
  linc_set_tmpdir(ctbuf);

  giop_connection_list_init();
  giop_send_buffer_init();
  giop_recv_buffer_init();
}
