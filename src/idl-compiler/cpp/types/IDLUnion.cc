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

#include "IDLUnion.hh"

#include "IDLTypedef.hh"

IDLUnion::IDLUnion(const string                &id,
		   IDL_tree                     node,
		   const IDLUnionDiscriminator &discriminator,
		   IDLScope                    *parentscope) :
    IDLScope (id, node, parentscope),
    m_discriminator (discriminator)
{
}

const IDLUnionDiscriminator &
IDLUnion::get_discriminator () const
{
	return m_discriminator;
}

bool
IDLUnion::is_fixed () const
{
#warning "WRITE ME"
	return false;
}

void
IDLUnion::typedef_decl_write (ostream          &ostr,
			      Indent           &indent,
			      IDLCompilerState &state,
			      const IDLTypedef &target,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

string
IDLUnion::stub_decl_arg_get (const string     &cpp_id,
			     IDL_param_attr    direction,
			     const IDLTypedef *active_typedef) const
{
	string cpp_typename = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "const " + cpp_typename + " &" + cpp_id;
		break;
	case IDL_PARAM_INOUT:
		retval = cpp_typename + " &" + cpp_id;
		break;
	case IDL_PARAM_OUT:
		retval = cpp_typename + "_out " + cpp_id;
		break;
	}

	return retval;
}

void
IDLUnion::stub_impl_arg_pre (ostream          &ostr,
			     Indent           &indent,
			     const string     &cpp_id,
			     IDL_param_attr    direction,
			     const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	
	if (direction != IDL_PARAM_IN)
		cerr << "IDLUnion::stub_impl_arg_pre: only IN parameters are handled" << endl;

	const string c_type = active_typedef ?
		active_typedef->get_c_typename () : get_c_typename ();
	
	ostr << indent << c_type << " *_c_" << cpp_id
	     << " = " << cpp_id << "._orbitcpp_pack ();" << endl;
}
	
string
IDLUnion::stub_impl_arg_call (const string     &cpp_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	
	if (direction != IDL_PARAM_IN)
		cerr << "IDLUnion::stub_impl_arg_call: only IN parameters are handled" << endl;

	return "_c_" + cpp_id;
}
	
void
IDLUnion::stub_impl_arg_post (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	
	if (direction != IDL_PARAM_IN)
		cerr << "IDLUnion::stub_impl_arg_post: only IN parameters are handled" << endl;

	ostr << indent << "CORBA_free (_c_" << cpp_id << ");" << endl;
}




string
IDLUnion::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}
	
void
IDLUnion::stub_impl_ret_pre (ostream &ostr,
			   Indent  &indent,
			   const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLUnion::stub_impl_ret_call (ostream          &ostr,
			      Indent           &indent,
			      const string     &c_call_expression,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLUnion::stub_impl_ret_post (ostream          &ostr,
			      Indent           &indent,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}
	



string
IDLUnion::skel_decl_arg_get (const string     &c_id,
			     IDL_param_attr    direction,
			     const IDLTypedef *active_typedef) const
{
	string c_typename = active_typedef ?
		active_typedef->get_c_typename () : get_c_typename ();
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "const " + c_typename + " *" + c_id;
		break;
	case IDL_PARAM_INOUT:
		retval = c_typename + " *" + c_id;
		break;
	case IDL_PARAM_OUT:
		retval = c_typename + " **" + c_id;
		break;
	}

	return retval;
}

void
IDLUnion::skel_impl_arg_pre (ostream          &ostr,
			     Indent           &indent,
			     const string     &c_id,
			     IDL_param_attr    direction,
			     const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	
	if (direction != IDL_PARAM_IN)
		cerr << "IDLUnion::skel_impl_arg_pre: only IN parameters are handled" << endl;

	const string cpp_type = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();
	
	ostr << indent << cpp_type << " _cpp_" << c_id
	     << " (*" << c_id << ");" << endl;
}
	
string
IDLUnion::skel_impl_arg_call (const string     &c_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	
	if (direction != IDL_PARAM_IN)
		cerr << "IDLUnion::skel_impl_arg_call: only IN parameters are handled" << endl;

	return "_cpp_" + c_id;
}
	
void
IDLUnion::skel_impl_arg_post (ostream          &ostr,
			      Indent           &indent,
			      const string     &c_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	
	if (direction != IDL_PARAM_IN)
		cerr << "IDLUnion::skel_impl_arg_post: only IN parameters are handled" << endl;
}




string
IDLUnion::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLUnion::skel_impl_ret_pre (ostream          &ostr,
			   Indent           &indent,
			   const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLUnion::skel_impl_ret_call (ostream          &ostr,
			    Indent           &indent,
			    const string     &cpp_call_expression,
			    const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLUnion::skel_impl_ret_post (ostream          &ostr,
			    Indent           &indent,
			    const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}


string
IDLUnion::get_cpp_member_typename (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

string
IDLUnion::get_c_member_typename (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

string
IDLUnion::get_seq_typename (unsigned int      length,
			    const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

string
IDLUnion::member_decl_arg_get (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLUnion::member_impl_arg_copy (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_id,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLUnion::member_init_cpp (ostream          &ostr,
			 Indent           &indent,
			 const string     &cpp_id,
			 const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLUnion::member_init_c (ostream          &ostr,
		       Indent           &indent,
		       const string     &c_id,
		       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLUnion::member_pack_to_c (ostream          &ostr,
			  Indent           &indent,
			  const string     &cpp_id,
			  const string     &c_id,
			  const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLUnion::member_unpack_from_c  (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_id,
			       const string     &c_id,
			       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}
