/*
 * test-basic-server.c: implementation of test::BasicServer interface.
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

#include "test-basic-server.h"
#include "constants.h"

struct _TestBasicServerPrivate {
	int dummy;
};

static GObjectClass *test_basic_server_parent_class;

static CORBA_char *
test_basic_server_get_foo (TestBasicServer   *bserver,
			   CORBA_Environment *ev)
{
	return CORBA_string_dup (constants_STRING_RETN);  
}

static void
test_basic_server_set_foo (TestBasicServer   *bserver,
			   const CORBA_char  *val,
			   CORBA_Environment *ev)
{
	g_assert (!strcmp (val, constants_STRING_IN));
}

static CORBA_long
test_basic_server_get_bah (TestBasicServer   *bserver,
			   CORBA_Environment *ev)
{
	return constants_LONG_RETN;
}

static CORBA_char *
test_basic_server_op_string (TestBasicServer    *bserver,
			     const CORBA_char   *inArg, 
			     CORBA_char        **inoutArg,
			     CORBA_char        **outArg,
			     CORBA_Environment  *ev)
{
	g_assert (!strcmp (inArg, constants_STRING_IN));
	g_assert (!strcmp (*inoutArg, constants_STRING_INOUT_IN));
  
	CORBA_free (*inoutArg);
	*inoutArg = CORBA_string_dup (constants_STRING_INOUT_OUT);
	*outArg = CORBA_string_dup (constants_STRING_OUT);

	return CORBA_string_dup (constants_STRING_RETN);
}

static CORBA_long
test_basic_server_op_long (TestBasicServer   *bserver,
			   const CORBA_long   inArg, 
			   CORBA_long        *inoutArg,
			   CORBA_long        *outArg,
			   CORBA_Environment *ev)
{
	g_assert (inArg == constants_LONG_IN);
	g_assert (*inoutArg == constants_LONG_INOUT_IN);
  
	*inoutArg = constants_LONG_INOUT_OUT;
	*outArg = constants_LONG_OUT;;

	return constants_LONG_RETN;
}

static CORBA_long_long
test_basic_server_op_long_long (TestBasicServer       *bserver,
				const CORBA_long_long  inArg, 
				CORBA_long_long       *inoutArg,
				CORBA_long_long       *outArg,
				CORBA_Environment     *ev)
{
	g_assert (inArg == constants_LONG_LONG_IN);
	g_assert (*inoutArg == constants_LONG_LONG_INOUT_IN);
  
	*inoutArg = constants_LONG_LONG_INOUT_OUT;
	*outArg = constants_LONG_LONG_OUT;;

	return constants_LONG_LONG_RETN;
}

static CORBA_float
test_basic_server_op_float (TestBasicServer   *bserver,
			    const CORBA_float  inArg, 
			    CORBA_float       *inoutArg,
			    CORBA_float       *outArg,
			    CORBA_Environment *ev)
{
	g_assert (inArg == constants_FLOAT_IN);
	g_assert (*inoutArg == constants_FLOAT_INOUT_IN);
  
	*inoutArg = constants_FLOAT_INOUT_OUT;
	*outArg = constants_FLOAT_OUT;;

	return constants_FLOAT_RETN;
}

static CORBA_double
test_basic_server_op_double (TestBasicServer    *bserver,
			     const CORBA_double  inArg, 
			     CORBA_double       *inoutArg,
			     CORBA_double       *outArg,
			     CORBA_Environment  *ev)
{
	g_assert (inArg == constants_DOUBLE_IN);
	g_assert (*inoutArg == constants_DOUBLE_INOUT_IN);
  
	*inoutArg = constants_DOUBLE_INOUT_OUT;
	*outArg = constants_DOUBLE_OUT;;

	return constants_DOUBLE_RETN;
}

static CORBA_long_double
test_basic_server_op_long_double (TestBasicServer         *bserver,
				  const CORBA_long_double  inArg, 
				  CORBA_long_double       *inoutArg,
				  CORBA_long_double       *outArg,
				  CORBA_Environment       *ev)
{
	g_assert (inArg == constants_LONG_DOUBLE_IN);
	g_assert (*inoutArg == constants_LONG_DOUBLE_INOUT_IN);
  
	*inoutArg = constants_LONG_DOUBLE_INOUT_OUT;
	*outArg = constants_LONG_DOUBLE_OUT;;

	return constants_LONG_DOUBLE_RETN;
}

static test_AnEnum
test_basic_server_op_enum (TestBasicServer   *bserver,
			   const test_AnEnum  inArg, 
			   test_AnEnum       *inoutArg,
			   test_AnEnum       *outArg,
			   CORBA_Environment *ev)
{
	g_assert (inArg == test_ENUM_IN);
	g_assert (*inoutArg == test_ENUM_INOUT_IN);
  
	*inoutArg = test_ENUM_INOUT_OUT;
	*outArg = test_ENUM_OUT;

	return test_ENUM_RETN;
}

static void
test_basic_server_op_exception (TestBasicServer   *bserver,
				CORBA_Environment *ev)
{
	test_TestException *ex = test_TestException__alloc ();

	ex->reason           = CORBA_string_dup (constants_STRING_IN);
	ex->number           = constants_LONG_IN;
	ex->aseq._buffer     = CORBA_sequence_CORBA_long_allocbuf (1);
	ex->aseq._length     = 1;
	ex->aseq._buffer [0] = constants_LONG_IN;
	ex->factory          = getFactoryInstance(ev);

	CORBA_sequence_set_release (&ex->aseq, CORBA_TRUE);

	CORBA_exception_set (
		ev, CORBA_USER_EXCEPTION, ex_test_TestException,ex);
}

static void
test_basic_server_op_one_way (TestBasicServer   *bserver,
			      const CORBA_char  *str,
			      CORBA_Environment *ev)
{
	g_assert (!strcmp (str, constants_STRING_IN));
}

static void
test_basic_server_test_large_string_seq (TestBasicServer   *bserver,
					 const test_StrSeq *seq,
					 CORBA_Environment *ev)
{
}

static void
test_basic_server_finalize (GObject *object)
{
	TestBasicServer *servant = TEST_BASIC_SERVER (object);

	g_free (servant->priv);
	servant->priv = NULL;

	test_basic_server_parent_class->finalize (object);
}

static void
test_basic_server_class_init (TestBasicServerClass *klass,
			      gpointer              dummy)
{
	GObjectClass *gobject_class = (GObjectClass *) klass;

	test_basic_server_parent_class = g_type_class_peek_parent (klass);

	gobject_class->finalize = test_basic_server_finalize;

        klass->get_foo               = test_basic_server_get_foo;
        klass->set_foo               = test_basic_server_set_foo;
        klass->get_bah               = test_basic_server_get_bah;
        klass->op_string             = test_basic_server_op_string;
        klass->op_long               = test_basic_server_op_long;
        klass->op_long_long          = test_basic_server_op_long_long;
        klass->op_float              = test_basic_server_op_float;
        klass->op_double             = test_basic_server_op_double;
        klass->op_long_double        = test_basic_server_op_long_double;
        klass->op_enum               = test_basic_server_op_enum;
        klass->op_exception          = test_basic_server_op_exception;
        klass->op_one_way            = test_basic_server_op_one_way;
        klass->no_implement          = NULL;
        klass->test_large_string_seq = test_basic_server_test_large_string_seq;

        ORBit_gmethod_register ("_get_foo",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS__get_foo, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, get_foo),
				test_BasicServer__get_foo__skeleton);

        ORBit_gmethod_register ("_set_foo",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS__set_foo, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, set_foo),
				test_BasicServer__set_foo__skeleton);

        ORBit_gmethod_register ("_get_bah",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS__get_bah, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, get_bah),
				test_BasicServer__get_bah__skeleton);

        ORBit_gmethod_register ("opString",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_opString, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, op_string),
				test_BasicServer_opString__skeleton);

        ORBit_gmethod_register ("opLong",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_opLong, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, op_long),
				test_BasicServer_opLong__skeleton);

        ORBit_gmethod_register ("opLongLong",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_opLongLong, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, op_long_long),
				test_BasicServer_opLongLong__skeleton);

        ORBit_gmethod_register ("opFloat",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_opFloat, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, op_float),
				test_BasicServer_opFloat__skeleton);

        ORBit_gmethod_register ("opDouble",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_opDouble, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, op_double),
				test_BasicServer_opDouble__skeleton);

        ORBit_gmethod_register ("opLongDouble",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_opLongDouble, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, op_long_double),
				test_BasicServer_opLongDouble__skeleton);

        ORBit_gmethod_register ("opEnum",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_opEnum, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, op_enum),
				test_BasicServer_opEnum__skeleton);

        ORBit_gmethod_register ("opException",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_opException, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, op_exception),
				test_BasicServer_opException__skeleton);

        ORBit_gmethod_register ("opOneWay",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_opOneWay, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, op_one_way),
				test_BasicServer_opOneWay__skeleton);

        ORBit_gmethod_register ("noImplement",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_noImplement, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, no_implement),
				test_BasicServer_noImplement__skeleton);

        ORBit_gmethod_register ("testLargeStringSeq",
				G_OBJECT_CLASS_TYPE (klass),
				test_BasicServer_IMETHODS_testLargeStringSeq, 0,
				G_STRUCT_OFFSET (TestBasicServerClass, test_large_string_seq),
				test_BasicServer_testLargeStringSeq__skeleton);
}

static void
test_basic_server_instance_init (TestBasicServer      *servant,
				 TestBasicServerClass *klass)
{
	servant->priv = g_new0 (TestBasicServerPrivate, 1);
}

GType
test_basic_server_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (TestBasicServerClass),
			NULL,
			NULL,
			(GClassInitFunc) test_basic_server_class_init,
			NULL,
			NULL,
			sizeof (TestBasicServer),
			0,
			(GInstanceInitFunc) test_basic_server_instance_init,
			NULL
		};

		type = g_type_register_static (
				ORBIT_TYPE_GSERVANT, "TestBasicServer", &info, 0);

		ORBit_gtype_register_static (type, &test_BasicServer__iinterface);
	}

	return type;
}

TestBasicServer *
test_basic_server_new (void)
{
	return g_object_new (TEST_TYPE_BASIC_SERVER, NULL);
}

/* Start of stuff that will be in everything-skels.c */

