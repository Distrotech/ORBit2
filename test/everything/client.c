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
 * Author: Phil Dawes <philipd@users.sourceforge.net>
 */

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "everything.h"
#include "constants.h"

#define NUM_RUNS 1

#ifdef TIMING_RUN
#  define d_print(a)
#else
#  define d_print(a) g_print(a)
#endif

extern CORBA_ORB global_orb;
gboolean         in_proc;

/* Ugly- but hey */
#define _IN_CLIENT_
#include "server.c"
#undef  _IN_CLIENT_

static void
testConst (void)
{
	d_print ("Testing constants...\n");
	g_assert (test_CONST_CHAR=='t');
	g_assert (test_CONST_LONG==0x12345678);
	g_assert (test_CONST_LONG_LONG==0x12345678);
	g_assert (strcmp (test_CONST_STRING, "ConstString")==0);
	/* I can never get constant floats to compare properly. */
	/* g_assert (test_CONST_FLOAT==1234.56); */
	g_assert (test_CONST_DOUBLE==1234.5678);
	g_assert (test_FAVORITE_COLOUR==test_red);
}

static void
testAttribute (test_TestFactory factory, 
	       CORBA_Environment *ev)
{  
	test_BasicServer objref;
	CORBA_char      *val;
	CORBA_long       lval;

	d_print ("Testing attributes...\n");
	objref = test_TestFactory_getBasicServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (objref != CORBA_OBJECT_NIL);
	g_assert (CORBA_Object_is_a (objref, "IDL:orbit/test/BasicServer:1.0", ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	val = test_BasicServer__get_foo (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);  
	g_assert (strcmp (val, constants_STRING_RETN)==0);	
	CORBA_free (val);

	test_BasicServer__set_foo (objref, constants_STRING_IN, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	lval = test_BasicServer__get_bah (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);  
	g_assert (lval == constants_LONG_RETN);

	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testString (test_TestFactory   factory, 
	    CORBA_Environment *ev)
{
	test_BasicServer objref;
	const CORBA_char *in;
	CORBA_char *inout, *out, *retn; 

	d_print ("Testing strings...\n");
	objref = test_TestFactory_getBasicServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	in = constants_STRING_IN;
	inout = CORBA_string_dup (constants_STRING_INOUT_IN);
	retn = test_BasicServer_opString (objref, in, &inout, &out, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
  
	g_assert (strcmp (in, constants_STRING_IN)==0);
	g_assert (strcmp (inout, constants_STRING_INOUT_OUT)==0);
	g_assert (strcmp (out, constants_STRING_OUT)==0);
	g_assert (strcmp (retn, constants_STRING_RETN)==0);	

	test_BasicServer_opOneWay (objref, constants_STRING_IN, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
  
	CORBA_free (inout);
	CORBA_free (out);
	CORBA_free (retn);
	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}


static void
testLong (test_TestFactory   factory, 
	  CORBA_Environment *ev)
{
	test_BasicServer objref;
	CORBA_long inArg, inoutArg, outArg, retn;

	d_print ("Testing longs...\n");
	objref = test_TestFactory_getBasicServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	inArg = constants_LONG_IN;
	inoutArg = constants_LONG_INOUT_IN;
	retn = test_BasicServer_opLong (objref, inArg, &inoutArg, &outArg, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (inArg == constants_LONG_IN);
	g_assert (inoutArg == constants_LONG_INOUT_OUT);
	g_assert (outArg == constants_LONG_OUT);
	g_assert (retn == constants_LONG_RETN);

	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testLongLong (test_TestFactory   factory, 
	      CORBA_Environment *ev)
{
	test_BasicServer objref;
	CORBA_long_long inArg, inoutArg, outArg, retn;

	d_print ("Testing long longs...\n");
	objref = test_TestFactory_getBasicServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	inArg = constants_LONG_LONG_IN;
	inoutArg = constants_LONG_LONG_INOUT_IN;
	retn = test_BasicServer_opLongLong (objref, inArg, &inoutArg, &outArg, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (inArg == constants_LONG_LONG_IN);
	g_assert (inoutArg == constants_LONG_LONG_INOUT_OUT);
	g_assert (outArg == constants_LONG_LONG_OUT);
	g_assert (retn == constants_LONG_LONG_RETN);
	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testEnum (test_TestFactory   factory, 
	  CORBA_Environment *ev)
{
	test_BasicServer objref;
	test_AnEnum inArg, inoutArg, outArg, retn;

	d_print ("Testing enums...\n");
	objref = test_TestFactory_getBasicServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	inArg = test_ENUM_IN;
	inoutArg = test_ENUM_INOUT_IN;
	retn = test_BasicServer_opEnum (objref, inArg, &inoutArg, &outArg, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (inArg == test_ENUM_IN);
	g_assert (inoutArg == test_ENUM_INOUT_OUT);
	g_assert (outArg == test_ENUM_OUT);
	g_assert (retn == test_ENUM_RETN);
	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testException (test_TestFactory   factory, 
	       CORBA_Environment *ev)
{
	test_BasicServer    objref;
	test_TestException *ex;
	CORBA_Environment  *cpyev, *rev = ev;

	d_print ("Testing exceptions...\n");

	objref = test_TestFactory_getBasicServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	test_BasicServer_opException (objref, ev);
  
	g_assert (ev->_major == CORBA_USER_EXCEPTION);
	g_assert (strcmp (CORBA_exception_id (ev), ex_test_TestException) == 0);
	ex = CORBA_exception_value (ev);
	g_assert (strcmp (ex->reason, constants_STRING_IN) == 0);
	g_assert (ex->number == constants_LONG_IN);
	g_assert (ex->aseq._length == 1);
	g_assert (ex->aseq._buffer[0] == constants_LONG_IN);

	cpyev = CORBA_exception__copy (ev);
	CORBA_exception_free (ev);
	ev = cpyev;

	g_assert (ev->_major == CORBA_USER_EXCEPTION);
	g_assert (strcmp (CORBA_exception_id (ev), ex_test_TestException) == 0);
/*	FIXME: we can't do this until we get exception data from
	the typelib - and make sure we register all system types
	there too */
/*	ex = CORBA_exception_value (ev);
	g_assert (strcmp (ex->reason, constants_STRING_IN) == 0);
	g_assert (ex->number == constants_LONG_IN);
	g_assert (ex->aseq._length == 1);
	g_assert (ex->aseq._buffer[0] == constants_LONG_IN);*/

	CORBA_free (cpyev);

	CORBA_Object_release (objref, rev);
	g_assert (rev->_major == CORBA_NO_EXCEPTION);
}

static gboolean
find_tc (CORBA_sequence_CORBA_TypeCode *tcs, 
	 const char                    *repo_id)
{
	int i;

	for (i = 0; i < tcs->_length; i++)
		if (!strcmp (tcs->_buffer [i]->repo_id, repo_id))
			return TRUE;

	return FALSE;
}

static void
testIInterface (test_TestFactory   factory, 
		CORBA_Environment *ev)
{
	test_StructServer objref;
	CORBA_char       *type_id;
	ORBit_IInterface *iinterface;
	CORBA_sequence_CORBA_TypeCode *tcs;

	d_print ("Testing IInterface code...\n");
	objref = test_TestFactory_getStructServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	/* Check nil check is working ! */
	g_assert (CORBA_Object_is_nil  (CORBA_OBJECT_NIL, ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (!CORBA_Object_is_nil (objref, ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	/* Check non_existant is working ! */
	g_assert (!CORBA_Object_non_existent (objref, ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	/* Ensure that we go over the wire at least once */
	g_assert (CORBA_Object_is_a (objref, "IDL:orbit/test/StructServer:1.0", ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (CORBA_Object_is_a (objref, "IDL:orbit/test/BasicServer:1.0", ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	/* Scripting stuff */

	/* Get real type id */
	g_assert ( (type_id = ORBit_small_get_type_id (objref, ev)));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (!strcmp (type_id, "IDL:orbit/test/StructServer:1.0"));
	CORBA_free (type_id);

	/* Get interface data */
	iinterface = ORBit_small_get_iinterface (
		objref, "foo_bar_jelly", ev);
	g_assert (ev->_major != CORBA_NO_EXCEPTION);
	g_assert (iinterface == NULL);
	g_assert (!strcmp (ev->_id, ex_ORBit_NoIInterface));
	CORBA_exception_free (ev);

	iinterface = ORBit_small_get_iinterface (
		objref, "IDL:orbit/test/StructServer:1.0", ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (iinterface != NULL);
	g_assert (!strcmp (iinterface->tc->repo_id, "IDL:orbit/test/StructServer:1.0"));
	CORBA_free (iinterface);

	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

#define TYPELIB_NAME "./Everything_module"

	if (!ORBit_small_load_typelib (TYPELIB_NAME))
		g_warning ("Failed to load '" TYPELIB_NAME "'");
	iinterface = ORBit_small_get_iinterface ( 
		objref, "IDL:orbit/test/StructServer:1.0", ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (iinterface != NULL);
	CORBA_free (iinterface);

	tcs = ORBit_small_get_types (TYPELIB_NAME);
	g_assert (find_tc (tcs, "IDL:orbit/test/Colour:1.0"));
	g_assert (find_tc (tcs, "IDL:orbit/test/ArrayUnion:1.0"));
	CORBA_free (tcs);
}

static void
testFixedLengthStruct (test_TestFactory   factory, 
		       CORBA_Environment *ev)
{
	test_StructServer objref;
	test_FixedLengthStruct inArg, inoutArg, outArg, retn;
	CORBA_char *ior;

	d_print ("Testing struct code ...\n");
	ior = test_TestFactory_getStructServerIOR (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
  
	objref = CORBA_ORB_string_to_object (global_orb, ior, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (objref != CORBA_OBJECT_NIL);
	CORBA_free (ior);


	inArg.a = constants_SHORT_IN;
	inoutArg.a = constants_SHORT_INOUT_IN;
  
	retn = test_StructServer_opFixed (objref, &inArg, &inoutArg, &outArg, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
  
	g_assert (inArg.a == constants_SHORT_IN);
	g_assert (inoutArg.a == constants_SHORT_INOUT_OUT);
	g_assert (outArg.a == constants_SHORT_OUT);
	g_assert (retn.a == constants_SHORT_RETN);
	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testVariableLengthStruct (test_TestFactory   factory, 
			  CORBA_Environment *ev)
{
  test_StructServer objref;
  test_VariableLengthStruct inArg, inoutArg, *outArg, *retn;
  d_print ("Testing variable length structs...\n");
  objref = test_TestFactory_getStructServer (factory, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  inArg.a = (CORBA_char*)constants_STRING_IN;  /* const cast */
  inoutArg.a = CORBA_string_dup (constants_STRING_INOUT_IN);
  
  retn = test_StructServer_opVariable (objref, &inArg, &inoutArg, &outArg, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
  
  g_assert (strcmp (inArg.a, constants_STRING_IN)==0);
  g_assert (strcmp (inoutArg.a, constants_STRING_INOUT_OUT)==0);
  g_assert (strcmp (outArg->a, constants_STRING_OUT)==0);
  g_assert (strcmp (retn->a, constants_STRING_RETN)==0);	

  CORBA_free (inoutArg.a);
  CORBA_free (outArg);
  CORBA_free (retn);
  CORBA_Object_release (objref, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
}


static void
testCompoundStruct (test_TestFactory   factory, 
		    CORBA_Environment *ev)
{
  test_StructServer objref;
  test_CompoundStruct inArg, inoutArg, *outArg, *retn;
  d_print ("Testing compound structs...\n");
  objref = test_TestFactory_getStructServer (factory, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  inArg.a.a = CORBA_string_dup (constants_STRING_IN);
  inoutArg.a.a = CORBA_string_dup (constants_STRING_INOUT_IN);
  
  retn = test_StructServer_opCompound (objref, &inArg, &inoutArg, &outArg, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
  
  g_assert (strcmp (inArg.a.a, constants_STRING_IN)==0);
  g_assert (strcmp (inoutArg.a.a, constants_STRING_INOUT_OUT)==0);
  g_assert (strcmp (outArg->a.a, constants_STRING_OUT)==0);
  g_assert (strcmp (retn->a.a, constants_STRING_RETN)==0);	
  
  CORBA_free (inArg.a.a);
  CORBA_free (inoutArg.a.a);
  CORBA_free (outArg);
  CORBA_free (retn);  
  CORBA_Object_release (objref, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testStructAny (test_TestFactory   factory, 
	       CORBA_Environment *ev)
{
	test_StructServer objref;
	test_StructAny   *a;

	d_print ("Testing 'any' structs...\n");
	objref = test_TestFactory_getStructServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	a = test_StructServer_opStructAny (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	g_assert (!strcmp (a->a, constants_STRING_IN));
	g_assert (* (CORBA_long *)a->b._value == constants_LONG_IN);
  
	CORBA_free (a);  

	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testUnboundedSequence (test_TestFactory   factory, 
		       CORBA_Environment *ev)
{
  test_SequenceServer objref;
  test_StrSeq inArg, inoutArg, *outArg, *retn;
  guint i;
  d_print ("Testing unbounded sequences...\n");
  objref = test_TestFactory_getSequenceServer (factory, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
  
  inArg._buffer = CORBA_sequence_CORBA_string_allocbuf (2);
  inArg._length = 2;
  CORBA_sequence_set_release (&inArg, CORBA_TRUE);

  for (i=0;i<inArg._length;i++){
	inArg._buffer[i] = CORBA_string_dup (constants_SEQ_STRING_IN[i]);
  }

  inoutArg._buffer = CORBA_sequence_CORBA_string_allocbuf (2);
  inoutArg._length = 2;
  CORBA_sequence_set_release (&inoutArg, CORBA_TRUE);

  for (i=0;i<inoutArg._length;i++){
	inoutArg._buffer[i] = CORBA_string_dup (constants_SEQ_STRING_INOUT_IN[i]);
  }

  retn = test_SequenceServer_opStrSeq (objref, &inArg, &inoutArg, &outArg, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
  
  for (i=0;i<inArg._length;i++){
	g_assert (strcmp (inArg._buffer[i], constants_SEQ_STRING_IN[i]) == 0);
  }

  for (i=0;i<inoutArg._length;i++){
	g_assert (strcmp (inoutArg._buffer[i], constants_SEQ_STRING_INOUT_OUT[i]) == 0);
  }

  for (i=0;i<outArg->_length;i++){
	g_assert (strcmp (outArg->_buffer[i], constants_SEQ_STRING_OUT[i]) == 0);
  }

  for (i=0;i<retn->_length;i++){
	g_assert (strcmp (retn->_buffer[i], constants_SEQ_STRING_RETN[i]) == 0);
  }

  CORBA_free (inArg._buffer);
  CORBA_free (inoutArg._buffer);
  CORBA_free (outArg);
  CORBA_free (retn);
  CORBA_Object_release (objref, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testBoundedSequence (test_TestFactory   factory, 
		     CORBA_Environment *ev)
{
  test_SequenceServer objref;
  test_BoundedStructSeq inArg, inoutArg, *outArg, *retn;
  guint i;
  d_print ("Testing bounded sequences...\n");
  objref = test_TestFactory_getSequenceServer (factory, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  
  inArg._buffer  = CORBA_sequence_test_CompoundStruct_allocbuf (2);
  inArg._length  = 2;
  inArg._maximum = 2;
  CORBA_sequence_set_release (&inoutArg, CORBA_TRUE);

  for (i=0;i<inArg._length;i++){
	inArg._buffer[i].a.a = CORBA_string_dup (constants_SEQ_STRING_IN[i]);
  }

  inoutArg._buffer  = CORBA_sequence_test_CompoundStruct_allocbuf (2);
  inoutArg._length  = 2;
  inoutArg._maximum = 2;
  CORBA_sequence_set_release (&inoutArg, CORBA_TRUE);

  for (i=0;i<inoutArg._length;i++){
	inoutArg._buffer[i].a.a = CORBA_string_dup (constants_SEQ_STRING_INOUT_IN[i]);
  }

  retn = test_SequenceServer_opBoundedStructSeq (objref, &inArg, &inoutArg, &outArg, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
  

  for (i=0;i<inArg._length;i++){
	g_assert (strcmp (inArg._buffer[i].a.a, constants_SEQ_STRING_IN[i]) == 0);
  }

  for (i=0;i<inoutArg._length;i++){
	g_assert (strcmp (inoutArg._buffer[i].a.a, constants_SEQ_STRING_INOUT_OUT[i]) == 0);
  }

  for (i=0;i<outArg->_length;i++){
	g_assert (strcmp (outArg->_buffer[i].a.a, constants_SEQ_STRING_OUT[i]) == 0);
  }

  for (i=0;i<retn->_length;i++){
	g_assert (strcmp (retn->_buffer[i].a.a, constants_SEQ_STRING_RETN[i]) == 0);
  }

  CORBA_free (inArg._buffer);
  CORBA_free (inoutArg._buffer);
  CORBA_free (outArg);
  CORBA_free (retn);
  CORBA_Object_release (objref, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testFixedLengthUnion (test_TestFactory   factory, 
		      CORBA_Environment *ev)
{
  test_UnionServer objref;
  test_FixedLengthUnion inArg, inoutArg, outArg, retn;
  d_print ("Testing fixed length unions...\n");
  objref = test_TestFactory_getUnionServer (factory, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  inArg._u.x = constants_LONG_IN;
  inArg._d = 'a';

  inoutArg._u.y = 't';
  inoutArg._d = 'b';
  
  retn = test_UnionServer_opFixed (objref, &inArg, &inoutArg, &outArg, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  g_assert (inArg._d == 'a');
  g_assert (inArg._u.x == constants_LONG_IN);
  g_assert (inoutArg._d == 'c');
  g_assert (inoutArg._u.z == TRUE);
  g_assert (outArg._d == 'a');
  g_assert (outArg._u.x == constants_LONG_OUT);
  g_assert (retn._d == 'd');
  g_assert (retn._u.z == FALSE);


  CORBA_Object_release (objref, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testVariableLengthUnion (test_TestFactory   factory, 
			 CORBA_Environment *ev)
{
  test_UnionServer objref;
  test_VariableLengthUnion inArg, inoutArg, *outArg, *retn;
  d_print ("Testing variable length unions...\n");
  objref = test_TestFactory_getUnionServer (factory, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  inArg._u.x = constants_LONG_IN;
  inArg._d = 1;

  inoutArg._u.y = CORBA_string_dup (constants_STRING_INOUT_IN);
  inoutArg._d = 2;
  
  retn = test_UnionServer_opVariable (objref, &inArg, &inoutArg, &outArg, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  g_assert (inArg._d == 1);
  g_assert (inArg._u.x == constants_LONG_IN);
  g_assert (inoutArg._d == 3);
  g_assert (inoutArg._u.z == TRUE);
  g_assert (outArg->_d == 1);
  g_assert (outArg->_u.x == constants_LONG_OUT);
  g_assert (retn->_d == 4);
  g_assert (retn->_u.z == FALSE);

  CORBA_free (outArg);
  CORBA_free (retn);

  CORBA_Object_release (objref, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testFixedLengthArray (test_TestFactory   factory, 
		      CORBA_Environment *ev)
{
  
  test_ArrayServer objref;
  test_LongArray inArg, inoutArg, outArg;
  test_LongArray_slice *retn;
  int i;
  d_print ("Testing arrays with fixed length members...\n");
  objref = test_TestFactory_getArrayServer (factory, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  for (i=0;i<constants_SEQLEN;i++)
	inArg[i] = constants_SEQ_LONG_IN[i];

  for (i=0;i<constants_SEQLEN;i++)
	inoutArg[i] = constants_SEQ_LONG_INOUT_IN[i];

  retn = test_ArrayServer_opLongArray (objref, inArg, inoutArg, outArg, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  for (i=0;i<constants_SEQLEN;i++)
	g_assert (inArg[i]==constants_SEQ_LONG_IN[i]);
  for (i=0;i<constants_SEQLEN;i++)
	g_assert (inoutArg[i]==constants_SEQ_LONG_INOUT_OUT[i]);
  for (i=0;i<constants_SEQLEN;i++)
	g_assert (outArg[i]==constants_SEQ_LONG_OUT[i]);
  for (i=0;i<constants_SEQLEN;i++)
	g_assert (retn[i]==constants_SEQ_LONG_RETN[i]);

  CORBA_free (retn);
  CORBA_Object_release (objref, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testVariableLengthArray (test_TestFactory   factory, 
			 CORBA_Environment *ev)
{
  test_ArrayServer objref;
  test_StrArray inArg, inoutArg;
  test_StrArray_slice *outArg, *retn;
  test_StrArrayMultiDimensional_slice *multidim;
  int i, n0, n1, n2;
  
  d_print ("Testing arrays with variable length members...\n");
  objref = test_TestFactory_getArrayServer (factory, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  for (i=0;i<constants_SEQLEN;i++)
	inArg[i] = CORBA_string_dup (constants_SEQ_STRING_IN[i]);

  for (i=0;i<constants_SEQLEN;i++)
	inoutArg[i] = CORBA_string_dup (constants_SEQ_STRING_INOUT_IN[i]);

  retn = test_ArrayServer_opStrArray (objref, inArg, inoutArg, &outArg, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  for (i=0;i<constants_SEQLEN;i++)
	CORBA_free (inArg[i]); 

  for (i=0;i<constants_SEQLEN;i++)
        CORBA_free (inoutArg[i]);

  CORBA_free (outArg);
  CORBA_free (retn);
  CORBA_Object_release (objref, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  
  multidim = test_StrArrayMultiDimensional__alloc ();
  for (n0 = 0; n0 < 2; n0++) {
	for (n1 = 0; n1 < 3; n1++) {
	  for (n2 = 0; n2 < 5; n2++) {
		multidim[n0][n1][n2] = CORBA_string_dup (constants_SEQ_STRING_INOUT_IN[0]);
	  }
	}
  }
  CORBA_free (multidim);

}


static void
testAnyLong (test_TestFactory   factory, 
	     CORBA_Environment *ev)
{
  test_AnyServer objref;
  CORBA_any inArg, inoutArg, *outArg, *retn;
  CORBA_long tmp, tmp1;

  d_print ("Testing any with longs...\n");
  objref = test_TestFactory_getAnyServer (factory, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  tmp = constants_LONG_IN;
  inArg._type = (CORBA_TypeCode)TC_CORBA_long;
  inArg._value = &tmp;
  CORBA_any_set_release (&inArg, CORBA_FALSE);

  inoutArg._type = (CORBA_TypeCode)TC_CORBA_long;
  tmp1 = constants_LONG_INOUT_IN;
  inoutArg._value = &tmp1;
  CORBA_any_set_release (&inoutArg, CORBA_FALSE);
    
  retn = test_AnyServer_opAnyLong (objref, &inArg, &inoutArg, &outArg, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);

  g_assert ( CORBA_TypeCode_equal (inArg._type, TC_CORBA_long, ev) );
  g_assert (* (CORBA_long*)inArg._value == constants_LONG_IN);

  g_assert ( CORBA_TypeCode_equal (inoutArg._type, TC_CORBA_long, ev) );
  g_assert (* (CORBA_long*)inoutArg._value == constants_LONG_INOUT_OUT);

  g_assert ( CORBA_TypeCode_equal (outArg->_type, TC_CORBA_long, ev) );
  g_assert (* (CORBA_long*)outArg->_value == constants_LONG_OUT);

  g_assert ( CORBA_TypeCode_equal (retn->_type, TC_CORBA_long, ev) );
  g_assert (* (CORBA_long*)retn->_value == constants_LONG_RETN);

  if (CORBA_any_get_release (&inArg)){
	CORBA_free (inArg._value);
	CORBA_Object_release ((CORBA_Object)inArg._type, ev);
  }

  if (CORBA_any_get_release (&inoutArg)){
	CORBA_free (inoutArg._value);
	CORBA_Object_release ((CORBA_Object)inoutArg._type, ev);
  }

  CORBA_free (outArg);
  CORBA_free (retn);

  CORBA_Object_release (objref, ev);
  g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testAnyString (test_TestFactory   factory, 
	       CORBA_Environment *ev)
{
	test_AnyServer objref;
	CORBA_any inArg, inoutArg, *outArg, *retn;

	d_print ("Testing any with strings...\n");
	objref = test_TestFactory_getAnyServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	inArg._type = (CORBA_TypeCode)TC_CORBA_string;
	inArg._value = &constants_STRING_IN;
	CORBA_any_set_release (&inArg, CORBA_FALSE);

	inoutArg._type = (CORBA_TypeCode)TC_CORBA_string;
	inoutArg._value = &constants_STRING_INOUT_IN;
	CORBA_any_set_release (&inoutArg, CORBA_FALSE);
  
	retn = test_AnyServer_opAnyString (objref, &inArg, &inoutArg, &outArg, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	g_assert (CORBA_TypeCode_equal (inArg._type, TC_CORBA_string, ev));
	g_assert (strcmp (* (CORBA_char **)inArg._value, constants_STRING_IN) == 0);

	g_assert (CORBA_TypeCode_equal (inoutArg._type, TC_CORBA_string, ev) );
	g_assert (strcmp (* (CORBA_char **)inoutArg._value, constants_STRING_INOUT_OUT) == 0);

	g_assert (CORBA_TypeCode_equal (outArg->_type, TC_CORBA_string, ev) );
	g_assert (strcmp (* (CORBA_char **)outArg->_value, constants_STRING_OUT) == 0);

	g_assert (CORBA_TypeCode_equal (retn->_type, TC_CORBA_string, ev) );
	g_assert (strcmp (* (CORBA_char **)retn->_value, constants_STRING_RETN) == 0);

	if (CORBA_any_get_release (&inArg)){
		CORBA_free (inArg._value);
		CORBA_Object_release ((CORBA_Object)inArg._type, ev);
	}

	if (CORBA_any_get_release (&inoutArg)){
		CORBA_free (inoutArg._value);
		CORBA_Object_release ((CORBA_Object)inoutArg._type, ev);
	}

	CORBA_free (outArg);
	CORBA_free (retn);

	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testAnyStrSeq (test_TestFactory   factory, 
	       CORBA_Environment *ev)
{
	test_AnyServer objref;
	CORBA_any     *retn;

	d_print ("Testing any with string sequences ...\n");

	objref = test_TestFactory_getAnyServer(factory,ev);
	g_assert(ev->_major == CORBA_NO_EXCEPTION);

	retn = test_AnyServer_opAnyStrSeq (objref, ev);
	g_assert(ev->_major == CORBA_NO_EXCEPTION);

	CORBA_free (retn);

	CORBA_Object_release (objref, ev);
}

static void
testAnyStruct (test_TestFactory   factory, 
	       CORBA_Environment *ev)
{
	test_AnyServer objref;
	CORBA_any inArg, inoutArg, *outArg, *retn;
	test_VariableLengthStruct inArgStruct; 
	test_VariableLengthStruct * inoutArgStruct;

	d_print ("Testing any with structs...\n");

	objref = test_TestFactory_getAnyServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	inoutArgStruct = test_VariableLengthStruct__alloc ();
	inArgStruct.a= (CORBA_char*)constants_STRING_IN; /* const cast */
	inArg._type = (CORBA_TypeCode)TC_test_VariableLengthStruct;
	inArg._value = &inArgStruct;
	CORBA_any_set_release (&inArg, CORBA_FALSE);


	inoutArgStruct->a = CORBA_string_dup (constants_STRING_INOUT_IN);
	inoutArg._type = (CORBA_TypeCode)TC_test_VariableLengthStruct;
	inoutArg._value = inoutArgStruct;
	CORBA_any_set_release (&inoutArg, CORBA_TRUE);

	retn = test_AnyServer_opAnyStruct (objref, &inArg, &inoutArg, &outArg, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

  
	g_assert (CORBA_TypeCode_equal (inArg._type, TC_test_VariableLengthStruct, ev));
	g_assert (strcmp ((* (test_VariableLengthStruct*)inArg._value).a, constants_STRING_IN) == 0);

	g_assert (CORBA_TypeCode_equal (inoutArg._type, TC_test_VariableLengthStruct, ev) );
	g_assert (strcmp ((* (test_VariableLengthStruct*)inoutArg._value).a, constants_STRING_INOUT_OUT) == 0);

	g_assert (CORBA_TypeCode_equal (outArg->_type, TC_test_VariableLengthStruct, ev) );
	g_assert (strcmp ((* (test_VariableLengthStruct*)outArg->_value).a, constants_STRING_OUT) == 0);

	g_assert (CORBA_TypeCode_equal (retn->_type, TC_test_VariableLengthStruct, ev) );
	g_assert (strcmp ((* (test_VariableLengthStruct*)retn->_value).a, constants_STRING_RETN) == 0);


	if (CORBA_any_get_release (&inArg)){
		/* This shouldn't be called */
		CORBA_free (inArg._value);
		CORBA_Object_release ((CORBA_Object)inArg._type, ev);
	}

	if (CORBA_any_get_release (&inoutArg)){
		CORBA_free (inoutArg._value);
		CORBA_Object_release ((CORBA_Object)inoutArg._type, ev);
	}

	CORBA_free (outArg);
	CORBA_free (retn);

	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

}

static void
testSequenceOfAny (test_TestFactory   factory, 
		   CORBA_Environment *ev)
{
	/* this test just checks the memory management for seq of any */

	test_AnySeq anyseq;
	int i;

	d_print ("Testing Sequence of Any...\n");

	anyseq._buffer  = CORBA_sequence_CORBA_any_allocbuf (2);
	anyseq._length  = 2;
	CORBA_sequence_set_release (&anyseq, CORBA_TRUE);
  
	for (i = 0; i < anyseq._length; i++) {
		anyseq._buffer [i]._type = (CORBA_TypeCode) TC_CORBA_string;
		anyseq._buffer [i]._value = &constants_STRING_IN;
		CORBA_any_set_release (
			&anyseq._buffer[i], CORBA_FALSE);
	}
  
	CORBA_free (anyseq._buffer);
}

static void
testAnyException (test_TestFactory   factory, 
		  CORBA_Environment *ev)
{
	CORBA_any *inArg; 
	test_TestException *testex;

	inArg = CORBA_any__alloc ();
	testex = test_TestException__alloc ();  
	inArg->_type = (CORBA_TypeCode) TC_test_TestException;
	inArg->_value = testex;
	CORBA_any_set_release (inArg, CORBA_TRUE);

	CORBA_free (inArg);
}

static void
testTypeCode (test_TestFactory   factory, 
	      CORBA_Environment *ev)
{
	test_AnyServer objref;
	CORBA_TypeCode inArg, inoutArg, outArg, retn;

	d_print ("Testing TypeCodes...\n");
	objref = test_TestFactory_getAnyServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	inArg = TC_test_ArrayUnion;
	inoutArg = TC_test_AnyServer;
  
	retn = test_AnyServer_opTypeCode (objref, inArg, &inoutArg, &outArg, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	g_assert (CORBA_TypeCode_equal (inArg, TC_test_ArrayUnion, ev));
	g_assert (CORBA_TypeCode_equal (inoutArg, TC_test_TestException, ev));
	g_assert (CORBA_TypeCode_equal (outArg, TC_test_AnEnum, ev));  
	g_assert (CORBA_TypeCode_equal (retn, TC_test_VariableLengthStruct, ev));

	CORBA_Object_release ((CORBA_Object)inArg, ev);
	CORBA_Object_release ((CORBA_Object)inoutArg, ev);
	CORBA_Object_release ((CORBA_Object)outArg, ev);
	CORBA_Object_release ((CORBA_Object)retn, ev);

	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testContext (test_TestFactory   factory, 
	     CORBA_Environment *ev)
{
	test_ContextServer objref;
	CORBA_Object inArg, inoutArg, outArg, retn;
	CORBA_Context ctx;

	d_print ("Testing Contexts...\n");

	objref = test_TestFactory_getContextServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	inArg = inoutArg = outArg = retn = CORBA_OBJECT_NIL;

	CORBA_Context_create_child (CORBA_OBJECT_NIL, "Whatever", &ctx, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	CORBA_Context_set_one_value (ctx, "foo", "foo1", ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	CORBA_Context_set_one_value (ctx, "foo", "foo2", ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	CORBA_Context_set_one_value (ctx, "bar", "baaaa", ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	retn = test_ContextServer_opWithContext (
		objref, inArg, &inoutArg, &outArg, ctx, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	CORBA_Object_release (retn, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	CORBA_Object_release (inArg, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	CORBA_Object_release (inoutArg, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	CORBA_Object_release (outArg, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	CORBA_Object_release ( (CORBA_Object)ctx, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	CORBA_Object_release (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testMisc (test_TestFactory   factory, 
	  CORBA_Environment *ev)
{
	CORBA_char   *foo;
	CORBA_Context ctx;

	d_print ("Testing Misc bits...\n");

	if (!in_proc) {
		/* Invoke a BasicServer method on a TestFactory */
		foo = test_BasicServer__get_foo (factory, ev);
		g_assert (ev->_major == CORBA_SYSTEM_EXCEPTION);
		g_assert (!strcmp (ev->_id, "IDL:CORBA/BAD_OPERATION:1.0"));
		CORBA_exception_free (ev);
	}
	
	if (!in_proc) {
		test_BasicServer objref;

		objref = test_TestFactory_getBasicServer (factory, ev);
		g_assert (ev->_major == CORBA_NO_EXCEPTION);
		g_assert (objref != CORBA_OBJECT_NIL);
		g_assert (CORBA_Object_is_a (objref, "IDL:orbit/test/BasicServer:1.0", ev));
		g_assert (ev->_major == CORBA_NO_EXCEPTION);

		test_BasicServer_noImplement (objref, ev);
		g_assert (ev->_major == CORBA_SYSTEM_EXCEPTION);
		g_assert (!strcmp (ev->_id, "IDL:CORBA/NO_IMPLEMENT:1.0"));
		CORBA_exception_free (ev);

		CORBA_Object_release (objref, ev);
		g_assert (ev->_major == CORBA_NO_EXCEPTION);
	}
		
	/* Check we are building full type data */
	if (strcmp (TC_test_ObjectStruct->subtypes [0]->repo_id, 
		    "IDL:orbit/test/DerivedServer:1.0"))
		g_warning ("Martin's bug needs fixing");
	
	/* Check a daft one seen in CORBA_exception_free */
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
			     ex_test_SimpleException, NULL);
	CORBA_exception_free (ev);
	g_assert (ev->_id == NULL);
	g_assert (ev->_any._value == NULL);

	/* TypeCode equal */
	g_assert (CORBA_TypeCode_equal (
		TC_CORBA_string, TC_CORBA_string, ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (!CORBA_TypeCode_equal (
		TC_test_StrSeq, TC_test_AnotherStrSeq, ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	/* TypeCode equivalent */
	g_assert (CORBA_TypeCode_equivalent (
		TC_CORBA_string, TC_CORBA_string, ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (CORBA_TypeCode_equivalent (
		TC_test_StrSeq, TC_test_AnotherStrSeq, ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	/*
	 * dead reference check
	 */
	if (!in_proc) {
		test_DeadReferenceObj obj;

		obj = test_TestFactory_createDeadReferenceObj (factory, ev);
		g_assert (ev->_major == CORBA_NO_EXCEPTION);
		g_assert (obj != CORBA_OBJECT_NIL);

		test_DeadReferenceObj_test (obj, ev);
		g_assert (ev->_major == CORBA_NO_EXCEPTION);

		CORBA_Object_release (obj, ev);
		g_assert (ev->_major == CORBA_NO_EXCEPTION);
	} else { /* Lets do some things on a de-activated ref */
		test_BasicServer         obj;
		PortableServer_ObjectId *oid;
		POA_test_BasicServer     servant = {
			NULL, &BasicServer_vepv
		};

		obj = create_object (
			global_poa, POA_test_BasicServer__init,
			&servant, ev);

		oid = PortableServer_POA_servant_to_id (
			global_poa, &servant, ev);
		g_assert (ev->_major == CORBA_NO_EXCEPTION);
		PortableServer_POA_deactivate_object (
			global_poa, oid, ev);
		g_assert (ev->_major == CORBA_NO_EXCEPTION);
	
		CORBA_free (oid);

		test_BasicServer_opException (obj, ev);
		g_assert (ev->_major == CORBA_SYSTEM_EXCEPTION);
		g_assert (!strcmp (ev->_id, ex_CORBA_OBJECT_NOT_EXIST));
		CORBA_exception_free (ev);

		test_BasicServer_opOneWay (obj, "Foo", ev);
		g_assert (ev->_major == CORBA_SYSTEM_EXCEPTION);
		g_assert (!strcmp (ev->_id, ex_CORBA_OBJECT_NOT_EXIST));
		CORBA_exception_free (ev);

		CORBA_Object_release (obj, ev);
		g_assert (ev->_major == CORBA_NO_EXCEPTION);
	}

	/* Check the ORB cleans up contexts properly */
	CORBA_ORB_get_default_context (
		global_orb, &ctx, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (ctx != CORBA_OBJECT_NIL);
	CORBA_Object_release ((CORBA_Object) ctx, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	/* Check that various bonobo hooks work */
	g_assert (ORBit_small_get_servant (NULL) == NULL);
	if (in_proc) {
		g_assert (ORBit_small_get_servant (factory));
		g_assert (ORBit_small_get_connection_status (factory) ==
			  ORBIT_CONNECTION_IN_PROC);
	} else {
		g_assert (!ORBit_small_get_servant (factory));
		g_assert (ORBit_small_get_connection_status (factory) ==
			  ORBIT_CONNECTION_CONNECTED);
	}
}

static int done = 0;

static void
test_BasicServer_opExceptionA_cb (CORBA_Object          object, 
				  ORBit_IMethod        *m_data, 
				  ORBitAsyncQueueEntry *aqe, 
				  gpointer              user_data, 
				  CORBA_Environment    *ev)
{
	test_TestException *ex;

	/* Not a broken connection */
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	ORBit_small_demarshal_async (aqe, NULL, NULL, ev);

	g_assert (ev->_major == CORBA_USER_EXCEPTION);
	g_assert (strcmp (CORBA_exception_id (ev), ex_test_TestException) == 0);

	ex = CORBA_exception_value (ev);
	g_assert (strcmp (ex->reason, constants_STRING_IN) == 0);
	g_assert (ex->number == constants_LONG_IN);
	g_assert (ex->aseq._length == 1);
	g_assert (ex->aseq._buffer[0] == constants_LONG_IN);

	done = 1;
}


static void
test_BasicServer_opExceptionA (CORBA_Object       obj, 
			       CORBA_Environment *ev)
{
	ORBit_IMethod *m_data;

	m_data = &test_BasicServer__iinterface.methods._buffer[7];
	/* if this failed, we re-ordered the IDL ... */
	g_assert (!strcmp (m_data->name, "opException"));

	ORBit_small_invoke_async (
		obj, m_data, test_BasicServer_opExceptionA_cb, 
		NULL, NULL, NULL, ev);
}

static void
test_BasicServer_opStringA_cb (CORBA_Object          object, 
			       ORBit_IMethod        *m_data, 
			       ORBitAsyncQueueEntry *aqe, 
			       gpointer              user_data, 
			       CORBA_Environment    *ev)
{
	CORBA_char  *inout_str = NULL, *out_str, *ret_str;
	CORBA_char **out_str_shim = &out_str;

	gpointer args[] = { NULL, &inout_str, &out_str_shim };
	gpointer ret    = &ret_str;

	/* Not a broken connection */
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	ORBit_small_demarshal_async (aqe, ret, args, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
  
	g_assert (!strcmp (inout_str, constants_STRING_INOUT_OUT));
	g_assert (!strcmp (out_str, constants_STRING_OUT));
	g_assert (!strcmp (ret_str, constants_STRING_RETN));

	CORBA_free (inout_str);
	CORBA_free (out_str);
	CORBA_free (ret_str);

	done = 1;
}

static void
test_BasicServer_opStringA (CORBA_Object       obj, 
			    const CORBA_char  *in_str, 
			    const CORBA_char  *inout_str, 
			    CORBA_Environment *ev)
{
	gpointer args[] = { &in_str, &inout_str, NULL };
	ORBit_IMethod *m_data;

	m_data = &test_BasicServer__iinterface.methods._buffer[3];
	/* if this failed, we re-ordered the IDL ... */
	g_assert (!strcmp (m_data->name, "opString"));

	ORBit_small_invoke_async (
		obj, m_data, test_BasicServer_opStringA_cb, 
		NULL, args, NULL, ev);
}


static void
testAsync (test_TestFactory   factory, 
	   CORBA_Environment *ev)
{
	test_BasicServer objref;

	d_print ("Testing Async invocations ...\n");
	objref = test_TestFactory_getBasicServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	if (in_proc) {
		g_assert (objref->profile_list == NULL);
		g_assert (ORBit_object_get_connection (objref) == NULL);
	}
			      
	done = 0;
	test_BasicServer_opStringA (
		objref, constants_STRING_IN, 
		constants_STRING_INOUT_IN, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	/* While waiting do some normal methods */
	testString (factory, ev);

	while (!done)
		linc_main_iteration (TRUE);

	done = 0;
	test_BasicServer_opExceptionA (objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	while (!done)
		linc_main_iteration (TRUE);

	CORBA_Object_release (objref, ev);  
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
testPingPong (test_TestFactory   factory, 
	      CORBA_Environment *ev)
{
	test_PingPongServer r_objref, l_objref, objref;

	d_print ("Testing ping pong invocations ...\n");
	r_objref = test_TestFactory_createPingPongServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	l_objref = TestFactory_createPingPongServer (NULL, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	CORBA_Object_release (l_objref, ev); /* only want 1 ref */
	g_assert (ORBit_small_get_servant (l_objref) != NULL);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	test_PingPongServer_pingPong (r_objref, l_objref, 64, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	
	d_print ("Testing ping pong reg / lookup ...\n");
	test_PingPongServer_set (r_objref, l_objref, "Foo", ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	
	objref = test_PingPongServer_get (r_objref, "Foo", ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (ORBit_small_get_servant (objref) != NULL);
	g_assert (l_objref == objref);
	CORBA_Object_release (objref, ev);

	CORBA_Object_release (l_objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	CORBA_Object_release (r_objref, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
}

static void
broken_cb (LINCConnection *connection, gboolean *broken)
{
	*broken = TRUE;
}


static void
dummy_cb (LINCConnection *connection, gboolean *invoked)
{
	*invoked = TRUE;
}

static void
testSegv (test_TestFactory   factory, 
	  CORBA_Environment *ev)
{
	d_print ("Testing Fatal invocations ...\n");
	if (!in_proc) {
		gboolean broken = FALSE;
		gboolean invoked = FALSE;
		CORBA_char *id;

		g_assert (ORBit_small_listen_for_broken (
			factory, G_CALLBACK (broken_cb), &broken) ==
			  ORBIT_CONNECTION_CONNECTED);

		g_assert (ORBit_small_listen_for_broken (
			factory, G_CALLBACK (dummy_cb), &invoked) ==
			  ORBIT_CONNECTION_CONNECTED);

		g_assert (ORBit_small_unlisten_for_broken (
			factory, G_CALLBACK (dummy_cb)) ==
			  ORBIT_CONNECTION_CONNECTED);

		id = test_TestFactory_segv (factory, "do it!", ev); 
#ifdef DO_HARDER_SEGV
		g_assert (ev->_major == CORBA_SYSTEM_EXCEPTION);
		g_assert (!strcmp (ev->_id, "IDL:CORBA/COMM_FAILURE:1.0"));
		CORBA_exception_free (ev);

		g_assert (ORBit_small_get_connection_status (factory) ==
			  ORBIT_CONNECTION_DISCONNECTED);
		g_assert (broken);
		g_assert (!invoked);
#else
		g_assert (ORBit_small_unlisten_for_broken (
			factory, G_CALLBACK (broken_cb)) ==
			  ORBIT_CONNECTION_CONNECTED);
		g_assert (ev->_major == CORBA_NO_EXCEPTION);
		CORBA_free (id);
#endif
	}
}

static void
test_initial_references (CORBA_ORB          orb,
			 CORBA_Environment *ev)
{
	CORBA_ORB_ObjectIdList *list;
	int                     i;

	fprintf (stderr, "\nInitial References:\n");

	list = CORBA_ORB_list_initial_services (orb, ev);

	g_assert (ev->_major == CORBA_NO_EXCEPTION && list && list->_length);

	for (i = 0; i < list->_length; i++) {
		CORBA_ORB_ObjectId id;
		CORBA_Object       obj;

		id = list->_buffer [i];

		g_assert (id);

		fprintf (stderr, "\t%s ... ", id);

		obj = CORBA_ORB_resolve_initial_references (orb, id, ev);

		g_assert (ev->_major == CORBA_NO_EXCEPTION && obj != CORBA_OBJECT_NIL);

		CORBA_Object_release (obj, ev);

		g_assert (ev->_major == CORBA_NO_EXCEPTION);

		fprintf (stderr, "okay\n");
	}

	CORBA_free (list);
}

static void
run_tests (test_TestFactory   factory, 
	   CORBA_Environment *ev)
{
	int i;

	for (i = 0; i < NUM_RUNS; i++) {
		testConst ();
		testAttribute (factory, ev);
		testString (factory, ev);
		testLong (factory, ev);
		testLongLong (factory, ev);
		testEnum (factory, ev);
		testException (factory, ev);
		testFixedLengthStruct (factory, ev);
		testVariableLengthStruct (factory, ev);
		testCompoundStruct (factory, ev);
		testStructAny (factory, ev);
		testUnboundedSequence (factory, ev);
		testBoundedSequence (factory, ev);
		testFixedLengthUnion (factory, ev);
		testVariableLengthUnion (factory, ev);
		testFixedLengthArray (factory, ev);
		testVariableLengthArray (factory, ev);
		testAnyStrSeq (factory, ev);
		testAnyLong (factory, ev);
		testAnyString (factory, ev);
		testAnyStruct (factory, ev);
		testAnyException (factory, ev);
		testSequenceOfAny (factory, ev);
		testTypeCode (factory, ev);
		testContext (factory, ev);
		testIInterface (factory, ev);
		testMisc (factory, ev);
		testAsync (factory, ev);
		if (!in_proc)
			testPingPong (factory, ev);
#if NUM_RUNS == 1
		testSegv (factory, ev);
#endif
	}
	
#if NUM_RUNS > 1
	g_warning ("Did '%d' iterations", i);
#endif
}

int
main (int argc, char *argv [])
{
	CORBA_Environment ev;
	test_TestFactory  factory;

	CORBA_exception_init (&ev);

	/* Tell linc we want a threaded ORB */
	linc_set_threaded (TRUE);

	global_orb = CORBA_ORB_init (&argc, argv, "", &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	test_initial_references (global_orb, &ev);

	free (malloc (8)); /* -lefence */

	/* In Proc ... */
	in_proc = TRUE;

	fprintf (stderr, "\n --- In proc ---\n\n\n");
	factory = get_server (global_orb, &ev);
	g_assert (factory->profile_list == NULL);
	g_assert (ORBit_object_get_connection (factory) == NULL);
	run_tests (factory, &ev);
	CORBA_Object_release (factory, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	factory = CORBA_OBJECT_NIL;

	fprintf (stderr, "\n\n --- Out of proc ---\n\n\n");
	in_proc = FALSE;

	{ /* read the ior from iorfile, and swizzle to an objref*/
		int   size;
		char  ior [1024];
		FILE *infile = fopen ("iorfile", "rb");

		if (!infile)
			g_error ("Start the server before running the client");

		size = fread (ior, 1, 1024, infile);
		fclose (infile);
		ior [size] = '\0';   /* insure that string is terminated correctly */

		factory = CORBA_ORB_string_to_object (global_orb, ior, &ev);
		g_assert (ev._major == CORBA_NO_EXCEPTION);

		if (CORBA_Object_non_existent (factory, &ev))
			g_error  ("Start the server before running the client");
		g_assert (ev._major == CORBA_NO_EXCEPTION);
	}

	run_tests (factory, &ev);

	CORBA_Object_release (factory, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	g_warning ("released factory");

	CORBA_Object_release ((CORBA_Object) global_poa, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	
	CORBA_ORB_destroy (global_orb, &ev);
	CORBA_exception_free (&ev);

	CORBA_Object_release ((CORBA_Object) global_orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	CORBA_exception_free (&ev);

	d_print ("All tests passed successfully\n");

	return 0;
}
