/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 2000 John K. Luebs
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
 *  Author: John K. Luebs <jkluebs@marikarpress.com>
 *
 *  Description: CORBA Any implementation
 */
#include <cwchar>
#include <orbit/orbit.h>
#include "orbitcpp.hh"
#include "orbitcpp_any.hh"
#include <string.h>

void
CORBA::Any::copy (const Any &in)
{
	free ();
	CORBA_any__copy (_orbitcpp_cobj(), in._orbitcpp_cobj());
}

void
CORBA::Any::free ()
{
	if (m_target._release)
		CORBA_free (m_target._value);
	
	::_orbitcpp::CEnvironment _ev;
	CORBA_Object_release ((CORBA_Object)m_target._type,
						  _ev._orbitcpp_cobj()); // release typecode
}


CORBA::Any
CORBA::Any::_orbitcpp_wrap (const CORBA_any *c_any)
{
	CORBA::Any cpp_any;

	CORBA_any__copy (cpp_any._orbitcpp_cobj (), c_any);

	return cpp_any;
}

CORBA::Any::Any ()
{
	m_target._type = _tc_null->_orbitcpp_cobj ();
	m_target._value = 0;
	CORBA_any_set_release (_orbitcpp_cobj (), CORBA_FALSE);
}

CORBA::Any::Any (const Any& in)
{ 
	CORBA_any__copy (_orbitcpp_cobj (), in._orbitcpp_cobj());
}
		
CORBA::Any::~Any()
{
	free ();
	
    // dont want the ORBit freekids to 'free' these again
	CORBA_any_set_release ((CORBA_any *)&m_target, CORBA_FALSE);
	m_target._value = 0;
	m_target._type = 0;
}

CORBA::Any& CORBA::Any::operator= (const CORBA::Any &in)
{
	if (&in != this)
		copy (in);
	return *this;
}

void
CORBA::Any::insert_simple(CORBA::TypeCode_ptr tc, void* value, Boolean v_copy)
{
	void *new_val;
	if(v_copy)
		new_val = ORBit_copy_value(value, reinterpret_cast<CORBA_TypeCode>(tc));
	else
		new_val = value;
	if( CORBA_any_get_release((CORBA_any*)&m_target))
		CORBA_free( m_target._value );
	m_target._value = new_val;
	CORBA_any_set_release((CORBA_any*)&m_target,CORBA_TRUE);

	if (TypeCode::_orbitcpp_wrap (m_target._type) != tc)
	{
		::_orbitcpp::CEnvironment _ev;
		CORBA_Object_release((CORBA_Object)m_target._type, _ev._orbitcpp_cobj()); // release typecode
		m_target._type = tc->_orbitcpp_cobj ();
	}
}	

void
CORBA::Any::operator<<=(from_string in)
{
	if( in.bound && (strlen(in.val) > in.bound) )
		return;
	::_orbitcpp::CEnvironment _ev;
	CORBA_Object_release((CORBA_Object)m_target._type, _ev._orbitcpp_cobj()); // release typecode

	CORBA_TypeCode new_tc = ::_orbitcpp::TypeCode_allocate();
	new_tc->kind = CORBA_tk_string;
	new_tc->length = in.bound;
	m_target._type = new_tc;

	if (CORBA_any_get_release((CORBA_any*)&m_target))
		CORBA_free (m_target._value);

	if (in.nocopy) {
		//		m_target._value = ORBit_alloc_tcval(m_target._type, 1);
		*(CORBA_char**)m_target._value = in.val;
	}
	else
		m_target._value = ORBit_copy_value(&in.val, m_target._type);
	CORBA_any_set_release((CORBA_any*)&m_target,CORBA_TRUE);
}

void
CORBA::Any::operator<<=(from_wstring in)
{
	if( in.bound && (wcslen((wchar_t*)in.val) > in.bound) )
		return;
	::_orbitcpp::CEnvironment _ev;
	CORBA_Object_release((CORBA_Object)m_target._type, _ev._orbitcpp_cobj()); // release typecode
	CORBA_TypeCode new_tc = ::_orbitcpp::TypeCode_allocate();
	new_tc->kind = CORBA_tk_wstring;
	new_tc->length = in.bound;
	m_target._type = new_tc;
		
	if( CORBA_any_get_release((CORBA_any*)&m_target) )
		CORBA_free( m_target._value );
	
	if( in.nocopy ) {
		m_target._value = ORBit_alloc_tcval(reinterpret_cast<CORBA_TypeCode>(m_target._type), 1);
		*(CORBA_char**)m_target._value = (CORBA_char*)in.val;
	}
	else
		m_target._value = ORBit_copy_value(&in.val, reinterpret_cast<CORBA_TypeCode>(m_target._type));
	CORBA_any_set_release((CORBA_any*)&m_target,CORBA_TRUE);
}

CORBA::Boolean
CORBA::Any::operator>>=(to_string out) const
{
	CORBA_TypeCode tmp = ::_orbitcpp::TypeCode_allocate();
	tmp->kind = CORBA_tk_string;
	tmp->length = out.bound;
	Boolean ret = extract (CORBA::TypeCode::_orbitcpp_wrap (tmp), const_cast<char*&> (out.val));

	::_orbitcpp::CEnvironment _ev;
	CORBA_Object_release((CORBA_Object)tmp, _ev._orbitcpp_cobj()); // release typecode

	return ret;
}

CORBA::Boolean
CORBA::Any::operator>>=(to_wstring out) const
{
	CORBA_TypeCode tmp = ::_orbitcpp::TypeCode_allocate();
	tmp->kind = CORBA_tk_wstring;
	tmp->length = out.bound;
	Boolean ret = extract(reinterpret_cast<CORBA::TypeCode_ptr>(tmp), const_cast<WChar*&>(out.val));
	::_orbitcpp::CEnvironment _ev;
	CORBA_Object_release((CORBA_Object)tmp, _ev._orbitcpp_cobj()); // release typecode
	return ret;
}

CORBA::Boolean
CORBA::Any::operator>>=(to_object out) const
{
	g_warning("I'm not sure if any extraction to objects works or not with ORBit stable -PD");
	// 1.16.5 widens any object reference and dupes

	if (m_target._type != _tc_Object->_orbitcpp_cobj ())
		return CORBA_FALSE;
	
	out.ref = Object::_orbitcpp_wrap (static_cast<CORBA_Object>(m_target._value));
	return CORBA_TRUE;
}
	
