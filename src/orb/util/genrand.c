#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include <glib.h>
#include <linc/linc.h>
#include <orbit/util/orbit-genrand.h>

#include "orbit-purify.h"

static ORBitGenUidType  genuid_type = ORBIT_GENUID_STRONG;
static int              dev_urandom_fd = -1;
static GRand           *glib_prng = NULL;
static pid_t            genuid_pid;

void
ORBit_genuid_init (ORBitGenUidType type)
{
	genuid_type = type;

	switch (genuid_type) {
	case ORBIT_GENUID_STRONG: {
		GTimeVal time;

		dev_urandom_fd = open ("/dev/urandom", O_RDONLY);

		glib_prng = g_rand_new ();
		g_get_current_time (&time);
		g_rand_set_seed (glib_prng, time.tv_sec ^ time.tv_usec);

		break;
	}
	case ORBIT_GENUID_SIMPLE:
		genuid_pid = getpid ();
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

void
ORBit_genuid_fini (void)
{
	switch (genuid_type) {
	case ORBIT_GENUID_STRONG:
		if (dev_urandom_fd >= 0)
			close (dev_urandom_fd); 
		dev_urandom_fd = 0;

		g_rand_free (glib_prng);
		glib_prng = NULL;
		break;
	case ORBIT_GENUID_SIMPLE:
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

static gboolean
genuid_rand_device (guchar *buffer, int length)
{
	if (read (dev_urandom_fd, buffer, length) < length) {
		close (dev_urandom_fd);
		dev_urandom_fd = -1;

		return FALSE;
	}

	return TRUE;
}

#if LINC_SSL_SUPPORT
#include <openssl/rand.h>

static gboolean
genrand_openssl (guchar *buffer, int length)
{
	static RAND_METHOD *rm = NULL;

	if (!rm)
		rm = RAND_get_rand_method ();

	RAND_bytes (buffer, length);

	return TRUE;
}
#endif

#define HEADER_SIZE (sizeof (gulong) + sizeof (genuid_pid))

static void
genuid_simple (guchar *buffer, int length)
{
	static glong  s = 0x6b842128;
	static gulong inc = 0;
	glong         i, t;
	GTimeVal      time;

	g_get_current_time (&time);

	t = time.tv_sec ^ time.tv_usec;
	inc++;

	memcpy (buffer, &inc, sizeof (inc));
	memcpy (buffer + sizeof (inc), &genuid_pid, sizeof (genuid_pid));

	for (i = HEADER_SIZE; i < length; i++)
		buffer [i] = (guchar) (s ^ (t << i));

	s ^= t;
}

static void
genuid_glib_pseudo (guchar *buffer, int length)
{
	int i;

	genuid_simple (buffer, length);

	for (i = HEADER_SIZE; i < length; i++)
		buffer [i] = g_rand_int_range (glib_prng, 0, 255);
}

void
ORBit_genuid_buffer (guchar *buffer, int length)
{
	g_return_if_fail (length > 0);

	switch (genuid_type) {
	case ORBIT_GENUID_STRONG:
		if (genuid_rand_device (buffer, length))
			return;
#if LINC_SSL_SUPPORT
		else if (genuid_rand_openssl (buffer, length))
			return;
#endif
		genuid_glib_pseudo (buffer, length);
		break;
	case ORBIT_GENUID_SIMPLE:
		genuid_simple (buffer, length);
		break;
	default:
		break;
	}
}