void
test_BasicServer__get_foo__skeleton(ORBitGServant * _o_servant,
					    gpointer _o_retval,
					    gpointer * _o_args,
					    CORBA_Context _o_ctx,
					    CORBA_Environment * _o_ev,
					    CORBA_string(*_impl__get_foo)
					    (ORBitGServant * _servant,
					     CORBA_Environment * ev))
{
   *(CORBA_string *) _o_retval = _impl__get_foo(_o_servant, _o_ev);
}

void
test_BasicServer__set_foo__skeleton(ORBitGServant * _o_servant,
					    gpointer _o_retval,
					    gpointer * _o_args,
					    CORBA_Context _o_ctx,
					    CORBA_Environment * _o_ev,
					    void (*_impl__set_foo)
					    (ORBitGServant * _servant,
					     const CORBA_char * value,
					     CORBA_Environment * ev))
{
   _impl__set_foo(_o_servant, *(const CORBA_char * *) _o_args[0], _o_ev);
}

void
test_BasicServer__get_bah__skeleton(ORBitGServant * _o_servant,
					    gpointer _o_retval,
					    gpointer * _o_args,
					    CORBA_Context _o_ctx,
					    CORBA_Environment * _o_ev,
					    CORBA_long(*_impl__get_bah)
					    (ORBitGServant * _servant,
					     CORBA_Environment * ev))
{
   *(CORBA_long *) _o_retval = _impl__get_bah(_o_servant, _o_ev);
}

