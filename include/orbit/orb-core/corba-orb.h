#ifndef CORBA_ORB_H
#define CORBA_ORB_H 1

#include <orbit/orb-core/orb-types.h>
#include <orbit/orb-core/corba-nvlist-type.h>
#include <orbit/orb-core/corba-orb-type.h>
#include <orbit/orb-core/corba-typecode-type.h>
#include <orbit/orb-core/corba-ir.h>

#define ORBIT_POPT_TABLE \
  { 0, 0, POPT_ARG_INCLUDE_TABLE, (void*)ORBit_options, 0, "ORBit options", 0 }
extern const struct poptOption ORBit_options[];

CORBA_ORB CORBA_ORB_init(int *argc, char **argv,
			 CORBA_ORBid orb_identifier, CORBA_Environment *ev);

/* ORBit extension */
void CORBA_ORB_set_initial_reference(CORBA_ORB orb,
				     CORBA_ORB_ObjectId identifier,
				     ORBit_InitialReference *val,
				     CORBA_Environment *ev);
void ORBit_ORB_forw_bind(CORBA_ORB orb, CORBA_sequence_CORBA_octet *okey,
			 CORBA_Object oref, CORBA_Environment *ev);

guint ORBit_ORB_idle_init (CORBA_ORB orb);

#endif
