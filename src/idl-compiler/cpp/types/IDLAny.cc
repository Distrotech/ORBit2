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

#include "IDLAny.hh"

#include "IDLTypedef.hh"

string
IDLAny::get_cpp_typename () const
{
	return "::CORBA::Any";
}

string
IDLAny::get_c_typename () const
{
	return "CORBA_any*";
}

bool
IDLAny::is_fixed () const
{
	return false;
}

void
IDLAny::typedef_decl_write (ostream          &ostr,
			    Indent           &indent,
			    IDLCompilerState &state,
			    const IDLTypedef &target,
			    const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

string
IDLAny::stub_decl_arg_get (const string     &cpp_id,
			   IDL_param_attr    direction,
			   const IDLTypedef *active_typedef) const
{
	std::string ret;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		ret = "const " + get_cpp_typename () + " &" + cpp_id;
		break;
	case IDL_PARAM_OUT:
		break;
	case IDL_PARAM_INOUT:
		break;
	}

	return ret;
}

void
IDLAny::stub_impl_arg_pre (ostream          &ostr,
			   Indent           &indent,
			   const string     &cpp_id,
			   IDL_param_attr    direction,
			   const IDLTypedef *active_typedef) const
{
	ostr << indent << get_c_typename () << " c_" << cpp_id
	     << " = " << cpp_id << "._orbitcpp_cobj ();"
	     << endl;
}
	
string
IDLAny::stub_impl_arg_call (const string     &cpp_id,
			    IDL_param_attr    direction,
			    const IDLTypedef *active_typedef) const
{
	return "c_" + cpp_id;
}
	
void
IDLAny::stub_impl_arg_post (ostream          &ostr,
			    Indent           &indent,
			    const string     &cpp_id,
			    IDL_param_attr    direction,
			    const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}




string
IDLAny::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
	return get_cpp_typename () + "*";
}
	
void
IDLAny::stub_impl_ret_pre (ostream &ostr,
			   Indent  &indent,
			   const IDLTypedef *active_typedef) const
{
	ostr << indent << get_c_typename () << " c_retval = 0;" << endl;	
}

void
IDLAny::stub_impl_ret_call (ostream          &ostr,
			    Indent           &indent,
			    const string     &c_call_expression,
			    const IDLTypedef *active_typedef) const
{
	ostr << indent << "c_retval = " << c_call_expression << ";"
	     << endl;
}

void
IDLAny::stub_impl_ret_post (ostream          &ostr,
			    Indent           &indent,
			    const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}
	



string
IDLAny::skel_decl_arg_get (const string     &c_id,
			   IDL_param_attr    direction,
			   const IDLTypedef *active_typedef) const
{
	std::string ret;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		ret = "const " + get_c_typename () + " " + c_id;
		break;
	case IDL_PARAM_OUT:
		break;
	case IDL_PARAM_INOUT:
		break;
	}

	return ret;
}

void
IDLAny::skel_impl_arg_pre (ostream          &ostr,
			   Indent           &indent,
			   const string     &c_id,
			   IDL_param_attr    direction,
			   const IDLTypedef *active_typedef) const
{
	ostr << indent << get_cpp_typename () << " cpp_" << c_id << " = "
	     << "::CORBA::Any::_orbitcpp_wrap (" << c_id << ");"
	     << endl;
}
	
string
IDLAny::skel_impl_arg_call (const string     &c_id,
			    IDL_param_attr    direction,
			    const IDLTypedef *active_typedef) const
{
	return "cpp_" + c_id;
}
	
void
IDLAny::skel_impl_arg_post (ostream          &ostr,
			    Indent           &indent,
			    const string     &c_id,
			    IDL_param_attr    direction,
			    const IDLTypedef *active_typedef) const
{
}




string
IDLAny::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
	return get_c_typename ();
}

void
IDLAny::skel_impl_ret_pre (ostream          &ostr,
			   Indent           &indent,
			   const IDLTypedef *active_typedef) const
{
	ostr << indent << get_cpp_typename () << " *cpp_ret = 0;"
	     << endl;
}

void
IDLAny::skel_impl_ret_call (ostream          &ostr,
			    Indent           &indent,
			    const string     &cpp_call_expression,
			    const IDLTypedef *active_typedef) const
{
	ostr << indent << "cpp_ret = " << cpp_call_expression << ";"
	     << endl;
}

void
IDLAny::skel_impl_ret_post (ostream          &ostr,
			    Indent           &indent,
			    const IDLTypedef *active_typedef) const
{
	ostr << indent << get_c_typename () << " c_ret;" << endl;
	ostr << indent << "CORBA_any__copy (c_ret, cpp_ret->_orbitcpp_cobj ());" << endl;
	ostr << indent << "delete cpp_ret;" << endl;
	ostr << indent << "return c_ret;" << endl;
}


string
IDLAny::get_cpp_member_typename (const IDLTypedef *active_typedef) const
{
	return "CORBA::Any";
}

string
IDLAny::get_c_member_typename (const IDLTypedef *active_typedef) const
{
	return "CORBA_any";
}

string
IDLAny::get_seq_typename (unsigned int      length,
			  const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

string
IDLAny::member_decl_arg_get (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	cerr << "Member_decl_arg_get" << endl;

	return "const CORBA::Any&";
}

void
IDLAny::member_impl_arg_copy (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_id,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLAny::member_init_cpp (ostream          &ostr,
			 Indent           &indent,
			 const string     &cpp_id,
			 const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLAny::member_init_c (ostream          &ostr,
		       Indent           &indent,
		       const string     &c_id,
		       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLAny::member_pack_to_c (ostream          &ostr,
			  Indent           &indent,
			  const string     &cpp_id,
			  const string     &c_id,
			  const IDLTypedef *active_typedef) const
{
#if 0
	ostr << indent << "CORBA_any__copy (" << c_id
	     << ", " << cpp_id << "._orbitcpp_cobj ());" << endl;
#endif
}

void
IDLAny::member_unpack_from_c  (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_id,
			       const string     &c_id,
			       const IDLTypedef *active_typedef) const
{
#if 0
	ostr << indent << cpp_id << ".copy ("
	     << "CORBA::Any::_orbitcpp_wrap (" << c_id << "));" << endl;
#endif
}
