#include <orbit/orb-core/orb-core.h>
#include <linc/linc.h>

CORBA_ORB
CORBA_ORB_init(int *argc, char **argv, CORBA_ORBid orb_identifier, CORBA_Environment *ev)
{
  CORBA_ORB retval = NULL;
  LINCProtocolInfo *info;

  retval = g_new0(struct _CORBA_ORB, 1);

  for(info = linc_protocol_all(); info->name; info++)
    {
      GIOPServer *server;

      server = giop_server_new(info->name, NULL, NULL, 0);
      if(server)
	{
	  retval->servers = g_list_prepend(retval->servers, server);
	  if(!(info->flags & LINC_PROTOCOL_SECURE))
	    {
	      server = giop_server_new(info->name, NULL, NULL, LINC_CONNECTION_SSL);
	      if(server)
		retval->servers = g_list_prepend(retval->servers, server);
	    }
	}
    }

  return retval;
}

CORBA_Current
CORBA_ORB_get_current(CORBA_ORB orb, CORBA_Environment *ev)
{
}

/* ORBit extension */
void
CORBA_ORB_set_initial_reference(CORBA_ORB orb, CORBA_ORB_ObjectId identifier, CORBA_Object obj, CORBA_Environment *ev)
{
}
