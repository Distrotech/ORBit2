#include "everything.h"
#include "constants.h"
#include <stdio.h>

extern PortableServer_POA global_poa;

static CORBA_Object registered = CORBA_OBJECT_NIL;

static void
PingPongServer_set (PortableServer_Servant    servant,
		    CORBA_Object              object,
		    const CORBA_char          *name,
		    CORBA_Environment        *ev)
{
	registered = CORBA_Object_duplicate (object, ev);
}

static CORBA_Object
PingPongServer_get (PortableServer_Servant    servant,
		    const CORBA_char          *name,
		    CORBA_Environment        *ev)
{
	return CORBA_Object_duplicate (registered, ev);
}

static void
PingPongServer_opOneWay (PortableServer_Servant    servant,
			 const CORBA_long          l,
			 CORBA_Environment        *ev)
{
	/* Do nothing, but try and confuse the queue */
	linc_main_iteration (FALSE);
}

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

	test_PingPongServer_opOneWay (replyTo, 3, ev);

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
	PingPongServer_opOneWay,
	PingPongServer_pingPong,
	PingPongServer_set,
	PingPongServer_get
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

