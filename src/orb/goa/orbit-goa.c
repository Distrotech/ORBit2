/*
 * orbit-goa.c:
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *	Mark McLoughlin <mark@skynet.ie>
 */

#include <config.h>

#include "orbit-goa.h"

#include <string.h>

struct _ORBit_GOA {
	struct ORBit_ObjectAdaptor_type  base;

	GHashTable                      *servant_hash;

	guint                            iterating_hash : 1;
	guint                            destroyed      : 1;
};

static ORBit_GOA goa_singleton = NULL;

static void
ORBit_GOA_finalize (ORBit_RootObject robj)
{
	ORBit_GOA goa = (ORBit_GOA) robj;

	ORBit_adaptor_free ((ORBit_ObjectAdaptor) goa);

	g_assert (goa->destroyed);

	g_free (goa);
}

static const ORBit_RootObject_Interface ORBit_GOA_epv = {
        ORBIT_ROT_ADAPTOR,
        ORBit_GOA_finalize
};

static ORBitGServant *
ORBit_GOA_lookup_servant (ORBit_GOA        goa,
			  ORBit_ObjectKey *objkey)
{
	char *object_id;

	g_return_val_if_fail (goa->servant_hash != NULL, NULL);
	g_return_val_if_fail (objkey != NULL && objkey->_buffer != NULL, NULL);
	g_return_val_if_fail (objkey->_length == ORBIT_ADAPTOR_PREFIX_LEN + ORBIT_GOA_OBJECT_ID_LEN, NULL);

	object_id = objkey->_buffer + ORBIT_ADAPTOR_PREFIX_LEN;

	return g_hash_table_lookup (goa->servant_hash, object_id);
}

char *
ORBit_goa_register_servant (ORBit_GOA      goa,
			    ORBitGServant *servant)
{
	static CORBA_long  id_inc = 0;
	char              *object_id;

	g_return_val_if_fail (!goa->destroyed, NULL);
	g_return_val_if_fail (ORBIT_IS_GSERVANT (servant), NULL);

	object_id = g_new (char, ORBIT_GOA_OBJECT_ID_LEN);

        *((CORBA_long *) object_id) = ++id_inc;

	ORBit_genuid_buffer (object_id + sizeof (CORBA_long),
                             ORBIT_GOA_OBJECT_ID_LEN,
			     ORBIT_GENUID_OBJECT_ID);

	g_hash_table_insert (goa->servant_hash, object_id, servant);

	return object_id;
}

gboolean
ORBit_goa_unregister_servant (ORBit_GOA   goa,
			      const char *object_id)
{
	g_return_val_if_fail (!goa->destroyed, FALSE);
	g_return_val_if_fail (object_id != NULL, FALSE);

	if (goa->iterating_hash) /* we are iterating over the hash */
		return FALSE;

	return g_hash_table_remove (goa->servant_hash, object_id);
}

static void
ORBit_GOA_handle_request (ORBit_GOA        goa,
                          GIOPRecvBuffer  *recv_buffer,
                          ORBit_ObjectKey *objkey)
{
	ORBitGServant     *servant;
	CORBA_Environment  env;

	CORBA_exception_init (&env);

	if ((servant = ORBit_GOA_lookup_servant (goa, objkey))) {
		CORBA_Identifier opname;

		opname = giop_recv_buffer_get_opname (recv_buffer);

		ORBit_gservant_handle_request (servant, opname, NULL, NULL,
					       NULL, recv_buffer, &env);
	} else
		CORBA_exception_set_system (
                        &env, ex_CORBA_OBJECT_NOT_EXIST,
                        CORBA_COMPLETED_NO);

	if (env._major != CORBA_NO_EXCEPTION)
		ORBit_recv_buffer_return_sys_exception (recv_buffer, &env);

	CORBA_exception_free (&env);
}

static gboolean
ORBit_goa_object_id_equal (gconstpointer  a,
			   gconstpointer  b)
{
	return !memcmp (a, b, ORBIT_GOA_OBJECT_ID_LEN);
}

static guint
ORBit_goa_object_id_hash (const char *object_id)
{
	return *(guint *) object_id;
}

ORBit_GOA
ORBit_goa_initial_reference (CORBA_ORB orb,
			     gboolean  only_if_exists)
{
	ORBit_GOA goa;

	if (goa_singleton)
		return ORBit_RootObject_duplicate (goa_singleton);

	if (only_if_exists)
		return NULL;

	g_return_val_if_fail (orb != NULL, NULL);

	goa = g_new0 (struct _ORBit_GOA, 1);

	ORBit_RootObject_init ((ORBit_RootObject) goa, &ORBit_GOA_epv);

	goa_singleton = ORBit_RootObject_duplicate (goa);

	ORBIT_ADAPTOR (goa)->handle_request = (ORBitReqHandlerFunc) ORBit_GOA_handle_request;

	ORBit_adaptor_setup (ORBIT_ADAPTOR (goa), ORBIT_ADAPTOR_GOA, orb);

	goa->servant_hash = g_hash_table_new (
				(GHashFunc) ORBit_goa_object_id_hash,
				(GEqualFunc) ORBit_goa_object_id_equal);

	return ORBit_RootObject_duplicate (goa_singleton);
}

static gboolean
ORBit_GOA_servant_remove_foreach (const char    *object_id,
				  ORBitGServant *servant)
{
	ORBit_gservant_deactivate (servant);

	return TRUE;
}

void
ORBit_GOA_destroy (ORBit_GOA            goa,
		   const CORBA_boolean  wait_for_completion,
		   CORBA_Environment   *ev)
{
	g_return_if_fail (!goa->destroyed);

	/* FIXME: handle wait_for_completion */

	goa->iterating_hash = TRUE;
	g_hash_table_foreach_remove (
		goa->servant_hash, (GHRFunc) ORBit_GOA_servant_remove_foreach, NULL);
	goa->iterating_hash = FALSE;

	ORBit_RootObject_release (goa_singleton);
	goa_singleton = NULL;

	goa->destroyed = TRUE;
}

CORBA_Object
ORBit_GOA_servant_to_reference (ORBit_GOA             goa,
				const ORBit_GServant  gservant,
				CORBA_Environment    *ev)
{
	/* FIXME: exceptions ? */

	return ORBit_gservant_to_reference (gservant);
}

ORBitGServant *
ORBit_GOA_reference_to_servant (ORBit_GOA           goa,
				const CORBA_Object  reference,
				CORBA_Environment  *ev)
{
	/* FIXME: exceptions ? */

	return ORBit_gservant_from_reference (reference);
}
