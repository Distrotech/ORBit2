#ifndef CORBA_ORB_H
#define CORBA_ORB_H 1

#include <orbit/orb-core/orb-types.h>
#include <orbit/orb-core/corba-nvlist-type.h>
#include <orbit/orb-core/corba-object-type.h>
#include <orbit/orb-core/corba-orb-type.h>
#include <orbit/orb-core/corba-typecode-type.h>
#include <orbit/orb-core/corba-ir.h>

CORBA_ORB CORBA_ORB_init(int *argc, char **argv,
			 CORBA_ORBid orb_identifier, CORBA_Environment *ev);

/* ORBit extension */
void CORBA_ORB_set_initial_reference(CORBA_ORB orb,
				     CORBA_ORB_ObjectId identifier,
				     CORBA_Object obj, CORBA_Environment *ev);
#endif
