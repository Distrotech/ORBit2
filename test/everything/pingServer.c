#include "everything.h"
#include "constants.h"
#include <stdio.h>

extern PortableServer_POA global_poa;

static CORBA_long
PingPongServer_pingPong (PortableServer_Servant    servant,
			 const test_PingPongServer replyTo,
			 const CORBA_long          idx,
			 CORBA_Environment        *ev)
{

	CORBA_Object me;

	me = PortableServer_POA_servant_to_reference (
		global_poa, servant, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	if (idx > 0)
		return test_PingPongServer_pingPong (replyTo, me, idx - 1, ev);
	else
		return 0;
}

static void
ping_pong_finalize (PortableServer_Servant servant,
		    CORBA_Environment     *ev)
{
	g_free (servant);
}

PortableServer_ServantBase__epv PingPongServer_base_epv = {
	NULL, ping_pong_finalize, NULL
};

POA_test_PingPongServer__epv PingPongServer_epv = {
	NULL,
	PingPongServer_pingPong
};

POA_test_PingPongServer__vepv PingPongServer_vepv = {
	&PingPongServer_base_epv,
	&PingPongServer_epv
};

POA_test_PingPongServer *
create_ping_pong_servant (void)
{
	POA_test_PingPongServer *servant;
	
	servant = g_new0 (POA_test_PingPongServer, 1);
	servant->vepv = &PingPongServer_vepv;

	return servant;
};

