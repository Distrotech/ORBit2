/*
 * test-basic-server.h: implementation of test::BasicServer interface.
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

#ifndef __TEST_BASIC_SERVER_H__
#define __TEST_BASIC_SERVER_H__

#include <glib/gmacros.h>
#include <orbit/orbit.h>

#include "everything.h"

G_BEGIN_DECLS

#define TEST_TYPE_BASIC_SERVER         (test_basic_server_get_type ())
#define TEST_BASIC_SERVER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TEST_TYPE_BASIC_SERVER, TestBasicServer))
#define TEST_BASIC_SERVER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TEST_TYPE_BASIC_SERVER, TestBasicServerClass))
#define TEST_IS_BASIC_SERVER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TEST_TYPE_BASIC_SERVER))
#define TEST_IS_BASIC_SERVER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TEST_TYPE_BASIC_SERVER))
#define TEST_BASIC_SERVER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TEST_TYPE_BASIC_SERVER, TestBasicServerClass))

typedef struct _TestBasicServer        TestBasicServer;
typedef struct _TestBasicServerClass   TestBasicServerClass;
typedef struct _TestBasicServerPrivate TestBasicServerPrivate;

struct _TestBasicServer {
	ORBitGServant            parent;

	TestBasicServerPrivate  *priv;
};

struct _TestBasicServerClass {
	ORBitGServantClass  parent_class;

	CORBA_string      (*get_foo)               (TestBasicServer         *bserver,
						    CORBA_Environment       *ev);

	void              (*set_foo)               (TestBasicServer         *bserver,
						    const CORBA_char        *value,
						    CORBA_Environment       *ev);

	CORBA_long        (*get_bah)               (TestBasicServer         *bserver,
						    CORBA_Environment       *ev);

	CORBA_string      (*op_string)             (TestBasicServer         *bserver,
						    const CORBA_char        *inArg,
						    CORBA_string            *inoutArg,
						    CORBA_string            *outArg,
						    CORBA_Environment       *ev);

	CORBA_long        (*op_long)               (TestBasicServer         *bserver,
						    const CORBA_long         inArg,
						    CORBA_long              *inoutArg,
						    CORBA_long              *outArg,
						    CORBA_Environment       *ev);

	CORBA_long_long   (*op_long_long)          (TestBasicServer         *bserver,
						    const CORBA_long_long    inArg,
						    CORBA_long_long         *inoutArg,
						    CORBA_long_long         *outArg,
						    CORBA_Environment       *ev);

	CORBA_float       (*op_float)              (TestBasicServer         *bserver,
						    const CORBA_float        inArg,
						    CORBA_float             *inoutArg,
						    CORBA_float             *outArg,
						    CORBA_Environment       *ev);

	CORBA_double      (*op_double)             (TestBasicServer         *bserver,
						    const CORBA_double       inArg,
						    CORBA_double            *inoutArg,
						    CORBA_double            *outArg,
						    CORBA_Environment       *ev);

	CORBA_long_double (*op_long_double)        (TestBasicServer         *bserver,
						    const CORBA_long_double  inArg,
						    CORBA_long_double       *inoutArg,
						    CORBA_long_double       *outArg,
						    CORBA_Environment       *ev);

	test_AnEnum       (*op_enum)               (TestBasicServer         *bserver,
						    const test_AnEnum        inArg,
						    test_AnEnum             *inoutArg,
						    test_AnEnum             *outArg,
						    CORBA_Environment       *ev);

	void              (*op_exception)          (TestBasicServer         *bserver,
						    CORBA_Environment       *ev);

	void              (*op_one_way)            (TestBasicServer         *bserver,
						    const CORBA_char        *inArg,
						    CORBA_Environment       *ev);

	void              (*no_implement)          (TestBasicServer         *bserver,
						    CORBA_Environment       *ev);

	void              (*test_large_string_seq) (TestBasicServer         *bserver,
						    const test_StrSeq       *seq,
						    CORBA_Environment       *ev);
};

GType            test_basic_server_get_type (void) G_GNUC_CONST;

TestBasicServer *test_basic_server_new      (void);

G_END_DECLS

#endif /* __TEST_BASIC_SERVER_H__ */
