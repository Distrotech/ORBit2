#include <orbit/orb-core/orb-core.h>
#include <linc/linc.h>

static void
CORBA_ORB_destroy(ORBit_RootObject robj)
{
  CORBA_ORB orb = (CORBA_ORB)robj;

  g_list_foreach(orb->servers, (GFunc)g_object_unref, NULL);
  g_ptr_array_free(orb->poas, TRUE);

  g_free(obj);
}

CORBA_ORB
CORBA_ORB_init(int *argc, char **argv, CORBA_ORBid orb_identifier, CORBA_Environment *ev)
{
  static CORBA_ORB retval = NULL;
  LINCProtocolInfo *info;
  static ORBit_RootObject_Interface orb_if = {
    ORBIT_ROT_ORB,
    CORBA_ORB_destroy
  };

  if(retval)
    return (CORBA_ORB)CORBA_Object_duplicate((CORBA_Object)orb);

  retval = g_new0(struct _CORBA_ORB, 1);
  orb = (CORBA_ORB)retval;

  ORBit_RootObject_init(retval, &orb_if);

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

  retval->poas = g_ptr_array_new();

  return retval;
}

/* ORBit extension */
void
CORBA_ORB_set_initial_reference(CORBA_ORB orb, CORBA_ORB_ObjectId identifier, CORBA_Object obj, CORBA_Environment *ev)
{
}
