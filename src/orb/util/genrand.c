#include "genrand.h"
#include <orbit/util/orbit-util.h>
#include <orbit/util/orbit-util.h>
#include <linc/linc.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <glib.h>
#include <signal.h>
#include <sys/time.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

void
ORBit_genrand_init(ORBit_genrand *gr)
{
  gr->fd = open("/dev/urandom", O_RDONLY);
}

void
ORBit_genrand_fini(ORBit_genrand *gr)
{
  if ( gr->fd >= 0 ) {
    close(gr->fd); 
    gr->fd = -1;
  }
}

static gboolean
genrand_dev(ORBit_genrand *gr, guchar *buffer, int buf_len)
{
  if(read(gr->fd, buffer, buf_len) < buf_len)
    {
      close(gr->fd);
      gr->fd = -1;
      return FALSE;
    }

  return TRUE;
}

#if LINC_SSL_SUPPORT
#include <openssl/rand.h>

static gboolean
genrand_openssl(guchar *buffer, int buf_len)
{
  static RAND_METHOD *rm = NULL;
  if(!rm) rm = RAND_get_rand_method();
  RAND_bytes(buffer, buf_len);

  return TRUE;
}
#endif

static inline guchar
hashlong(long val)
{
  guchar retval = 0; /* Quiet gcc */
  guchar *ptr;
  int i;

  for(ptr = (guchar *)&val, i = 0; i < sizeof(val); i++)
    retval ^= ptr[i];

  return retval;
}

static volatile int received_alarm = 0;

static void
handle_alarm(int signum)
{
  received_alarm = 1;
}

static gboolean
genrand_unix(guchar *buffer, int buf_len)
{
  struct sigaction sa, oldsa;
  struct itimerval it, oldit;
  int i;
  long min, max;
  long *counts;
  double diff;
  long *uninit;

  counts = alloca(buf_len * sizeof(long));

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handle_alarm;
  sigaction(SIGALRM, &sa, &oldsa);
  memset(&it, 0, sizeof(it));
  it.it_value.tv_usec = 1;
  getitimer(ITIMER_REAL, &oldit);

  for(i = 0, min = LONG_MAX, max = 0; i < buf_len; i++)
    {
      received_alarm = 0;
      setitimer(ITIMER_REAL, &it, NULL);
      for(counts[i] = 0; !received_alarm; counts[i]++);

      max = MAX(counts[i], max);
      min = MIN(counts[i], min);
    }

  if(!(max - min))
    return FALSE;

  diff = max - min;

  uninit = alloca(buf_len * sizeof(long)); /* Purposely not initialized */
  for(i = 0; i < buf_len; i++)
    {
      long diffval;
      diffval = counts[i] - min;

      buffer[i] ^= (guchar)( ((double) (diffval*256)  / diff )) ^ hashlong(uninit[i]);
    }

  setitimer(ITIMER_REAL, &oldit, NULL);
  sigaction(SIGALRM, &oldsa, NULL);

  return TRUE;
}

void
ORBit_genrand_buf(ORBit_genrand *gr, guchar *buffer, int buf_len)
{
  g_return_if_fail(buf_len);

  if(gr && gr->fd >=0 && genrand_dev(gr, buffer, buf_len))
    return;
#if LINC_SSL_SUPPORT
  else if(genrand_openssl(buffer, buf_len))
    return;
#endif
  else if(genrand_unix(buffer, buf_len))
    return;
  else
    g_error("Couldn't generate random data!");
}
