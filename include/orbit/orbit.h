/* This stays outside */
#include <orbit/orbit-config.h>

#ifdef ORBIT_IDL_SERIAL
#if ORBIT_IDL_SERIAL < ORBIT_CONFIG_SERIAL
#error "You need to rerun 'orbit-idl' on the .idl file whose stubs you are using. These stubs were generated with an older version of ORBit, and need to be regenerated."
#endif
#endif

#ifndef ORBIT_H
#define ORBIT_H 1

#include <orbit/orbit-types.h>
#include <orbit/GIOP/giop.h>
#include <orbit/orb-core/orb-core.h>
#include <orbit/poa/poa.h>

#endif
