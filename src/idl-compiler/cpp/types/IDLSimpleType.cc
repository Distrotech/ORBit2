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
 *  Purpose:	IDL compiler type representation
 *
 *
 */

#include "IDLSimpleType.hh"

void
IDLSimpleType::const_decl_write (ostream          &ostr,
				 Indent           &indent,
				 const string     &cpp_id,
				 const string     &value,
				 const IDLTypedef *active_typedef) const
{
	ostr << indent << "const " << get_fixed_cpp_typename ()
	     << cpp_id << " = " << value
	     << ';' << endl;
}

string
IDLSimpleType::stub_decl_arg_get (const string     &cpp_id,
				  IDL_param_attr    direction,
				  const IDLTypedef *active_typedef) const
{
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = get_fixed_cpp_typename () + " " + cpp_id;
		break;
	case IDL_PARAM_INOUT:
		retval = get_fixed_cpp_typename () + " &" + cpp_id;
		break;
	case IDL_PARAM_OUT:
		retval = get_fixed_cpp_typename () + "_out " + cpp_id;
		break;
	}

	return retval;
}

void
IDLSimpleType::stub_impl_arg_pre (ostream        &ostr,
				  Indent         &indent,
				  const string   &cpp_id,
				  IDL_param_attr  direction) const
{
	// Do nothing
}
	
string
IDLSimpleType::stub_impl_arg_call (const string   &cpp_id,
				   IDL_param_attr  direction) const
{
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = cpp_id;
		break;
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		retval = "&" + cpp_id;
		break;
	}
	
	return retval;
}
	
void
IDLSimpleType::stub_impl_arg_post (ostream        &ostr,
				   Indent         &indent,
				   const string   &cpp_id,
				   IDL_param_attr  direction) const
{
	// Do nothing
}




string
IDLSimpleType::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
	return get_fixed_cpp_typename ();
}

void
IDLSimpleType::stub_impl_ret_pre (ostream &ostr,
				  Indent  &indent) const
{
	// Do nothing
}

void
IDLSimpleType::stub_impl_ret_call (ostream      &ostr,
				   Indent       &indent,
				   const string &c_call_expression) const
{
	ostr << indent << get_fixed_cpp_typename () << " _retval = "
	     << c_call_expression << ";" << endl;
}

void
IDLSimpleType::stub_impl_ret_post (ostream &ostr,
				   Indent  &indent) const
{
	// Do nothing
}
	



string
IDLSimpleType::skel_decl_arg_get (const string     &c_id,
				  IDL_param_attr    direction,
				  const IDLTypedef *active_typedef) const
{
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "const " + get_fixed_c_typename () + " " + c_id;
		break;
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		retval = get_fixed_c_typename () + " *" + c_id;
		break;
	}

	return retval;
}

void
IDLSimpleType::skel_impl_arg_pre (ostream        &ostr,
				  Indent         &indent,
				  const string   &c_id,
				  IDL_param_attr  direction) const
{
	// Do nothing
}
	
string
IDLSimpleType::skel_impl_arg_call (const string   &c_id,
				   IDL_param_attr  direction) const
{
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval =  c_id;
		break;
	case IDL_PARAM_INOUT:
		retval = "*" + c_id;
		break;
	case IDL_PARAM_OUT:
		retval = get_fixed_cpp_typename () + "_out (" + c_id + ")";
		break;
	}
	
	return retval;
}
	
void
IDLSimpleType::skel_impl_arg_post (ostream        &ostr,
				   Indent         &indent,
				   const string   &c_id,
				   IDL_param_attr  direction) const
{
	// Do nothing
}




string
IDLSimpleType::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
	return get_fixed_c_typename ();
}

void
IDLSimpleType::skel_impl_ret_pre (ostream &ostr,
				  Indent  &indent) const
{
	ostr << indent << get_fixed_c_typename () << " _retval"
	     << ';' << endl;
}

void
IDLSimpleType::skel_impl_ret_call (ostream      &ostr,
				   Indent       &indent,
				   const string &cpp_call_expression) const
{
	ostr << indent << " _retval = " << cpp_call_expression << ";" << endl;
}

void
IDLSimpleType::skel_impl_ret_post (ostream &ostr,
				   Indent  &indent) const
{
	// WRITE ME
}

string
IDLSimpleType::get_cpp_member_typename () const
{
	return get_fixed_cpp_typename ();
}

string
IDLSimpleType::member_decl_arg_get () const
{
	return get_fixed_cpp_typename ();
}

void
IDLSimpleType::member_impl_arg_copy (ostream      &ostr,
				     Indent       &indent,
				     const string &cpp_id) const
{
	ostr << indent << cpp_id << " = _par_" << cpp_id
	     << ';' << endl;
}

void
IDLSimpleType::member_pack_to_c_pre  (ostream      &ostr,
				      Indent       &indent,
				      const string &member_id,
				      const string &c_struct_id) const
{
	// Do nothing
}


void
IDLSimpleType::member_pack_to_c_pack (ostream      &ostr,
				      Indent       &indent,
				      const string &member_id,
				      const string &c_struct_id) const
{
	ostr << indent << c_struct_id << '.' << member_id << " = "
	     << member_id << ';' << endl;
}

void
IDLSimpleType::member_pack_to_c_post (ostream      &ostr,
				      Indent       &indent,
				      const string &member_id,
				      const string &c_struct_id) const
{
	// Do nothing
}

void
IDLSimpleType::member_unpack_from_c_pre  (ostream      &ostr,
					  Indent       &indent,
					  const string &member_id,
					  const string &c_struct_id) const
{
	// Do nothing
}

void
IDLSimpleType::member_unpack_from_c_pack (ostream      &ostr,
					  Indent       &indent,
					  const string &member_id,
					  const string &c_struct_id) const
{
	ostr << indent << member_id << " = "
	     << c_struct_id << '.' << member_id << ';' << endl;
}

void
IDLSimpleType::member_unpack_from_c_post  (ostream      &ostr,
					   Indent       &indent,
					   const string &member_id,
					   const string &c_struct_id) const
{
	// Do nothing
}
