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

#include "IDLVoid.hh"

string
IDLVoid::stub_decl_arg_get (const string     &cpp_id,
			    IDL_param_attr    direction,
			    const IDLTypedef *active_typedef) const
{
	throw IDLExVoid ();
}

string
IDLVoid::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
	return "void";
}
	
void
IDLVoid::stub_impl_arg_pre (ostream        &ostr,
			    Indent         &indent,
			    const string   &cpp_id,
			    IDL_param_attr  direction) const
{
	throw IDLExVoid ();
}
	
string
IDLVoid::stub_impl_arg_call (const string   &cpp_id,
			     IDL_param_attr  direction) const
{
	throw IDLExVoid ();
}
	
void
IDLVoid::stub_impl_arg_post (ostream        &ostr,
			     Indent         &indent,
			     const string   &cpp_id,
			     IDL_param_attr  direction) const
{
	throw IDLExVoid ();
}

void
IDLVoid::stub_impl_ret_pre (ostream &ostr,
			    Indent  &indent) const
{
	// Do nothing
}

void
IDLVoid::stub_impl_ret_call (ostream      &ostr,
			     Indent       &indent,
			     const string &c_call_expression) const
{
	ostr << indent << c_call_expression << ";" << endl;
}

void
IDLVoid::stub_impl_ret_post (ostream &ostr,
			     Indent  &indent) const
{
	// Do nothing
}
	
string
IDLVoid::skel_decl_arg_get (const string     &c_id,
			    IDL_param_attr    direction,
			    const IDLTypedef *active_typedef) const
{
	throw IDLExVoid ();
}

string
IDLVoid::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
	return "void";
}

void
IDLVoid::skel_impl_arg_pre (ostream        &ostr,
			    Indent         &indent,
			    const string   &c_id,
			    IDL_param_attr  direction) const
{
	throw IDLExVoid ();
}
	
string
IDLVoid::skel_impl_arg_call (const string   &c_id,
			     IDL_param_attr  direction) const
{
	throw IDLExVoid ();
}
	
void
IDLVoid::skel_impl_arg_post (ostream        &ostr,
			     Indent         &indent,
			     const string   &c_id,
			     IDL_param_attr  direction) const
{
	throw IDLExVoid ();
}

void
IDLVoid::skel_impl_ret_pre (ostream &ostr,
			    Indent  &indent) const
{
	// Do nothing
}

void
IDLVoid::skel_impl_ret_call (ostream      &ostr,
			     Indent       &indent,
			     const string &cpp_call_expression) const
{
	ostr << indent << cpp_call_expression << ";" << endl;
}

void
IDLVoid::skel_impl_ret_post (ostream &ostr,
			     Indent  &indent) const
{
	// Do nothing
}


string
IDLVoid::get_cpp_member_typename () const
{
	throw IDLExVoid ();
}

string
IDLVoid::member_decl_arg_get () const
{
	throw IDLExVoid ();
}

void
IDLVoid::member_impl_arg_copy (ostream      &ostr,
			       Indent       &indent,
			       const string &cpp_id) const
{
	throw IDLExVoid ();
}

void
IDLVoid::member_pack_to_c_pre  (ostream      &ostr,
				Indent       &indent,
				const string &cpp_id,
				const string &c_id) const
{
	throw IDLExVoid ();
}

void
IDLVoid::member_pack_to_c_pack (ostream      &ostr,
				Indent       &indent,
				const string &cpp_id,
				const string &c_id) const
{
	throw IDLExVoid ();
}

void
IDLVoid::member_pack_to_c_post (ostream      &ostr,
				Indent       &indent,
				const string &cpp_id,
				const string &c_id) const
{
	throw IDLExVoid ();
}

void
IDLVoid::member_unpack_from_c_pre  (ostream      &ostr,
				    Indent       &indent,
				    const string &cpp_id,
				    const string &c_id) const
{
	throw IDLExVoid ();
}

void
IDLVoid::member_unpack_from_c_pack (ostream      &ostr,
				    Indent       &indent,
				    const string &cpp_id,
				    const string &c_id) const
{
	throw IDLExVoid ();
}

void
IDLVoid::member_unpack_from_c_post  (ostream      &ostr,
				     Indent       &indent,
				     const string &cpp_id,
				     const string &c_id) const
{
	throw IDLExVoid ();
}
