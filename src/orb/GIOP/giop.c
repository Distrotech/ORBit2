#include <config.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <utime.h>

#include "giop-private.h"
#include "giop-debug.h"
#include <orbit/util/orbit-genrand.h>

const char giop_version_ids [GIOP_NUM_VERSIONS][2] = {
	{1,0},
	{1,1},
	{1,2}
};

#define S_PRINT(a) g_warning a

static gboolean
test_safe_socket_dir (const char *dirname)
{
	struct stat statbuf;

	if (stat (dirname, &statbuf) != 0) {
		S_PRINT (("Can not stat %s\n", dirname));
		return FALSE;
	}
	
	if (statbuf.st_uid != getuid ()) {
		S_PRINT (("Owner of %s is not the current user\n", dirname));
		return FALSE;
	}
	
	if ((statbuf.st_mode & (S_IRWXG|S_IRWXO)) ||
	    !S_ISDIR (statbuf.st_mode)) {
		S_PRINT (("Wrong permissions for %s\n", dirname));
		return FALSE;
	}

	return TRUE;
}

/*
 *   In the absence of being told which directory to
 * use, we have to scan /tmp/orbit-$USER-* to work out
 * which directory to use.
 */
static char *
scan_socket_dir (const char *dir, const char *prefix)
{
	int prefix_len;
	char *cur_dir = NULL;
	DIR   *dirh;
	struct dirent *dent;

	g_return_val_if_fail (dir != NULL, NULL);
	g_return_val_if_fail (prefix != NULL, NULL);
	
	dirh = opendir (dir);
	prefix_len = strlen (prefix);

	while ((dent = readdir (dirh))) {
		char *name;

		if (strncmp (dent->d_name, prefix, prefix_len))
			continue;

		name = g_strconcat (dir, "/", dent->d_name, NULL);

		/* Check it's credentials */
		if (!test_safe_socket_dir (name)) {
			dprintf (GIOP, "DOS attack with '%s'", name);
			g_free (name);
			continue;
		}
		
		/* Sort into some repeatable order */
		if (!cur_dir || strcmp (cur_dir, name)) {
			g_free (cur_dir);
			cur_dir = name;
		} else
			g_free (name);
	}
	closedir (dirh);

	return cur_dir;
}

#define PATH_ROOT "/tmp"

static void
giop_tmpdir_init (void)
{
	char *dirname;
	char *safe_dir = NULL;
	long iteration = 0;

	dirname = g_strdup_printf ("orbit-%s",
				   g_get_user_name ());
	while (!safe_dir) {
		char *newname;

		safe_dir = scan_socket_dir (PATH_ROOT, dirname);
		if (safe_dir) {
			dprintf (GIOP, "Have safe dir '%s'", safe_dir);
			linc_set_tmpdir (safe_dir);
			break;
		}

		if (iteration == 0)
			newname = g_strconcat (PATH_ROOT, "/", dirname, NULL);
		else {
			struct {
				guint32 a;
				guint32 b;
			} id;

			ORBit_genuid_buffer ((guint8 *)&id, sizeof (id));

			newname = g_strdup_printf (
				"%s/%s-%4x", PATH_ROOT, dirname, id.b);
		}

		if (mkdir (newname, 0700) < 0) {
			switch (errno) {
			case EACCES:
				g_error ("I can't write to '%s', ORB init failed",
					 newname);
				break;
				
			case ENAMETOOLONG:
				g_error ("Name '%s' too long your unix is broken",
					 newname);
				break;

			case ENOMEM:
			case ELOOP:
			case ENOSPC:
			case ENOTDIR:
			case ENOENT:
				g_error ("Resource problem creating '%s'", newname);
				break;
				
			default: /* carry on going */
				break;
			}
		}

		{ /* Hide some information ( apparently ) */
			struct utimbuf utb;
			memset (&utb, 0, sizeof (utb));
			utime (newname, &utb);
		}
		
		/* Possible race - so we re-scan. */

		iteration++;
		g_free (newname);

		if (iteration == 1000)
			g_error ("Cannot find a safe socket path in '%s'", PATH_ROOT);
	}

	g_free (safe_dir);
	g_free (dirname);
}

void
giop_init (void)
{
	linc_init (FALSE);

	giop_tmpdir_init ();

	giop_connection_list_init ();

	giop_send_buffer_init ();
	giop_recv_buffer_init ();
}

void
giop_dump (FILE *out, guint8 const *ptr, guint32 len, guint32 offset)
{
	guint32 lp,lp2;
	guint32 off;

	for (lp = 0;lp<(len+15)/16;lp++) {
		fprintf (out, "0x%.4x: ", offset + lp * 16);
		for (lp2=0;lp2<16;lp2++) {
			fprintf (out, "%s", lp2%4?" ":"  ");
			off = lp2 + (lp<<4);
			off<len?fprintf (out, "%.2x", ptr[off]):fprintf (out, "XX");
		}
		fprintf (out, " | ");
		for (lp2=0;lp2<16;lp2++) {
			off = lp2 + (lp<<4);
			fprintf (out, "%c", off<len?(ptr[off]>'!'&&ptr[off]<127?ptr[off]:'.'):'*');
		}
		if (lp == 0)
			fprintf (out, " --- \n");
		else
			fprintf (out, "\n");
	}
}

void
giop_dump_send (GIOPSendBuffer *send_buffer)
{
	gulong nvecs;
	struct iovec *curvec;
	guint32 offset = 0;

	g_return_if_fail (send_buffer != NULL);

	nvecs = send_buffer->num_used;
	curvec = (struct iovec *) send_buffer->iovecs;

	fprintf (stderr, "Outgoing IIOP data:\n");
	while (nvecs-- > 0) {
		giop_dump (stderr, curvec->iov_base, curvec->iov_len, offset);
		offset += curvec->iov_len;
		curvec++;
	}
}

void
giop_dump_recv (GIOPRecvBuffer *recv_buffer)
{
	const char *status;

	g_return_if_fail (recv_buffer != NULL);

	if (recv_buffer->connection &&
	    LINC_CONNECTION (recv_buffer->connection)->status == LINC_CONNECTED)
		status = "connected";
	else
		status = "not connected";

	fprintf (stderr, "Incoming IIOP data: %s\n", status);

	giop_dump (stderr, (guint8 *)recv_buffer, sizeof (GIOPMsgHeader), 0);

	giop_dump (stderr, recv_buffer->message_body + 12,
		   recv_buffer->msg.header.message_size, 12);
}
