#include <orbit/orbit.h>
#include "orbit-poa.h"

PortableServer_POA
PortableServer_ServantBase__default_POA(PortableServer_Servant servant,
					CORBA_Environment *ev)
{
  g_return_val_if_fail(servant, NULL);

  return ORBIT_SERVANT_TO_POAOBJECT(servant)->poa;
}

void
PortableServer_ServantBase__init(PortableServer_Servant p_servant,
				 CORBA_Environment *ev)
{
  PortableServer_ServantBase *servant = p_servant;
  /* NOTE: servant must be init'd before POAObj activated */
  g_assert( servant->_private == NULL );
}

void
PortableServer_ServantBase__fini(PortableServer_Servant p_servant,
				 CORBA_Environment *ev)
{
  /* NOTE: servant must be fini'd after POAObj de-activated */
  PortableServer_ServantBase *servant = p_servant;
  g_assert( servant->_private == NULL );
}
