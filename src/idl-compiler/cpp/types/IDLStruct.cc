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
	string c_type = active_typedef ?
		active_typedef->get_c_typename () : get_c_typename ();

	// Try to use casts if possible, otherwise use real conversion methods
	if (!conversion_required ())
	{
		string cast;
		
		switch (direction)
		{
		case IDL_PARAM_IN:
			cast = "(const " + c_type + "*)";
			c_type = "const " + c_type;
			break;
		case IDL_PARAM_INOUT:
		case IDL_PARAM_OUT:
			cast = "(" + c_type + "*)";
			break;
		}
		
		ostr << indent << c_type << " *_c_" << cpp_id << " = "
		     << cast + "&" + cpp_id << ";" << endl;

	} else {
		
		switch (direction)
		{
		case IDL_PARAM_IN:
		case IDL_PARAM_INOUT:
			ostr << indent << c_type << " *_c_" << cpp_id << " = "
			     << cpp_id << "._orbitcpp_pack ();" << endl;
			break;
		case IDL_PARAM_OUT:
			ostr << indent << c_type << " *_c_" << cpp_id << ";" << endl;
			break;
		}
	}
}
	
string
IDLStruct::stub_impl_arg_call (const string     &cpp_id,
			       IDL_param_attr    direction,
			       const IDLTypedef *active_typedef) const
{
	if (!is_fixed () && direction == IDL_PARAM_OUT)
		return "&_c_" + cpp_id;
	else
		return "_c_" + cpp_id;
}
	
void
IDLStruct::stub_impl_arg_post (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_id,
			       IDL_param_attr    direction,
			       const IDLTypedef *active_typedef) const
{
	if (!conversion_required ())
		// Do nothing
		return;

	string cpp_type = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();

	// Load back values
	switch (direction)
	{
	case IDL_PARAM_IN:
		// Do nothing
		break;
	case IDL_PARAM_INOUT:
		ostr << indent << cpp_id << "._orbitcpp_unpack "
		     << "(*_c_" << cpp_id << ");" << endl;
		break;
	case IDL_PARAM_OUT:
		if (is_fixed ())
			ostr << indent << cpp_id << "._orbitcpp_unpack "
			     << "(*_c_" << cpp_id << ");" << endl;
		else
			ostr << indent << cpp_id << " = new " << cpp_type << ";" << endl;
			ostr << indent << cpp_id << "->_orbitcpp_unpack "
			     << "(*_c_" << cpp_id << ");" << endl;			
		break;
	}
	
	if (!is_fixed ())
	{
		ostr << indent << "CORBA_free (_c_" << cpp_id << ");" << endl;
	}
}




string
IDLStruct::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
	string cpp_typename = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();

	if (is_fixed ())
		return cpp_typename;
	else
		return cpp_typename + "*";
}
	
void
IDLStruct::stub_impl_ret_pre (ostream &ostr,
			      Indent  &indent,
			      const IDLTypedef *active_typedef) const
{
}

void
IDLStruct::stub_impl_ret_call (ostream          &ostr,
			       Indent           &indent,
			       const string     &c_call_expression,
			       const IDLTypedef *active_typedef) const
{
	string c_typename = active_typedef ?
		active_typedef->get_c_typename () : get_c_typename ();

	string ret_id = is_fixed () ? "_c_retval" : "*_c_retval";

	ostr << indent << c_typename << " " << ret_id << " = "
	     << c_call_expression << ";" << endl;
}

void
IDLStruct::stub_impl_ret_post (ostream          &ostr,
			       Indent           &indent,
			       const IDLTypedef *active_typedef) const
{
	string cpp_typename = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();

	if (!conversion_required ())
	{
		string cast = "(" + cpp_typename + "*)&";
		if (is_fixed ())
			cast = "*" + cast;
			
		ostr << indent << "return " << cast << "_c_retval;" << endl;
	} else {
		if (is_fixed ())
		{
			ostr << indent << cpp_typename << " _cpp_retval;" << endl;
			ostr << indent << "_cpp_retval._orbitcpp_unpack  (_c_retval);" << endl;
		} else {
			ostr << indent << cpp_typename << " *_cpp_retval = "
			     << "new " << cpp_typename << ";" << endl;
			ostr << indent << "_cpp_retval->_orbitcpp_unpack  (*_c_retval);" << endl;
			ostr << indent << "CORBA_free (_c_retval);" << endl;
		}
		
		ostr << indent << "return _cpp_retval;" << endl;
	}
}
	



