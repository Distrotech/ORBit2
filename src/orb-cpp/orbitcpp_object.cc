/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 1998 Phil Dawes
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Phil Dawes <philipd@users.sourceforge.co.uk>
 *			Andreas Kloeckner <ak@ixion.net>
 *
 */




#include "orbitcpp_object.hh"
#include "orbitcpp_exception.hh"




using namespace _orbitcpp;




void 
CORBA::Object::operator delete(void* c_objref) {
	CEnvironment ev;
	CORBA_Object_release(reinterpret_cast<CORBA_Object>(c_objref),ev);
	ev.propagate_sysex();
}




CORBA::Boolean 
CORBA::Object::_is_a(CORBA::RepositoryId const repoid) {
	_orbitcpp::CEnvironment ev;
	Boolean result = CORBA_Object_is_a(*this,repoid,ev);
	ev.propagate_sysex();
	return result;
}




CORBA::Boolean 
CORBA::Object::_non_existent() {
	_orbitcpp::CEnvironment ev;
	Boolean result = CORBA_Object_non_existent(*this,ev);
	ev.propagate_sysex();
	return result;
}




CORBA::Boolean 
CORBA::Object::_is_equivalent(::CORBA::Object_ptr other_object) {
	_orbitcpp::CEnvironment ev;
	Boolean result = CORBA_Object_is_equivalent(*this,*other_object,ev);
	ev.propagate_sysex();
	return result;
}




CORBA::ULong 
CORBA::Object::_hash(::CORBA::ULong maximum) {
	_orbitcpp::CEnvironment ev;
	ULong result = CORBA_Object_hash(*this,maximum,ev);
	ev.propagate_sysex();
	return result;
}




CORBA::Boolean
CORBA::is_nil(Object_ptr o) {
	_orbitcpp::CEnvironment ev;
	Boolean result = CORBA_Object_is_nil(*o,ev);
	ev.propagate_sysex();
	return result;
}
