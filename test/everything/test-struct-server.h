/*
 * test-struct-server.h: implementation of test::StructServer interface.
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
 *	Mark McLoughlin <mark@skynet.ie>
 */

#ifndef __TEST_STRUCT_SERVER_H__
#define __TEST_STRUCT_SERVER_H__

#include <glib/gmacros.h>
#include <orbit/orbit.h>

#include "everything.h"
#include "test-basic-server.h"

G_BEGIN_DECLS

#define TEST_TYPE_STRUCT_SERVER         (test_struct_server_get_type ())
#define TEST_STRUCT_SERVER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TEST_TYPE_STRUCT_SERVER, TestStructServer))
#define TEST_STRUCT_SERVER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TEST_TYPE_STRUCT_SERVER, TestStructServerClass))
#define TEST_IS_STRUCT_SERVER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TEST_TYPE_STRUCT_SERVER))
#define TEST_IS_STRUCT_SERVER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TEST_TYPE_STRUCT_SERVER))
#define TEST_STRUCT_SERVER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TEST_TYPE_STRUCT_SERVER, TestStructServerClass))

typedef struct _TestStructServer        TestStructServer;
typedef struct _TestStructServerClass   TestStructServerClass;
typedef struct _TestStructServerPrivate TestStructServerPrivate;

struct _TestStructServer {
	TestBasicServer           parent;

	TestStructServerPrivate  *priv;
};

struct _TestStructServerClass {
	TestBasicServerClass         parent_class;

	test_FixedLengthStruct     (*op_fixed)         (TestStructServer                  *sserver,
							const test_FixedLengthStruct      *inArg,
							test_FixedLengthStruct            *inoutArg,
							test_FixedLengthStruct            *outArg,
							CORBA_Environment                 *ev);
	test_VariableLengthStruct *(*op_variable)      (TestStructServer                 *sserver,
							const test_VariableLengthStruct  *inArg,
							test_VariableLengthStruct        *inoutArg,
							test_VariableLengthStruct       **outArg,
							CORBA_Environment                *ev);
	test_CompoundStruct       *(*op_compound)      (TestStructServer                 *sserver,
							const test_CompoundStruct        *inArg,
							test_CompoundStruct              *inoutArg,
							test_CompoundStruct             **outArg,
							CORBA_Environment                *ev);
	void                       (*op_object_struct) (TestStructServer                 *sserver,
							const test_ObjectStruct          *inArg,
							CORBA_Environment                *ev);
	test_StructAny            *(*op_struct_any)    (TestStructServer                 *sserver,
							CORBA_Environment                *ev);
};

GType             test_struct_server_get_type (void) G_GNUC_CONST;

TestStructServer *test_struct_server_new      (void);

G_END_DECLS

#endif /* __TEST_STRUCT_SERVER_H__ */
