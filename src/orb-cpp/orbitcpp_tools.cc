/* -*- Mode: C++; indent-tabs-mode: t -*- */

/*
 *	ORBit-C++: C++ bindings for ORBit.
 *
 *	Copyright (C) 2000 Andreas Kloeckner
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Library General Public
 *	License as published by the Free Software Foundation; either
 *	version 2 of the License, or (at your option) any later version.
 *
 *	This library is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	Library General Public License for more details.
 *
 *	You should have received a copy of the GNU Library General Public
 *	License along with this library; if not, write to the Free
 *	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *	Authors: Andreas Kloeckner <ak@ixion.net>
 *           Phil Dawes <philipd@users.sourceforge.net>
 *			 John K. Luebs <jkluebs@marikarpress.com>
 *
 */




#include <cstdio>
#include <cstdlib>
#include "orbitcpp_tools.hh"
#include "orbitcpp_exception.hh"
#include <orbit/orb-core/corba-orb.h>




// tool functions -------------------------------------------------------------


CORBA_Object _orbitcpp::duplicate_guarded(CORBA_Object obj) {
	CEnvironment ev;
	CORBA_Object result = CORBA_Object_duplicate(obj, ev._orbitcpp_cobj());
	ev.propagate_sysex();
	return result;
}


void _orbitcpp::release_guarded(CORBA_Object obj) {
	CEnvironment ev;
	CORBA_Object_release(obj, ev._orbitcpp_cobj());
	ev.propagate_sysex();
}




void _orbitcpp::error(char *text) {
	printf("%s\n",text);
	abort();
}
