#include "linc-private.h"

GMainLoop    *linc_loop = NULL;
GMainContext *linc_context = NULL;

#ifdef LINC_SSL_SUPPORT
SSL_METHOD *linc_ssl_method;
SSL_CTX    *linc_ssl_ctx;
#endif

void
linc_init (gboolean init_threads)
{
	if (init_threads && !g_thread_supported ())
		g_thread_init (NULL);

	g_type_init ();

	linc_context = g_main_context_new ();
	linc_loop    = g_main_loop_new (linc_context, TRUE);

#ifdef LINC_SSL_SUPPORT
	SSLeay_add_ssl_algorithms ();
	linc_ssl_method = SSLv23_method ();
	linc_ssl_ctx = SSL_CTX_new (linc_ssl_method);
#endif
}

struct _LincWatch {
	guint main_id;
	guint linc_id;
};

LincWatch *
linc_io_add_watch (GIOChannel    *channel,
		   GIOCondition   condition,
		   GIOFunc        func,
		   gpointer       user_data)
{
	GSource  *source;
	LincWatch *w;
  
	g_return_val_if_fail (channel != NULL, 0);

	w = g_new (LincWatch, 1);

	/* Linc loop */
	source = g_io_create_watch (channel, condition);
	g_source_set_callback (source, (GSourceFunc)func, user_data, NULL);
	w->linc_id = g_source_attach (source, linc_context);
	g_source_unref (source);

	/* Main loop */
	source = g_io_create_watch (channel, condition);
	g_source_set_callback (source, (GSourceFunc)func, user_data, NULL);
	w->main_id = g_source_attach (source, NULL);
	g_source_unref (source);

	return w;
}

void
linc_io_remove_watch (LincWatch *watch)
{
	if (watch) {
		GSource *source;

		source = g_main_context_find_source_by_id (
			NULL, watch->main_id);
		if (source)
			g_source_destroy (source);
		else
			g_warning ("Missing source on main context");

		source = g_main_context_find_source_by_id (
			linc_context, watch->linc_id);
		if (source)
			g_source_destroy (source);
		else
			g_warning ("Missing source on linc context");

		g_free (watch);
	}
}

void
linc_main_iteration (gboolean block_for_reply)
{
	g_main_context_iteration (
		linc_context, block_for_reply);
}

gboolean
linc_main_pending (void)
{
	return g_main_context_pending (linc_context);
}

void
linc_main_loop_run (void)
{
	g_main_loop_run (linc_loop);
}

GMutex *
linc_mutex_new (void)
{
#ifdef G_THREADS_ENABLED
	if (g_thread_supported ())
		return g_mutex_new ();
#endif

	return NULL;
}
