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
	if( m_target._type != tc )
	{
		::_orbitcpp::CEnvironment _ev;
		CORBA_Object_release((CORBA_Object)m_target._type, _ev._orbitcpp_get_c_object()); // release typecode
		m_target._type = tc;
	}
}	

void
CORBA::Any::operator<<=(from_string in)
{
	if( in.bound && (strlen(in.val) > in.bound) )
		return;
	::_orbitcpp::CEnvironment _ev;
	CORBA_Object_release((CORBA_Object)m_target._type, _ev._orbitcpp_get_c_object()); // release typecode

	CORBA_TypeCode new_tc = ::_orbitcpp::TypeCode_allocate();
	new_tc->kind = CORBA_tk_string;
	new_tc->length = in.bound;
	m_target._type = reinterpret_cast<CORBA::TypeCode_ptr>(new_tc);

	if(CORBA_any_get_release((CORBA_any*)&m_target))
		CORBA_free( m_target._value );

	if( in.nocopy ) {
		//		m_target._value = ORBit_alloc_tcval(m_target._type, 1);
		*(CORBA_char**)m_target._value = in.val;
	}
	else
		m_target._value = ORBit_copy_value(&in.val, reinterpret_cast<CORBA_TypeCode>(m_target._type));
	CORBA_any_set_release((CORBA_any*)&m_target,CORBA_TRUE);
}

void
CORBA::Any::operator<<=(from_wstring in)
{
	if( in.bound && (wcslen((wchar_t*)in.val) > in.bound) )
		return;
	::_orbitcpp::CEnvironment _ev;
	CORBA_Object_release((CORBA_Object)m_target._type, _ev._orbitcpp_get_c_object()); // release typecode
	CORBA_TypeCode new_tc = ::_orbitcpp::TypeCode_allocate();
	new_tc->kind = CORBA_tk_wstring;
	new_tc->length = in.bound;
	m_target._type = reinterpret_cast<CORBA::TypeCode_ptr>(new_tc);
		
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
	Boolean ret = extract(reinterpret_cast<CORBA::TypeCode_ptr>(tmp), const_cast<char*&>(out.val));
	::_orbitcpp::CEnvironment _ev;
	CORBA_Object_release((CORBA_Object)tmp, _ev._orbitcpp_get_c_object()); // release typecode
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
	CORBA_Object_release((CORBA_Object)tmp, _ev._orbitcpp_get_c_object()); // release typecode
	return ret;
}
