/*
 * CORBA echo tests
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Author: Elliot Lee <sopwith@redhat.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <orb/orbit.h>
#include <ctype.h>
#include "echo.h"
#include "echo-share.h"

/*public*/ gboolean echo_opt_quiet = 0;
static gboolean echo_opt_donothing = 0;
static int      echo_opt_numlives = 1;
static int	echo_opt_iterations = 100;


static int doinvokes(CORBA_Object echo_ref, CORBA_Environment *ev) {
	int	i;
	if ( echo_opt_donothing ) {
		for (i = 0; i < echo_opt_iterations; i++) {
			Echo_doNothing(echo_ref, ev);
			if(ev->_major != CORBA_NO_EXCEPTION) {
				printf("we got exception %d from doNothing!\n",
				       ev->_major);
				return 1;
			}
		}
	} else {
		for(i = 0; i < echo_opt_iterations; i++) {
			char buf[30];
			CORBA_double rv;
			Echo bec;
			g_snprintf(buf, sizeof(buf), "Hello, world [%d]", i);
			bec = Echo_echoString(echo_ref, buf, &rv, ev);
			if(ev->_major != CORBA_NO_EXCEPTION) {
				printf("we got exception %d from echoString!\n",
				       ev->_major);
				return 1;
			}
			g_assert(echo_ref == bec);
			CORBA_Object_release(echo_ref, ev);
			if(ev->_major != CORBA_NO_EXCEPTION) {
				printf("we got exception %d from release!\n", 
				       ev->_major);
				return 1;
			} else {
				if ( !echo_opt_quiet ) {
					g_message("[client] %f", rv);
				}
			}
			echo_ref = bec;
		}
	}
	return 0;
}


int
main (int argc, char *argv[])
{
	CORBA_Environment ev;
	CORBA_ORB orb;
	int i, argi;
	CORBA_Object echo_ref;


	CORBA_exception_init(&ev);
	orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

	for ( argi=1; argi < argc; ) {
		/*IF*/ if ( strcmp(argv[argi],"--quiet")==0 ) {
			echo_opt_quiet = 1; argi++;
		} else if ( strcmp(argv[argi],"--donothing")==0 ) {
			echo_opt_donothing = 1; argi++;
		} else if ( strncmp(argv[argi],"--numlives=",11)==0 ) {
			echo_opt_numlives = atoi(argv[argi]+11); argi++;
		} else if ( isdigit(argv[argi][0]) ) {
			echo_opt_iterations = atoi(argv[argi]); argi++;
		} else {
			fprintf(stderr,"usage: echo-local [--quiet] "
				"[--donothing] [--numlives=lives] "
				"[num_iterations]\n");
			exit(1);
		}
	}

	echo_srv_start_poa(orb, &ev);

	for (i = 0; i < echo_opt_numlives; i++) {
		echo_ref = echo_srv_start_object(&ev);

		if ( doinvokes(echo_ref, &ev) )
			return 1;

		echo_srv_finish_object(&ev);
	}
	echo_srv_finish_poa(&ev);

	g_message("[client] Completed %d iterations of %d lives.", 
		  echo_opt_iterations, echo_opt_numlives);

	CORBA_ORB_destroy(orb, &ev);
	if ( ev._major ) {
		printf("ORB_destroy failed: %d\n", ev._major);
		return 1;
	}
	CORBA_Object_release((CORBA_Object)orb, &ev);
	if ( ev._major ) {
		printf("ORB release failed: %d\n", ev._major);
		return 1;
	}
	g_blow_chunks();
	return 0;
}
