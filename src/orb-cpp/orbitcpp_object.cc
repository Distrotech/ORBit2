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



CORBA_Object CORBA::Object::_orbitcpp_get_c_object() {
	return m_target;
}

CORBA::Object::Object()
{
}

CORBA::Object::Object(CORBA_Object cobject)
{
  ::_orbitcpp::CEnvironment ev;
	m_target = CORBA_Object_duplicate(cobject, ev._orbitcpp_get_c_object());
}

CORBA::Object::~Object()
{
}

CORBA::Object::Object* CORBA::Object::_orbitcpp_wrap(CORBA_Object cobject, bool release_c_object /* = true */)
{
	CORBA::Object::Object* cppObject = new CORBA::Object::Object(cobject);

	if(release_c_object)
	{
		::_orbitcpp::CEnvironment ev;
		CORBA_Object_release(cobject, ev._orbitcpp_get_c_object());
		ev.propagate_sysex();
	}

	return cppObject;
}

CORBA::Object_ptr CORBA::Object::_duplicate(Object_ptr o) {
	return new CORBA::Object( _orbitcpp::duplicate_guarded(o->_orbitcpp_get_c_object()) );
}

CORBA::Object_ptr CORBA::Object::_narrow(Object_ptr o) {
	return _duplicate(o);
}

CORBA::Object_ptr CORBA::Object::_nil() {
	return CORBA_OBJECT_NIL;
}


void 
CORBA::Object::operator delete(void* cpp_objref) {
	Object* pObject = static_cast<Object*>(cpp_objref);
	CEnvironment ev;
	CORBA_Object_release(pObject->_orbitcpp_get_c_object(), ev._orbitcpp_get_c_object());
	ev.propagate_sysex();
}




CORBA::Boolean 
CORBA::Object::_is_a(CORBA::RepositoryId const repoid) {
	_orbitcpp::CEnvironment ev;
	Boolean result = CORBA_Object_is_a(_orbitcpp_get_c_object(), repoid, ev._orbitcpp_get_c_object());
	ev.propagate_sysex();
	return result;
}




CORBA::Boolean 
CORBA::Object::_non_existent() {
	_orbitcpp::CEnvironment ev;
	Boolean result = CORBA_Object_non_existent(_orbitcpp_get_c_object(), ev._orbitcpp_get_c_object());
	ev.propagate_sysex();
	return result;
}




CORBA::Boolean 
CORBA::Object::_is_equivalent(::CORBA::Object_ptr other_object) {
	_orbitcpp::CEnvironment ev;
	Boolean result = CORBA_Object_is_equivalent(_orbitcpp_get_c_object(), other_object->_orbitcpp_get_c_object(), ev._orbitcpp_get_c_object());
	ev.propagate_sysex();
	return result;
}




CORBA::ULong 
CORBA::Object::_hash(::CORBA::ULong maximum) {
	_orbitcpp::CEnvironment ev;
	ULong result = CORBA_Object_hash(_orbitcpp_get_c_object(), maximum, ev._orbitcpp_get_c_object());
	ev.propagate_sysex();
	return result;
}




CORBA::Boolean
CORBA::is_nil(Object_ptr o) {
	_orbitcpp::CEnvironment ev;
	Boolean result = CORBA_Object_is_nil(o->_orbitcpp_get_c_object(), ev._orbitcpp_get_c_object());
	ev.propagate_sysex();
	return result;
}