void
test_BasicServer_opString__skeleton(ORBitGServant * _o_servant,
					    gpointer _o_retval,
					    gpointer * _o_args,
					    CORBA_Context _o_ctx,
					    CORBA_Environment * _o_ev,
					    CORBA_string(*_impl_opString)
					    (ORBitGServant * _servant,
					     const CORBA_char * inArg,
					     CORBA_string * inoutArg,
					     CORBA_string * outArg,
					     CORBA_Environment * ev))
{
   *(CORBA_string *) _o_retval =
      _impl_opString(_o_servant, *(const CORBA_char * *) _o_args[0],
		     (CORBA_string *) _o_args[1],
		     *(CORBA_string * *)_o_args[2], _o_ev);
}

void
test_BasicServer_opLong__skeleton(ORBitGServant * _o_servant,
					  gpointer _o_retval,
					  gpointer * _o_args,
					  CORBA_Context _o_ctx,
					  CORBA_Environment * _o_ev,
					  CORBA_long(*_impl_opLong)
					  (ORBitGServant * _servant,
					   const CORBA_long inArg,
					   CORBA_long * inoutArg,
					   CORBA_long * outArg,
					   CORBA_Environment * ev))
{
   *(CORBA_long *) _o_retval =
      _impl_opLong(_o_servant, *(const CORBA_long *) _o_args[0],
		   (CORBA_long *) _o_args[1], *(CORBA_long * *)_o_args[2],
		   _o_ev);
}

void
test_BasicServer_opLongLong__skeleton(ORBitGServant *
					      _o_servant, gpointer _o_retval,
					      gpointer * _o_args,
					      CORBA_Context _o_ctx,
					      CORBA_Environment * _o_ev,
					      CORBA_long_long
					      (*_impl_opLongLong)
					      (ORBitGServant *
					       _servant,
					       const CORBA_long_long inArg,
					       CORBA_long_long * inoutArg,
					       CORBA_long_long * outArg,
					       CORBA_Environment * ev))
{
   *(CORBA_long_long *) _o_retval =
      _impl_opLongLong(_o_servant, *(const CORBA_long_long *) _o_args[0],
		       (CORBA_long_long *) _o_args[1],
		       *(CORBA_long_long * *)_o_args[2], _o_ev);
}

