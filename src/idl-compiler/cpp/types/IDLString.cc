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

#include "IDLString.hh"

#include "IDLTypedef.hh"

void
IDLString::const_decl_write (ostream          &ostr,
			     Indent           &indent,
			     const string     &cpp_id,
			     const string     &value,
			     const IDLTypedef *active_typedef) const
{
	ostr << indent << "const char *" << cpp_id << " = " << value
	     << ';' << endl;
}

void
IDLString::typedef_decl_write (ostream          &ostr,
			       Indent           &indent,
			       IDLCompilerState &state,
			       const IDLTypedef &target,
			       const IDLTypedef *active_typedef) const
{
	string target_id = target.get_cpp_identifier ();
	
	ostr << indent << "typedef char * " << target_id << ';' << endl;	
	ostr << indent << "typedef ::CORBA::String_var " << target_id << "_var;" << endl;
}

string
IDLString::stub_decl_arg_get (const string     &cpp_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
	string retval;

	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "char const *" + cpp_id;
		break;
	case IDL_PARAM_INOUT:
		retval = "char *&" + cpp_id;
		break;
	case IDL_PARAM_OUT:
		retval = "::CORBA::String_out " + cpp_id;
		break;
	}

	return retval;
}

void
IDLString::stub_impl_arg_pre (ostream        &ostr,
			      Indent         &indent,
			      const string   &cpp_id,
			      IDL_param_attr  direction,
			      const IDLTypedef *active_typedef) const
{
	// Do nothing
}
	
string
IDLString::stub_impl_arg_call (const string   &cpp_id,
			       IDL_param_attr  direction,
			       const IDLTypedef *active_typedef) const
{
	string retval;

	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = cpp_id;
		break;
	case IDL_PARAM_INOUT:
		retval = '&' + cpp_id;
		break;
	case IDL_PARAM_OUT:
		retval = "&(char*&) " + cpp_id;
		break;
	}

	return retval;
}
	
void
IDLString::stub_impl_arg_post (ostream        &ostr,
			       Indent         &indent,
			       const string   &cpp_id,
			       IDL_param_attr  direction,
			       const IDLTypedef *active_typedef) const
{
	// Do nothing
}




string
IDLString::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
	return "char *";
}
	
void
IDLString::stub_impl_ret_pre (ostream &ostr,
			      Indent  &indent,
			      const IDLTypedef *active_typedef) const
{
	// Do nothing
}

void
IDLString::stub_impl_ret_call (ostream      &ostr,
			       Indent       &indent,
			       const string &c_call_expression,
			       const IDLTypedef *active_typedef) const
{
	ostr << indent << "char *_retval = " << c_call_expression
	     << ';' << endl;
}

void
IDLString::stub_impl_ret_post (ostream &ostr,
			       Indent  &indent,
			       const IDLTypedef *active_typedef) const
{
	ostr << indent << "return _retval;" << endl;
}
	



string
IDLString::skel_decl_arg_get (const string     &c_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "char const *" + c_id;
		break;
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		retval = "char **" + c_id;
		break;
	}

	return retval;	
}

void
IDLString::skel_impl_arg_pre (ostream        &ostr,
			      Indent         &indent,
			      const string   &c_id,
			      IDL_param_attr  direction,
			      const IDLTypedef *active_typedef) const
{
	// Do nothing
}
	
string
IDLString::skel_impl_arg_call (const string   &c_id,
			       IDL_param_attr  direction,
			       const IDLTypedef *active_typedef) const
{
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = c_id;
		break;
	case IDL_PARAM_INOUT:
		retval = "*" + c_id;
		break;
	case IDL_PARAM_OUT:
		retval = "::CORBA::String_out (*" + c_id + ")";
		break;
	}

	return retval;	
}
	
void
IDLString::skel_impl_arg_post (ostream        &ostr,
			       Indent         &indent,
			       const string   &c_id,
			       IDL_param_attr  direction,
			       const IDLTypedef *active_typedef) const
{
	// Do nothing
}




string
IDLString::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
	return "char *";
}

void
IDLString::skel_impl_ret_pre (ostream &ostr,
			      Indent  &indent,
			      const IDLTypedef *active_typedef) const
{
	ostr << indent << "char *_retval = 0"
	     << ';' << endl;
}

void
IDLString::skel_impl_ret_call (ostream      &ostr,
			       Indent       &indent,
			       const string &cpp_call_expression,
			       const IDLTypedef *active_typedef) const
{
	ostr << indent << "_retval = " << cpp_call_expression
	     << ';' << endl;
}

void
IDLString::skel_impl_ret_post (ostream &ostr,
			       Indent  &indent,
			       const IDLTypedef *active_typedef) const
{
	ostr << indent << "return _retval;" << endl;
}


string
IDLString::get_cpp_member_typename (const IDLTypedef *active_typedef) const
{
	return "CORBA::String_mgr";
}

string
IDLString::get_c_member_typename (const IDLTypedef *active_typedef) const
{
	return "char*";
}

string
IDLString::get_seq_typename (unsigned int      length,
			     const IDLTypedef *active_typedef) const
{
	if (!length)
		return IDL_IMPL_NS "::StringUnboundedSeq";

	char *tmp = g_strdup_printf (IDL_IMPL_NS "::StringBoundedSeq<%d>",
				     length);

	string retval = tmp;
	g_free (tmp);
	return retval;
}

string
IDLString::member_decl_arg_get (const IDLTypedef *active_typedef) const
{
	return "const char *";
}

void
IDLString::member_impl_arg_copy (ostream      &ostr,
				 Indent       &indent,
				 const string &cpp_id,
				 const IDLTypedef *active_typedef) const
{
	ostr << indent << cpp_id << " = _par_" << cpp_id
	     << ';' << endl;
}

void
IDLString::member_init_cpp (ostream          &ostr,
			    Indent           &indent,
			    const string     &cpp_id,
			    const IDLTypedef *active_typedef) const
{
	ostr << indent << cpp_id << " = CORBA::string_dup (\"\");" << endl;
}

void
IDLString::member_init_c (ostream          &ostr,
			  Indent           &indent,
			  const string     &c_id,
			  const IDLTypedef *active_typedef) const
{
	ostr << indent << c_id << " = CORBA::string_dup (\"\");" << endl;
}

void
IDLString::member_pack_to_c (ostream      &ostr,
			     Indent       &indent,
			     const string &cpp_id,
			     const string &c_id,
			     const IDLTypedef *active_typedef) const
{
	ostr << indent << c_id << " = "
	     << "CORBA::string_dup (" << cpp_id << ")"
	     << ';' << endl;
}

void
IDLString::member_unpack_from_c (ostream      &ostr,
				 Indent       &indent,
				 const string &cpp_id,
				 const string &c_id,
				 const IDLTypedef *active_typedef) const
{
#if 0
	ostr << indent << "if (" << cpp_id << ")" << endl
	     << ++indent << "CORBA::string_free (" << cpp_id << ");"
	     << endl;
	--indent;
#endif
	
	ostr << indent << cpp_id << " = CORBA::string_dup (" << c_id << ")"
	     << ';' << endl;
}
