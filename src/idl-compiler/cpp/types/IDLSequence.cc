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

#include "IDLSequence.hh"

#include "IDLTypedef.hh"

IDLSequence::IDLSequence (const IDLType &elementType,
			  CORBA::ULong   length) :
	m_elementType (elementType),
	m_length (length)
{
}

bool
IDLSequence::is_fixed () const
{
}

void
IDLSequence::const_decl_write (ostream          &ostr,
			  Indent           &indent,
			  const string     &cpp_id,
			  const string     &value,
			  const IDLTypedef *active_typedef = 0) const
{
}

void
IDLSequence::typedef_decl_write (ostream          &ostr,
			    Indent           &indent,
			    const IDLTypedef &target,
			    const IDLTypedef *active_typedef = 0) const
{
}

string
IDLSequence::stub_decl_arg_get (const string     &cpp_id,
			   IDL_param_attr    direction,
			   const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::stub_impl_arg_pre (ostream          &ostr,
			   Indent           &indent,
			   const string     &cpp_id,
			   IDL_param_attr    direction,
			   const IDLTypedef *active_typedef) const
{
}
	
string
IDLSequence::stub_impl_arg_call (const string     &cpp_id,
			    IDL_param_attr    direction,
			    const IDLTypedef *active_typedef) const
{
}
	
void
IDLSequence::stub_impl_arg_post (ostream          &ostr,
			    Indent           &indent,
			    const string     &cpp_id,
			    IDL_param_attr    direction,
			    const IDLTypedef *active_typedef) const
{
}




string
IDLSequence::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
}
	
void
IDLSequence::stub_impl_ret_pre (ostream &ostr,
			   Indent  &indent,
			   const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::stub_impl_ret_call (ostream          &ostr,
			    Indent           &indent,
			    const string     &c_call_expression,
			    const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::stub_impl_ret_post (ostream          &ostr,
			    Indent           &indent,
			    const IDLTypedef *active_typedef) const
{
}
	



string
IDLSequence::skel_decl_arg_get (const string     &c_id,
			   IDL_param_attr    direction,
			   const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::skel_impl_arg_pre (ostream          &ostr,
			   Indent           &indent,
			   const string     &c_id,
			   IDL_param_attr    direction,
			   const IDLTypedef *active_typedef) const
{
}
	
string
IDLSequence::skel_impl_arg_call (const string     &c_id,
			    IDL_param_attr    direction,
			    const IDLTypedef *active_typedef) const
{
}
	
void
IDLSequence::skel_impl_arg_post (ostream          &ostr,
			    Indent           &indent,
			    const string     &c_id,
			    IDL_param_attr    direction,
			    const IDLTypedef *active_typedef) const
{
}




string
IDLSequence::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::skel_impl_ret_pre (ostream          &ostr,
			   Indent           &indent,
			   const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::skel_impl_ret_call (ostream          &ostr,
			    Indent           &indent,
			    const string     &cpp_call_expression,
			    const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::skel_impl_ret_post (ostream          &ostr,
			    Indent           &indent,
			    const IDLTypedef *active_typedef) const
{
}


string
IDLSequence::get_cpp_member_typename (const IDLTypedef *active_typedef) const
{
}

string
IDLSequence::get_c_member_typename (const IDLTypedef *active_typedef) const
{
}

string
IDLSequence::member_decl_arg_get (const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::member_impl_arg_copy (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_id,
			      const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::member_init_cpp (ostream          &ostr,
			 Indent           &indent,
			 const string     &cpp_id,
			 const IDLTypedef *active_typedef = 0) const
{
}

void
IDLSequence::member_init_c (ostream          &ostr,
		       Indent           &indent,
		       const string     &c_id,
		       const IDLTypedef *active_typedef = 0) const
{
}

void
IDLSequence::member_pack_to_c (ostream          &ostr,
			  Indent           &indent,
			  const string     &cpp_id,
			  const string     &c_id,
			  const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::member_unpack_from_c  (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_id,
			       const string     &c_id,
			       const IDLTypedef *active_typedef) const
{
}
