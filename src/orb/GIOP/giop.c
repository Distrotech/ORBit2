#include <config.h>
#include <stdio.h>

#include "giop-private.h"

const char giop_version_ids [GIOP_NUM_VERSIONS][2] = {
	{1,0},
	{1,1},
	{1,2}
};

void
giop_init (void)
{
	char ctbuf [1024];

	linc_init (FALSE);

	g_snprintf (ctbuf, sizeof (ctbuf), "/tmp/orbit-%s", g_get_user_name ());
	linc_set_tmpdir (ctbuf);

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
