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
	if (!dirh)
		return NULL;
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

static void
giop_tmpdir_init (void)
{
	const char *tmp_root;
	char *dirname;
	char *safe_dir = NULL;
	long iteration = 0;

	tmp_root = g_get_tmp_dir ();
	dirname = g_strdup_printf ("orbit-%s",
				   g_get_user_name ());
	while (!safe_dir) {
		char *newname;

		safe_dir = scan_socket_dir (tmp_root, dirname);
		if (safe_dir) {
			dprintf (GIOP, "Have safe dir '%s'", safe_dir);
			linc_set_tmpdir (safe_dir);
			break;
		}

		if (iteration == 0)
			newname = g_strconcat (tmp_root, "/", dirname, NULL);
		else {
			struct {
				guint32 a;
				guint32 b;
			} id;

			ORBit_genuid_buffer ((guint8 *)&id, sizeof (id),
					     ORBIT_GENUID_OBJECT_ID);

			newname = g_strdup_printf (
				"%s/%s-%4x", tmp_root, dirname, id.b);
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
			g_error ("Cannot find a safe socket path in '%s'", tmp_root);
	}

	g_free (safe_dir);
	g_free (dirname);
}

static gboolean giop_is_threaded = FALSE;

gboolean
giop_threaded (void)
{
	return giop_is_threaded;
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
		fprintf (out, "\n");
	}
	fprintf (out, " --- \n");
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

G_LOCK_DEFINE_STATIC (giop_thread_list);
static GList *giop_thread_list = NULL;

static GIOPThread *
giop_thread_new (GMainContext *context)
{
	GIOPThread *tdata = g_new0 (GIOPThread, 1);

	tdata->lock = g_mutex_new ();
	tdata->incoming = g_cond_new ();
	tdata->wake_context = context;

	G_LOCK (giop_thread_list);
	giop_thread_list = g_list_prepend (giop_thread_list, tdata);
	G_UNLOCK (giop_thread_list);

	return tdata;
}

static GPrivate *giop_tdata_private = NULL;

GIOPThread *
giop_thread_self (void)
{
	GIOPThread *tdata;

	if (!giop_is_threaded)
		return NULL;

	if (!(tdata = g_private_get (giop_tdata_private))) {
		tdata = giop_thread_new (NULL);
		g_private_set (giop_tdata_private, tdata);
	}

	return tdata;
}

/* FIXME: need to clean this up at shutdown */
static int      corba_wakeup_fds[2];
#define WAKEUP_POLL  corba_wakeup_fds [0]
#define WAKEUP_WRITE corba_wakeup_fds [1]
static GSource *giop_main_source = NULL;
static GThread *giop_incoming_thread = NULL;

static gboolean
giop_mainloop_handle_input (GIOChannel     *source,
			    GIOCondition    condition,
			    gpointer        data)
{
	char c;

	read (WAKEUP_POLL, &c, sizeof (c));
	giop_recv_handle_queued_input ();

	return TRUE;
}

void
giop_init (gboolean threaded, gboolean blank_wire_data)
{
	if (threaded)
		g_warning ("\n --- you're entering a whole world of pain --- ");
	giop_is_threaded = threaded;
	linc_init (threaded);

	if (threaded) {
		GIOPThread *tdata;
		GError     *error = NULL;

		/* FIXME: should really cleanup with descructor */
		giop_tdata_private = g_private_new (NULL);

		tdata = giop_thread_new ( /* main thread */
			g_main_context_default ());

		if (pipe (corba_wakeup_fds) < 0) /* cf. g_main_context_init_pipe */
			g_error ("Can't create CORBA main-thread wakeup pipe");

		giop_main_source = linc_source_create_watch (
			g_main_context_default (), WAKEUP_POLL,
			NULL, (G_IO_IN | G_IO_PRI),
			giop_mainloop_handle_input, NULL);
		
		g_private_set (giop_tdata_private, tdata);

		giop_incoming_thread =
			g_thread_create_full (
				giop_recv_thread_fn,
				NULL, /* data */
				0, TRUE, FALSE, 
				G_THREAD_PRIORITY_NORMAL,
				&error);
		if (!giop_incoming_thread || error)
			g_error ("Failed to create ORB worker thread");
	}

	giop_tmpdir_init ();

	giop_connection_list_init ();

	giop_send_buffer_init (blank_wire_data);
	giop_recv_buffer_init ();
}

static void
wakeup_mainloop (void)
{
	char c = 'A'; /* magic */
	int  res;
	while ((res = write (WAKEUP_WRITE, &c, sizeof (c))) < 0  &&
	       errno == EAGAIN);
	if (res < 0)
		g_error ("Failed to write to wakeup socket");
}

void
giop_thread_push_recv (GIOPMessageQueueEntry *ent)
{
	GIOPThread *tdata;

	g_return_if_fail (ent != NULL);
	g_return_if_fail (ent->buffer != NULL);
	g_return_if_fail (ent->src_thread != NULL);

	tdata = ent->src_thread;

	g_mutex_lock (tdata->lock);

	if (ent->async_cb) /* we need the recv buffer later */
		tdata->recv_buffers = g_slist_append (
			tdata->recv_buffers, ent->buffer);
	/* else - someone already waiting on the stack */

	/* wakeup thread ... */
	g_cond_signal (tdata->incoming);

	if (ent->async_cb) {
		if (!tdata->wake_context)
			g_error ("Can't do async code outside of main thread");
		wakeup_mainloop ();
	}
	g_mutex_unlock (tdata->lock);
}

void
giop_shutdown (void)
{
	if (!giop_is_threaded)
		giop_connections_shutdown ();
	g_main_loop_quit (linc_loop); /* break into the linc loop */

	if (giop_is_threaded) {
		if (!giop_thread_self ()->wake_context)
			g_error ("Must shutdown ORB from main thread");

		g_thread_join (giop_incoming_thread);
		giop_incoming_thread = NULL;

		g_source_destroy (giop_main_source);
		g_source_unref (giop_main_source);
		giop_main_source = NULL;

		close (WAKEUP_WRITE);
		close (WAKEUP_POLL);
		WAKEUP_WRITE = -1;
		WAKEUP_POLL = -1;
	}
}

typedef struct {
	gpointer poa_object;
	gpointer recv_buffer;
} GIOPQueueEntry;


struct _GIOPQueue {
	GList           *entries;
	GIOPQueueHandler handler_fn;
	void           (*push_fn) (GIOPQueue      *queue,
				   GIOPQueueEntry *qe);
};

void
giop_queue_push (GIOPQueue *queue,
		 gpointer   poa_object,
		 gpointer   recv_buffer)
{
	GIOPQueueEntry *qe;

	g_return_if_fail (queue != NULL);

	qe = g_new (GIOPQueueEntry, 1);

	qe->poa_object  = poa_object;
	qe->recv_buffer = recv_buffer;

	/* FIXME: no locking */
	queue->entries = g_list_append (queue->entries, qe);

	/* FIXME: no special casing */
	if (queue->push_fn)
		queue->push_fn (queue, qe);
}

GIOPQueue *
giop_queue_new (gpointer queue_handler)
{
	GIOPQueue *queue = g_new0 (GIOPQueue, 1);

	queue->handler_fn = queue_handler;
	
	return queue;
}

GIOPQueue *
giop_queue_get_main (void)
{
	return NULL;
}

void
giop_queue_free (GIOPQueue *queue)
{
	g_assert (queue->entries == NULL);
	g_free (queue);
}
