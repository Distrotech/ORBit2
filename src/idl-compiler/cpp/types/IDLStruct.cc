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

#include "IDLStruct.hh"

#include "IDLTypedef.hh"

IDLStruct::IDLStruct (const string &id,
		      IDL_tree      node,
		      IDLScope     *parentscope = 0):
	IDLCompound (id, node, parentscope)
{
}

bool
IDLStruct::conversion_required () const
{
	bool have_nonsimple_member = false;
	for (const_iterator i = begin ();
	     i != end () && !have_nonsimple_member;
	     i++)
	{
		IDLMember &member = (IDLMember &) **i;
		have_nonsimple_member = member.getType ()->conversion_required ();
	}

	return have_nonsimple_member;
}

bool
IDLStruct::is_fixed () const
{
	bool fixed = true;
	for (const_iterator i = begin ();
	     i != end () && fixed;
	     i++)
	{
		IDLMember &member = (IDLMember &) **i;
		fixed = member.getType ()->is_fixed ();
	}

	return fixed;
}

void
IDLStruct::typedef_decl_write (ostream          &ostr,
			       Indent           &indent,
			       IDLCompilerState &state,
			       const IDLTypedef &target,
			       const IDLTypedef *active_typedef = 0) const
{
#warning "WRITE ME"
}

string
IDLStruct::stub_decl_arg_get (const string     &cpp_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	string retval;
	string cpp_typename = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();

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
IDLStruct::stub_impl_arg_pre (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	if (!conversion_required ())
		// Do nothing
		return;
}
	
string
IDLStruct::stub_impl_arg_call (const string     &cpp_id,
			       IDL_param_attr    direction,
			       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	string c_type = active_typedef ?
		active_typedef->get_c_typename () : get_c_typename ();
	string retval;
	
	if (!conversion_required ())
	{
		string cast;
		
		switch (direction)
		{
		case IDL_PARAM_IN:
			cast = "(const " + c_type + "*)";
			break;
		case IDL_PARAM_INOUT:
		case IDL_PARAM_OUT:
			cast = "(" + c_type + "*)";
			break;
		}

		retval = cast + "&" + cpp_id;
	}

	return retval;
}
	
void
IDLStruct::stub_impl_arg_post (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_id,
			       IDL_param_attr    direction,
			       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	if (!conversion_required ())
		// Do nothing
		return;
}




string
IDLStruct::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	string cpp_typename = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();

	return cpp_typename;
}
	
void
IDLStruct::stub_impl_ret_pre (ostream &ostr,
			      Indent  &indent,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLStruct::stub_impl_ret_call (ostream          &ostr,
			       Indent           &indent,
			       const string     &c_call_expression,
			       const IDLTypedef *active_typedef) const
{
	string c_typename = active_typedef ?
		active_typedef->get_c_typename () : get_c_typename ();

	if (!conversion_required ())
	{
		ostr << indent << c_typename << " _c_retval = "
		     << c_call_expression << ";" << endl;
	}
}

void
IDLStruct::stub_impl_ret_post (ostream          &ostr,
			       Indent           &indent,
			       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	string cpp_typename = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();

	if (!conversion_required ())
	{
		string cast = "*(" + cpp_typename + "*)&";
		ostr << indent << "return " << cast << "_c_retval;" << endl;
	}
}
	



string
IDLStruct::skel_decl_arg_get (const string     &c_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	string c_typename = active_typedef ?
		active_typedef->get_c_typename () : get_c_typename ();
	string retval;

	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "const " + c_typename + " *" + c_id;
		break;
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		retval = c_typename + " *" + c_id;
		break;		
	}

	return retval;
}

void
IDLStruct::skel_impl_arg_pre (ostream          &ostr,
			      Indent           &indent,
			      const string     &c_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	if (!conversion_required ())
		// Do nothing
		return;
}
	
string
IDLStruct::skel_impl_arg_call (const string     &c_id,
			       IDL_param_attr    direction,
			       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	string cpp_typename = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();
	string retval;
	
	if (!conversion_required ())
	{
		string cast;
		
		switch (direction)
		{
		case IDL_PARAM_IN:
			cast = "(const " + cpp_typename + "*)";
			break;
		case IDL_PARAM_INOUT:
		case IDL_PARAM_OUT:
			cast = "(" + cpp_typename + "*)";
			break;
		}

		retval = "*" + cast + c_id;
	}

	return retval;
}
	
void
IDLStruct::skel_impl_arg_post (ostream          &ostr,
			       Indent           &indent,
			       const string     &c_id,
			       IDL_param_attr    direction,
			       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	if (!conversion_required ())
		// Do nothing
		return;
}




string
IDLStruct::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	string c_typename = active_typedef ?
		active_typedef->get_c_typename () : get_c_typename ();

	return c_typename;
}

void
IDLStruct::skel_impl_ret_pre (ostream          &ostr,
			      Indent           &indent,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	string cpp_typename = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();
	
	if (!conversion_required ())
		ostr << indent << cpp_typename << " _cpp_retval;" << endl;
}

void
IDLStruct::skel_impl_ret_call (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_call_expression,
			       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	if (!conversion_required ())
		ostr << indent << "_cpp_retval = " << cpp_call_expression
		     << ";" << endl;
}

void
IDLStruct::skel_impl_ret_post (ostream          &ostr,
			       Indent           &indent,
			       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	string c_typename = active_typedef ?
		active_typedef->get_c_typename () : get_c_typename ();
	
	if (!conversion_required ())
	{
		string cast = "*(" + c_typename + "*)&";
		
		ostr << indent << "return " << cast << "_cpp_retval;"
		     << endl;
	}
}


string
IDLStruct::get_cpp_member_typename (const IDLTypedef *active_typedef) const
{
}

string
IDLStruct::get_c_member_typename (const IDLTypedef *active_typedef) const
{
}

string
IDLStruct::member_decl_arg_get (const IDLTypedef *active_typedef) const
{
}

void
IDLStruct::member_impl_arg_copy (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_id,
			      const IDLTypedef *active_typedef) const
{
}

void
IDLStruct::member_init_cpp (ostream          &ostr,
			    Indent           &indent,
			    const string     &cpp_id,
			    const IDLTypedef *active_typedef) const
{
}

void
IDLStruct::member_init_c (ostream          &ostr,
			  Indent           &indent,
			  const string     &cpp_id,
			  const IDLTypedef *active_typedef) const
{
}

void
IDLStruct::member_pack_to_c (ostream          &ostr,
			     Indent           &indent,
			     const string     &cpp_id,
			     const string     &c_id,
			     const IDLTypedef *active_typedef) const
{
}

void
IDLStruct::member_unpack_from_c  (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_id,
			       const string     &c_id,
			       const IDLTypedef *active_typedef) const
{
}
