#include "config.h"
#include <orbit/orbit.h>
#include "poa/orbit-poa.h"

void
ORBit_init_internals(CORBA_ORB orb, CORBA_Environment *ev)
{
  ORBit_InitialReference root_poa_val = {NULL, TRUE, FALSE};
  ORBit_InitialReference poa_manager_val = {NULL, TRUE, FALSE};

  root_poa_val.objref = ORBit_POA_setup_root(orb, ev);
  CORBA_ORB_set_initial_reference(orb, "RootPOA", &root_poa_val, ev);
}
