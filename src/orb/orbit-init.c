#include "config.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <orbit/orbit.h>

#include "orbit-init.h"
#include "poa/orbit-poa.h"
#include "orb-core/orb-core-private.h"

void
ORBit_init_internals (CORBA_ORB          orb,
		      CORBA_Environment *ev)
{
	PortableServer_POA     root_poa;
	PortableServer_Current poa_current;
	struct timeval         t;

	root_poa = ORBit_POA_setup_root (orb, ev);
	ORBit_set_initial_reference (orb, "RootPOA", root_poa);
	ORBit_RootObject_release (root_poa);

	poa_current = ORBit_POACurrent_new (orb);
	ORBit_set_initial_reference (orb, "POACurrent", poa_current);
	ORBit_RootObject_release (poa_current);

	gettimeofday (&t, NULL);
	srand (t.tv_sec ^ t.tv_usec ^ getpid ());
}

const char  *orbit_version       = ORBIT_VERSION;
unsigned int orbit_major_version = ORBIT_MAJOR_VERSION;
unsigned int orbit_minor_version = ORBIT_MINOR_VERSION; 
unsigned int orbit_micro_version = ORBIT_MICRO_VERSION;