void
test_BasicServer_opFloat__skeleton(ORBitGServant * _o_servant,
					   gpointer _o_retval,
					   gpointer * _o_args,
					   CORBA_Context _o_ctx,
					   CORBA_Environment * _o_ev,
					   CORBA_float(*_impl_opFloat)
					   (ORBitGServant * _servant,
					    const CORBA_float inArg,
					    CORBA_float * inoutArg,
					    CORBA_float * outArg,
					    CORBA_Environment * ev))
{
   *(CORBA_float *) _o_retval =
      _impl_opFloat(_o_servant, *(const CORBA_float *) _o_args[0],
		    (CORBA_float *) _o_args[1], *(CORBA_float * *)_o_args[2],
		    _o_ev);
}

void
test_BasicServer_opDouble__skeleton(ORBitGServant * _o_servant,
					    gpointer _o_retval,
					    gpointer * _o_args,
					    CORBA_Context _o_ctx,
					    CORBA_Environment * _o_ev,
					    CORBA_double(*_impl_opDouble)
					    (ORBitGServant * _servant,
					     const CORBA_double inArg,
					     CORBA_double * inoutArg,
					     CORBA_double * outArg,
					     CORBA_Environment * ev))
{
   *(CORBA_double *) _o_retval =
      _impl_opDouble(_o_servant, *(const CORBA_double *) _o_args[0],
		     (CORBA_double *) _o_args[1],
		     *(CORBA_double * *)_o_args[2], _o_ev);
}

void
test_BasicServer_opLongDouble__skeleton(ORBitGServant *
						_o_servant,
						gpointer _o_retval,
						gpointer * _o_args,
						CORBA_Context _o_ctx,
						CORBA_Environment * _o_ev,
						CORBA_long_double
						(*_impl_opLongDouble)
						(ORBitGServant *
						 _servant,
						 const CORBA_long_double
						 inArg,
						 CORBA_long_double * inoutArg,
						 CORBA_long_double * outArg,
						 CORBA_Environment * ev))
{
   *(CORBA_long_double *) _o_retval =
      _impl_opLongDouble(_o_servant, *(const CORBA_long_double *) _o_args[0],
			 (CORBA_long_double *) _o_args[1],
			 *(CORBA_long_double * *)_o_args[2], _o_ev);
}

void
test_BasicServer_opEnum__skeleton(ORBitGServant * _o_servant,
					  gpointer _o_retval,
					  gpointer * _o_args,
					  CORBA_Context _o_ctx,
					  CORBA_Environment * _o_ev,
					  test_AnEnum(*_impl_opEnum)
					  (ORBitGServant * _servant,
					   const test_AnEnum inArg,
					   test_AnEnum * inoutArg,
					   test_AnEnum * outArg,
					   CORBA_Environment * ev))
{
   *(test_AnEnum *) _o_retval =
      _impl_opEnum(_o_servant, *(const test_AnEnum *) _o_args[0],
		   (test_AnEnum *) _o_args[1], *(test_AnEnum * *)_o_args[2],
		   _o_ev);
}

void
test_BasicServer_opException__skeleton(ORBitGServant *
					       _o_servant, gpointer _o_retval,
					       gpointer * _o_args,
					       CORBA_Context _o_ctx,
					       CORBA_Environment * _o_ev,
					       void (*_impl_opException)
					       (ORBitGServant *
						_servant,
						CORBA_Environment * ev))
{
   _impl_opException(_o_servant, _o_ev);
}

void
test_BasicServer_opOneWay__skeleton(ORBitGServant * _o_servant,
					    gpointer _o_retval,
					    gpointer * _o_args,
					    CORBA_Context _o_ctx,
					    CORBA_Environment * _o_ev,
					    void (*_impl_opOneWay)
					    (ORBitGServant * _servant,
					     const CORBA_char * inArg,
					     CORBA_Environment * ev))
{
   _impl_opOneWay(_o_servant, *(const CORBA_char * *) _o_args[0], _o_ev);
}

void
test_BasicServer_noImplement__skeleton(ORBitGServant *
					       _o_servant, gpointer _o_retval,
					       gpointer * _o_args,
					       CORBA_Context _o_ctx,
					       CORBA_Environment * _o_ev,
					       void (*_impl_noImplement)
					       (ORBitGServant *
						_servant,
						CORBA_Environment * ev))
{
   _impl_noImplement(_o_servant, _o_ev);
}

void
test_BasicServer_testLargeStringSeq__skeleton(ORBitGServant *
						      _o_servant,
						      gpointer _o_retval,
						      gpointer * _o_args,
						      CORBA_Context _o_ctx,
						      CORBA_Environment *
						      _o_ev,
						      void
						      (*_impl_testLargeStringSeq)
						      (ORBitGServant *
						       _servant,
						       const test_StrSeq *
						       seq,
						       CORBA_Environment *
						       ev))
{
   _impl_testLargeStringSeq(_o_servant,
			    (const CORBA_sequence_CORBA_string *) _o_args[0],
			    _o_ev);
}

/* End of stuff that will be in everything-skels.c */