string
IDLStruct::skel_decl_arg_get (const string     &c_id,
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
		if (is_fixed ())
			retval = c_typename + " *" + c_id;
		else
			retval = c_typename + " **" + c_id;			
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
	string cpp_type = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();
	string cpp_id = "_cpp_" + c_id;
	
	// Try to use casts if possible, otherwise use real conversion methods
	if (!conversion_required ())
	{
		string cast;
		
		switch (direction)
		{
		case IDL_PARAM_IN:
			cast = "(const " + cpp_type + "*)";
			cpp_type = "const " + cpp_type;
			break;
		case IDL_PARAM_INOUT:
		case IDL_PARAM_OUT:
			cast = "(" + cpp_type + "*)";
			break;
		}
		
		ostr << indent << cpp_type << " *" << cpp_id << " = "
		     << cast + "&" + c_id << ";" << endl;

	} else {
		
		switch (direction)
		{
		case IDL_PARAM_IN:
		case IDL_PARAM_INOUT:
			ostr << indent << cpp_type << " " << cpp_id << ";" << endl;
			ostr << indent << cpp_id << "._orbitcpp_unpack (*"
			     << c_id << ");" << endl;
			break;
		case IDL_PARAM_OUT:
			ostr << indent << cpp_type << "_var " << cpp_id << ";" << endl;
			break;
		}
	}
}
	
string
IDLStruct::skel_impl_arg_call (const string     &c_id,
			       IDL_param_attr    direction,
			       const IDLTypedef *active_typedef) const
{
	if (is_fixed ())
		return "*_cpp_" + c_id;

	if (direction == IDL_PARAM_OUT)
		return get_cpp_typename () + "_out (_cpp_" + c_id + ")";

	return "_cpp_" + c_id;
}
	
void
IDLStruct::skel_impl_arg_post (ostream          &ostr,
			       Indent           &indent,
			       const string     &c_id,
			       IDL_param_attr    direction,
			       const IDLTypedef *active_typedef) const
{
	string cpp_id = "_cpp_" + c_id;
	
	if (!conversion_required ())
		// Do nothing
		return;

	// Load back values
	switch (direction)
	{
	case IDL_PARAM_IN:
		// Do nothing
		break;
	case IDL_PARAM_INOUT:
		ostr << indent << cpp_id << "._orbitcpp_pack "
		     << "(*" << c_id << ");" << endl;
		break;
	case IDL_PARAM_OUT:
		ostr << indent << "*" << c_id << " = "
		     << cpp_id << "->_orbitcpp_pack ();" << endl;
		break;
	}
}




string
IDLStruct::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
	string c_typename = active_typedef ?
		active_typedef->get_c_typename () : get_c_typename ();

	if (is_fixed ())
		return c_typename;
	else
		return c_typename + "*";
}

void
IDLStruct::skel_impl_ret_pre (ostream          &ostr,
			      Indent           &indent,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	string cpp_typename = active_typedef ?
		active_typedef->get_cpp_typename () : get_cpp_typename ();
	
	if (is_fixed ())
		ostr << indent << cpp_typename << " _cpp_retval;" << endl;
	else
		ostr << indent << cpp_typename << " *_cpp_retval = 0;" << endl;
}

void
IDLStruct::skel_impl_ret_call (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_call_expression,
			       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

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
		string cast = "(" + c_typename + "*)&";
		if (!is_fixed ())
			cast = "*" + cast;
		
		ostr << indent << "return " << cast << "_cpp_retval;"
		     << endl;
	} else {
		if (is_fixed ())
		{
			ostr << indent << c_typename << " _c_retval;" << endl;
			ostr << indent << "_cpp_retval._orbitcpp_pack (_c_retval);" << endl;
		} else {
			ostr << indent << c_typename << " *_c_retval = "
			     << "_cpp_retval->_orbitcpp_pack ();" << endl;
			ostr << indent << "delete _cpp_retval;" << endl;
		}

		ostr << indent << "return _c_retval;" << endl;
	}
}


string
IDLStruct::get_cpp_member_typename (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

string
IDLStruct::get_c_member_typename (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

string
IDLStruct::member_decl_arg_get (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLStruct::member_impl_arg_copy (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_id,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLStruct::member_init_cpp (ostream          &ostr,
			    Indent           &indent,
			    const string     &cpp_id,
			    const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLStruct::member_init_c (ostream          &ostr,
			  Indent           &indent,
			  const string     &cpp_id,
			  const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLStruct::member_pack_to_c (ostream          &ostr,
			     Indent           &indent,
			     const string     &cpp_id,
			     const string     &c_id,
			     const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLStruct::member_unpack_from_c  (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_id,
			       const string     &c_id,
			       const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}
