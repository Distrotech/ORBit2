#include "config.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <orbit/orbit.h>
#include "poa/orbit-poa.h"

void
ORBit_init_internals(CORBA_ORB orb, CORBA_Environment *ev)
{
  static ORBit_InitialReference root_poa_val = {NULL, TRUE, FALSE};
  struct timeval t;

  root_poa_val.objref = ORBit_POA_setup_root(orb, ev);

  /* released in CORBA_ORB_destroy */
  ORBit_RootObject_duplicate(root_poa_val.objref);

  CORBA_ORB_set_initial_reference(orb, "RootPOA", &root_poa_val, ev);

  gettimeofday (&t, NULL);
  srand (t.tv_sec ^ t.tv_usec ^ getpid ());
}

unsigned int orbit_major_version=ORBIT_MAJOR_VERSION,
	orbit_minor_version=ORBIT_MINOR_VERSION, 
	orbit_micro_version=ORBIT_MICRO_VERSION;
