/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 2000 Andreas Kloeckner
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
 *  Author:	Andreas Kloeckner <ak@ixion.net>
 *
 *  Purpose:	IDL compiler language representation
 *
 */

#include "IDLAttribAccessor.hh"

#include "IDLVoid.hh"
#include "types.hh"

IDLAttribGetter::IDLAttribGetter (IDLAttribute &attr_):
	IDLMethod (*(IDLInterface*)attr_.getParentScope ()),
	attr (attr_)
{
	m_returntype = attr.getType ();
}

string
IDLAttribGetter::skel_decl_impl () const
{
	string skel_name = parent_interface.get_cpp_poa_method_prefix ();

	return skel_ret_get () + " " + skel_name + "::" +
		"_skel__get_" + attr.get_cpp_identifier () +
		" (" + skel_arglist_get () + ")";
}

string
IDLAttribGetter::get_cpp_methodname () const
{
	return attr.get_cpp_identifier ();
}

string
IDLAttribGetter::get_c_methodname () const
{
	return parent_interface.get_c_typename () +
		"__get_" + attr.get_c_identifier ();
}

namespace
{
	static IDLVoid void_type;
}

IDLAttribSetter::IDLAttribSetter (IDLAttribute &attr_):
	IDLMethod (*(IDLInterface*)attr_.getParentScope ()),
	attr (attr_)
{
	m_returntype = &void_type;

	IDLMethod::ParameterInfo param_info;
	param_info.id = "val";
	param_info.type = attr.getType ();
	param_info.direction = IDL_PARAM_IN;

	m_parameterinfo.push_back (param_info);
}

string
IDLAttribSetter::skel_decl_impl () const
{
	string skel_name = parent_interface.get_cpp_poa_method_prefix ();

	return skel_ret_get () + " " + skel_name + "::" +
		"_skel__set_" + attr.get_cpp_identifier () +
		" (" + skel_arglist_get () + ")";
}

string
IDLAttribSetter::get_cpp_methodname () const
{
	return attr.get_cpp_identifier ();
}

string
IDLAttribSetter::get_c_methodname () const
{
	return parent_interface.get_c_typename () +
		"__set_" + attr.get_c_identifier ();
}
