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
 * Author: Mark McLoughlin <mark@skynet.ie>
 */

#include "everything.h"

static void
LifeCycleServer_deactivateOnReturn (PortableServer_Servant  servant,
				    CORBA_Environment      *ev)
{
	PortableServer_ObjectId *oid;

	oid = PortableServer_POA_servant_to_id (global_poa, servant, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	PortableServer_POA_deactivate_object (global_poa, oid, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	CORBA_free (oid);
}

static POA_test_LifeCycleServer__epv LifeCycleServer_epv = {
	NULL,
	LifeCycleServer_deactivateOnReturn
};

static PortableServer_ServantBase__epv LifeCycleServer_base_epv = {
	NULL,
	simple_finalize,
	NULL
};
static POA_test_LifeCycleServer__vepv LifeCycleServer_vepv = {
	&LifeCycleServer_base_epv,
	&LifeCycleServer_epv
};
