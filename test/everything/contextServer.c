/*
 * CORBA C language mapping tests
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
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Michael Meeks <michael@ximian.com>
 */

#include "everything.h"
#include "constants.h"
#include <stdio.h>

static  
CORBA_Object
ContextServer_opWithContext(PortableServer_Servant _servant,
			    const CORBA_Object     inArg,
			    CORBA_Object          *inoutArg,
			    CORBA_Object          *outArg,
			    CORBA_Context          ctx,
			    CORBA_Environment     *ev)
{
  *outArg = CORBA_OBJECT_NIL;

  g_warning ("Write the context checks");
  
  return CORBA_Object_duplicate (inArg, ev);
}


PortableServer_ServantBase__epv ContextServer_base_epv = {NULL,NULL,NULL};

POA_test_ContextServer__epv ContextServer_epv = {
  NULL,
  ContextServer_opWithContext,
};

POA_test_ContextServer__vepv ContextServer_vepv = {&ContextServer_base_epv,&ContextServer_epv};

POA_test_ContextServer ContextServer_servant = {NULL,&ContextServer_vepv};  /* Singleton */
