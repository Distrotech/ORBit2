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

#include "IDLUserDefSimpleType.hh"

string
IDLUserDefSimpleType::get_fixed_cpp_typename () const
{
	return get_cpp_typename ();
}

string
IDLUserDefSimpleType::get_fixed_c_typename () const
{
	return get_c_typename ();
}


string
IDLUserDefSimpleType::stub_decl_arg_get (const string   &cpp_id,
					 IDL_param_attr  direction,
					 IDLTypedef     *active_typedef) const
{
    	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = get_cpp_typename () + " " + cpp_id;
		break;
	case IDL_PARAM_INOUT:
		retval = get_cpp_typename () + " &" + cpp_id;
		break;
	case IDL_PARAM_OUT:
		retval = get_cpp_typename () + "_out " + cpp_id;
		break;
	}

	return retval;
}

void
IDLUserDefSimpleType::stub_impl_arg_pre (ostream        &ostr,
					 Indent         &indent,
					 const string   &cpp_id,
					 IDL_param_attr  direction) const
{
	// Do nothing
}
	
string
IDLUserDefSimpleType::stub_impl_arg_call (const string   &cpp_id,
					  IDL_param_attr  direction) const
{
	string retval;
	string expr = "(" + get_c_typename () + ")" + cpp_id;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = expr;
		break;
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		retval = "&" + expr;
		break;
	}
	
	return retval;
}
	
void
IDLUserDefSimpleType::stub_impl_arg_post (ostream        &ostr,
					  Indent         &indent,
					  const string   &cpp_id,
					  IDL_param_attr  direction) const
{
	// Do nothing
}




string
IDLUserDefSimpleType::stub_decl_ret_get (IDLTypedef *active_typedef) const
{
	return get_cpp_typename ();
}

void
IDLUserDefSimpleType::stub_impl_ret_pre (ostream &ostr,
					 Indent  &indent) const
{
	// Do nothing
}

void
IDLUserDefSimpleType::stub_impl_ret_call (ostream      &ostr,
					  Indent       &indent,
					  const string &c_call_expression) const
{
	ostr << indent << get_cpp_typename () << " _retval = "
	     << "(" << get_cpp_typename () << ")" << c_call_expression
	     << ';' << endl;
}

void
IDLUserDefSimpleType::stub_impl_ret_post (ostream &ostr,
					  Indent  &indent) const
{
	ostr << indent << "return _retval;" << endl;
}
	



string
IDLUserDefSimpleType::skel_decl_arg_get (const string   &c_id,
					 IDL_param_attr  direction,
					 IDLTypedef     *active_typedef) const
{
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "const " + get_c_typename () + " " + c_id;
		break;
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		retval = get_c_typename () + " *" + c_id;
		break;
	}

	return retval;
}

void
IDLUserDefSimpleType::skel_impl_arg_pre (ostream        &ostr,
					 Indent         &indent,
					 const string   &c_id,
					 IDL_param_attr  direction) const
{
	// Do nothing
}
	
string
IDLUserDefSimpleType::skel_impl_arg_call (const string   &c_id,
					  IDL_param_attr  direction) const
{
	string retval;
	string expr = "(" + get_cpp_typename () + ")" + c_id;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval =  expr;
		break;
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		retval = "*" + expr;
		break;
	}
	
	return retval;	
}
	
void
IDLUserDefSimpleType::skel_impl_arg_post (ostream        &ostr,
					  Indent         &indent,
					  const string   &c_id,
					  IDL_param_attr  direction) const
{
	// Do nothing
}




string
IDLUserDefSimpleType::skel_decl_ret_get (IDLTypedef *active_typedef) const
{
	return get_c_typename ();
}

void
IDLUserDefSimpleType::skel_impl_ret_pre (ostream &ostr,
					 Indent  &indent) const
{
	ostr << indent << get_c_typename () << " _retval"
	     << ';' << endl;
}

void
IDLUserDefSimpleType::skel_impl_ret_call (ostream      &ostr,
					  Indent       &indent,
					  const string &cpp_call_expression) const
{
	ostr << indent << " _retval = "
	     << "(" << get_c_typename () << ")" << cpp_call_expression
	     << ';' << endl;
}

void
IDLUserDefSimpleType::skel_impl_ret_post (ostream &ostr,
					  Indent  &indent) const
{
	ostr << indent << "return _retval;" << endl;
}


