#include "config.h"
#include <orbit/orbit.h>
#include "poa/orbit-poa.h"

void
ORBit_init_internals(CORBA_ORB orb, CORBA_Environment *ev)
{
  static ORBit_InitialReference root_poa_val = {NULL, TRUE, FALSE};

  root_poa_val.objref = ORBit_POA_setup_root(orb, ev);
  CORBA_ORB_set_initial_reference(orb, "RootPOA", &root_poa_val, ev);
}

unsigned int orbit_major_version=ORBIT_MAJOR_VERSION,
	orbit_minor_version=ORBIT_MINOR_VERSION, 
	orbit_micro_version=ORBIT_MICRO_VERSION;
