#include <orbit/orbit.h>
#include "orbit-poa.h"

PortableServer_POA
PortableServer_ServantBase__default_POA(PortableServer_Servant servant,
					CORBA_Environment *ev)
{
  g_return_val_if_fail(servant, NULL);

  return ORBIT_SERVANT_TO_FIRST_POAOBJECT(servant)->poa;
}

void
PortableServer_ServantBase__init(PortableServer_Servant p_servant,
				 CORBA_Environment *ev)
{
}

void
PortableServer_ServantBase__fini(PortableServer_Servant p_servant,
				 CORBA_Environment *ev)
{
}
