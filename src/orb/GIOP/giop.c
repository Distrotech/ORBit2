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

/* FIXME: need to clean this up at shutdown */
static int      corba_wakeup_fds[2];
#define WAKEUP_POLL  corba_wakeup_fds [0]
#define WAKEUP_WRITE corba_wakeup_fds [1]
static GSource *giop_main_source = NULL;
static GIOPThread *giop_main_thread = NULL;

/* Incoming dispatch thread pool */
static GThreadPool *giop_thread_pool    = NULL;
static GMutex      *giop_pool_hash_lock = NULL;
static GHashTable  *giop_pool_hash      = NULL;

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
	
#ifndef __CYGWIN__
	if (statbuf.st_uid != getuid ()) {
		S_PRINT (("Owner of %s is not the current user\n", dirname));
		return FALSE;
	}
	
	if ((statbuf.st_mode & (S_IRWXG|S_IRWXO)) ||
	    !S_ISDIR (statbuf.st_mode)) {
		S_PRINT (("Wrong permissions for %s\n", dirname));
		return FALSE;
	}
#endif

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
giop_thread_new (GMainContext *context, gpointer key)
{
	GIOPThread *tdata = g_new0 (GIOPThread, 1);

	tdata->lock = g_mutex_new ();
	tdata->incoming = g_cond_new ();
	tdata->wake_context = context;
	tdata->key = key;
	tdata->async_ents = g_queue_new();
	tdata->request_queue = g_queue_new();

	if (giop_main_thread)
		tdata->request_handler = giop_main_thread->request_handler;

	G_LOCK (giop_thread_list);
	giop_thread_list = g_list_prepend (giop_thread_list, tdata);
	G_UNLOCK (giop_thread_list);

	return tdata;
}

static void
giop_thread_free (GIOPThread *tdata)
{
	G_LOCK (giop_thread_list);
	giop_thread_list = g_list_remove (giop_thread_list, tdata);
	G_UNLOCK (giop_thread_list);

	g_free (tdata);
}

static GPrivate *giop_tdata_private = NULL;

GIOPThread *
giop_thread_self (void)
{
	GIOPThread *tdata;

	if (!giop_is_threaded)
		return NULL;

	if (!(tdata = g_private_get (giop_tdata_private))) {
		tdata = giop_thread_new (NULL, NULL);
		g_private_set (giop_tdata_private, tdata);
	}

	return tdata;
}

static void
giop_thread_key_release_T (gpointer key)
{
	g_hash_table_remove (giop_pool_hash, key);
}

void
giop_thread_key_release (gpointer key)
{
	if (giop_is_threaded) {
		g_mutex_lock (giop_pool_hash_lock);
		giop_thread_key_release_T (key);
		g_mutex_unlock (giop_pool_hash_lock);
	}
}

void
giop_thread_request_push_key (gpointer  key,
			      gpointer *poa_object,
			      gpointer *recv_buffer)
{
	GIOPThread *tdata, *new_tdata = NULL;

	g_mutex_lock (giop_pool_hash_lock);

	if (!(tdata = g_hash_table_lookup (giop_pool_hash, key))) {
		new_tdata = giop_thread_new (NULL, key);
		tdata = new_tdata;
		dprintf (GIOP, "Create new thread %p for op", tdata);
	} else
		dprintf (GIOP, "Re-use thread %p for op", tdata);

	giop_thread_request_push (tdata, poa_object, recv_buffer);

	if (new_tdata) {
		if (key)
			g_hash_table_insert (giop_pool_hash, key, new_tdata);
		g_thread_pool_push (giop_thread_pool, tdata, NULL);
	}

	g_mutex_unlock (giop_pool_hash_lock);
}

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

static void
giop_request_handler_fn (gpointer data, gpointer user_data)
{
	gboolean done;
	GIOPThread *tdata = user_data;

	g_private_set (giop_tdata_private, tdata);

	dprintf (GIOP, "Thread %p woken to handle request", tdata);

	do {
		giop_thread_request_process (tdata);

		g_mutex_lock (giop_pool_hash_lock);
		LINC_MUTEX_LOCK (tdata->lock);

		if ((done = g_queue_is_empty (tdata->request_queue)) &&
		    tdata->key)
			giop_thread_key_release_T (tdata->key);

		LINC_MUTEX_UNLOCK (tdata->lock);
		g_mutex_unlock (giop_pool_hash_lock);

	} while (!done);

	dprintf (GIOP, "Thread %p returning to pool", tdata);

	giop_thread_free (tdata);
	g_private_set (giop_tdata_private, NULL);
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

		/* FIXME: should really cleanup with descructor */
		giop_tdata_private = g_private_new (NULL);

		giop_main_thread = tdata = giop_thread_new (
			g_main_context_default (), NULL); /* main thread */

		if (pipe (corba_wakeup_fds) < 0) /* cf. g_main_context_init_pipe */
			g_error ("Can't create CORBA main-thread wakeup pipe");

		giop_main_source = linc_source_create_watch (
			g_main_context_default (), WAKEUP_POLL,
			NULL, (G_IO_IN | G_IO_PRI),
			giop_mainloop_handle_input, NULL);
		
		g_private_set (giop_tdata_private, tdata);

		/* Setup thread pool for incoming requests */
		giop_thread_pool = g_thread_pool_new
			(giop_request_handler_fn, NULL, -1, FALSE, NULL);
		giop_pool_hash_lock = linc_mutex_new ();
		giop_pool_hash = g_hash_table_new (NULL, NULL);
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
	       (errno == EAGAIN || errno == EINTR));
	if (res < 0)
		g_error ("Failed to write to GIOP wakeup socket %d 0x%x(%d) (%d)",
			 res, errno, errno, WAKEUP_WRITE);
}

