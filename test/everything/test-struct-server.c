/*
 * test-struct-server.c: implementation of test::StructServer interface.
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
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
 * Authors:
 *	Phil Dawes <philipd@users.sourceforge.net>
 *	Mark McLoughlin <mark@skynet.ie>
 */

#include <config.h>

#include "test-struct-server.h"
#include "constants.h"

struct _TestStructServerPrivate {
	int dummy;
};

static GObjectClass *test_struct_server_parent_class;

static test_FixedLengthStruct
test_struct_server_op_fixed (TestStructServer             *sserver,
			     const test_FixedLengthStruct *inArg,
			     test_FixedLengthStruct       *inoutArg,
			     test_FixedLengthStruct       *outArg,
			     CORBA_Environment            *ev)
{
	test_FixedLengthStruct retval;
	g_assert (inArg->a == constants_SHORT_IN);
	g_assert (inoutArg->a == constants_SHORT_INOUT_IN);
  
	inoutArg->a = constants_SHORT_INOUT_OUT;
	outArg->a   = constants_SHORT_OUT;
	retval.a    = constants_SHORT_RETN;

	return retval;
}



static test_VariableLengthStruct*
test_struct_server_op_variable (TestStructServer                *sserver,
				const test_VariableLengthStruct *inArg,
				test_VariableLengthStruct       *inoutArg,
				test_VariableLengthStruct      **outArg,
				CORBA_Environment               *ev)
{
	test_VariableLengthStruct *retval;
	g_assert (!strcmp (inArg->a,constants_STRING_IN));
	g_assert (!strcmp (inoutArg->a,constants_STRING_INOUT_IN));
  
	*outArg = test_VariableLengthStruct__alloc ();
	retval  = test_VariableLengthStruct__alloc ();
  
	CORBA_free (inoutArg->a);

	inoutArg->a  = CORBA_string_dup (constants_STRING_INOUT_OUT);
	(*outArg)->a = CORBA_string_dup (constants_STRING_OUT);
	retval->a    = CORBA_string_dup (constants_STRING_RETN);
  
	return retval;
}

static test_CompoundStruct *
test_struct_server_op_compound (TestStructServer          *sserver,
				const test_CompoundStruct *inArg,
				test_CompoundStruct       *inoutArg,
				test_CompoundStruct      **outArg,
				CORBA_Environment         *ev)
{
	test_CompoundStruct *retval;
	g_assert (!strcmp (inArg->a.a,constants_STRING_IN));
	g_assert (!strcmp (inoutArg->a.a,constants_STRING_INOUT_IN));
  
	*outArg = test_CompoundStruct__alloc ();
	retval  = test_CompoundStruct__alloc ();
  
	CORBA_free (inoutArg->a.a);

	inoutArg->a.a = CORBA_string_dup (constants_STRING_INOUT_OUT);
	(*outArg)->a.a = CORBA_string_dup (constants_STRING_OUT);
	retval->a.a = CORBA_string_dup (constants_STRING_RETN);
  
	return retval;
}

static void
test_struct_server_op_object_struct (TestStructServer        *sserver,
				     const test_ObjectStruct *inArg,
				     CORBA_Environment       *ev)
{
	CORBA_Object    objref;
	test_StructAny *val;

	objref = CORBA_Object_duplicate (inArg->serv, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	val = test_StructServer_opStructAny (inArg->serv, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	
	CORBA_free (val);

	CORBA_Object_release (objref, ev);
}

static test_StructAny *
test_struct_server_op_struct_any (TestStructServer  *sserver,
				  CORBA_Environment *ev)
{
	test_StructAny   *a = test_StructAny__alloc ();
	static CORBA_long l;

	a->a = CORBA_string_dup (constants_STRING_IN);

	l = constants_LONG_IN;
	a->b._release = FALSE;
	a->b._value = &l;
	a->b._type  = TC_CORBA_long;

	return a;
}

static void
test_struct_server_finalize (GObject *object)
{
	TestStructServer *servant = TEST_STRUCT_SERVER (object);

	g_free (servant->priv);
	servant->priv = NULL;

	test_struct_server_parent_class->finalize (object);
}

static void
test_struct_server_class_init (TestStructServerClass *klass,
			      gpointer              dummy)
{
	GObjectClass *gobject_class = (GObjectClass *) klass;

	test_struct_server_parent_class = g_type_class_peek_parent (klass);

	gobject_class->finalize = test_struct_server_finalize;

        klass->op_fixed         = test_struct_server_op_fixed;
        klass->op_variable      = test_struct_server_op_variable;
        klass->op_compound      = test_struct_server_op_compound;
        klass->op_object_struct = test_struct_server_op_object_struct;
        klass->op_struct_any    = test_struct_server_op_struct_any;

        ORBit_gmethod_register ("opFixed",
				G_OBJECT_CLASS_TYPE (klass),
				test_StructServer_IMETHODS_opFixed, 0,
				G_STRUCT_OFFSET (TestStructServerClass, op_fixed),
				test_StructServer_opFixed__skeleton);

        ORBit_gmethod_register ("opVariable",
				G_OBJECT_CLASS_TYPE (klass),
				test_StructServer_IMETHODS_opVariable, 0,
				G_STRUCT_OFFSET (TestStructServerClass, op_variable),
				test_StructServer_opVariable__skeleton);

        ORBit_gmethod_register ("opCompound",
				G_OBJECT_CLASS_TYPE (klass),
				test_StructServer_IMETHODS_opCompound, 0,
				G_STRUCT_OFFSET (TestStructServerClass, op_compound),
				test_StructServer_opCompound__skeleton);

        ORBit_gmethod_register ("opObjectStruct",
				G_OBJECT_CLASS_TYPE (klass),
				test_StructServer_IMETHODS_opObjectStruct, 0,
				G_STRUCT_OFFSET (TestStructServerClass, op_object_struct),
				test_StructServer_opObjectStruct__skeleton);

        ORBit_gmethod_register ("opStructAny",
				G_OBJECT_CLASS_TYPE (klass),
				test_StructServer_IMETHODS_opStructAny, 0,
				G_STRUCT_OFFSET (TestStructServerClass, op_struct_any),
				test_StructServer_opStructAny__skeleton);
}

static void
test_struct_server_instance_init (TestStructServer      *servant,
				  TestStructServerClass *klass)
{
	servant->priv = g_new0 (TestStructServerPrivate, 1);
}

GType
test_struct_server_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (TestStructServerClass),
			NULL,
			NULL,
			(GClassInitFunc) test_struct_server_class_init,
			NULL,
			NULL,
			sizeof (TestStructServer),
			0,
			(GInstanceInitFunc) test_struct_server_instance_init,
			NULL
		};

		type = g_type_register_static (
				TEST_TYPE_BASIC_SERVER, "TestStructServer", &info, 0);

		ORBit_gtype_register_static (type, &test_StructServer__iinterface);
	}

	return type;
}

TestStructServer *
test_struct_server_new (void)
{
	return g_object_new (TEST_TYPE_STRUCT_SERVER, NULL);
}