void
giop_incoming_signal_T (GIOPThread *tdata)
{
	g_cond_signal (tdata->incoming);

	if (tdata->wake_context)
		wakeup_mainloop ();
}

static void
giop_incoming_signal (GIOPThread *tdata)
{
	g_mutex_lock (tdata->lock); /* ent_lock */
	giop_incoming_signal_T (tdata);
	g_mutex_unlock (tdata->lock); /* ent_unlock */
}

void
giop_thread_push_recv (GIOPMessageQueueEntry *ent)
{
	g_return_if_fail (ent != NULL);
	g_return_if_fail (ent->buffer != NULL);
	g_return_if_fail (ent->src_thread != NULL);

	/* someone already waiting on the stack */
	giop_incoming_signal_T (ent->src_thread);
}

void
giop_invoke_async (GIOPMessageQueueEntry *ent)
{
	GIOPRecvBuffer *buf = ent->buffer;

	dprintf (GIOP, "About to invoke %p:%p (%d) (%p:%p)",
		 ent, ent->async_cb, giop_is_threaded,
		 ent->src_thread, giop_main_thread);

	if (!giop_is_threaded)
		ent->async_cb (ent);

	else if (ent->src_thread == giop_main_thread) {

		if (giop_thread_self () == giop_main_thread)
			ent->async_cb (ent);

		else {
			GIOPThread *tdata = ent->src_thread;

			g_mutex_lock (tdata->lock); /* ent_lock */

			buf = NULL;
			g_queue_push_tail (tdata->async_ents, ent);
			
			g_assert (tdata->wake_context);
			giop_incoming_signal_T (tdata);

			g_mutex_unlock (tdata->lock); /* ent_unlock */
		}
	} else { /* Push callback to a new thread */
		g_warning ("FIXME: emit async callback in it's own thread ?");

		ent->async_cb (ent);
	}

	/* NB. At the tail end of async_cb 'Ent' is invalid / freed */
	giop_recv_buffer_unuse (buf);
}

static GMainLoop *giop_main_loop = NULL;

void
giop_main_run (void)
{
	if (giop_is_threaded) {
		g_assert (giop_thread_self () == giop_main_thread);
		g_assert (giop_main_loop == NULL);
		giop_main_loop = g_main_loop_new (NULL, TRUE);
		g_main_loop_run (giop_main_loop);
		g_main_loop_unref (giop_main_loop);
		giop_main_loop = NULL;
	} else
		linc_main_loop_run ();
}

void
giop_shutdown (void)
{
	if (!giop_is_threaded)
		giop_connections_shutdown ();

	if (linc_loop) /* break into the linc loop */
		g_main_loop_quit (linc_loop);
	if (giop_main_loop)
		g_main_loop_quit (giop_main_loop);

	if (giop_is_threaded) {
		if (!giop_thread_self ()->wake_context)
			g_error ("Must shutdown ORB from main thread");

		if (giop_main_source) {
			g_source_destroy (giop_main_source);
			g_source_unref (giop_main_source);
			giop_main_source = NULL;
		}

		if (WAKEUP_WRITE >= 0) {
			close (WAKEUP_WRITE);
			close (WAKEUP_POLL);
			WAKEUP_WRITE = -1;
			WAKEUP_POLL = -1;
		}
	}
}

typedef struct {
	gpointer poa_object;
	gpointer recv_buffer;
} GIOPQueueEntry;

void
giop_thread_request_process (GIOPThread *tdata)
{
	GIOPQueueEntry *qe = NULL;

	LINC_MUTEX_LOCK (tdata->lock);
	qe = g_queue_pop_head (tdata->request_queue);	
	LINC_MUTEX_UNLOCK (tdata->lock);

	if (qe)
		tdata->request_handler (qe->poa_object, qe->recv_buffer, NULL);
}

void
giop_thread_request_push (GIOPThread *tdata,
			  gpointer   *poa_object,
			  gpointer   *recv_buffer)
{
	GIOPQueueEntry *qe;

	g_return_if_fail (tdata != NULL);
	g_return_if_fail (poa_object != NULL);
	g_return_if_fail (recv_buffer != NULL);

	qe = g_new (GIOPQueueEntry, 1);

	qe->poa_object  = *poa_object;
	qe->recv_buffer = *recv_buffer;
	*poa_object = NULL;
	*recv_buffer = NULL;

	LINC_MUTEX_LOCK (tdata->lock);

	g_queue_push_tail (tdata->request_queue, qe);
	giop_incoming_signal_T (tdata);

	LINC_MUTEX_UNLOCK (tdata->lock);
}

GIOPThread *
giop_thread_get_main (void)
{
	return giop_main_thread;
}

void
giop_thread_set_main_handler (gpointer request_handler)
{
	if (!giop_is_threaded)
		return;
	g_assert (giop_main_thread != NULL);

	giop_main_thread->request_handler = request_handler;
}
