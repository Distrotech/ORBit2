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

IDLSequence::IDLSequence (const IDLType &element_type,
			  unsigned int   length) :
	m_element_type (element_type),
	m_length (length)
{
}

bool
IDLSequence::is_fixed () const
{
	return false;
}

void
IDLSequence::const_decl_write (ostream          &ostr,
			  Indent           &indent,
			  const string     &cpp_id,
			  const string     &value,
			  const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLSequence::typedef_decl_write (ostream          &ostr,
				 Indent           &indent,
				 IDLCompilerState &state,
				 const IDLTypedef &target,
				 const IDLTypedef *active_typedef) const
{
	string cpp_type = target.get_cpp_identifier ();
	string cpp_elem = m_element_type.get_cpp_member_typename ();
	string c_elem = m_element_type.get_c_member_typename ();
	string cpp_traits = m_element_type.get_seq_traits_typename ();
	string c_type = target.get_c_typename ();
	
	string base_type = IDL_IMPL_NS "::";
	if (m_length)
	{
		// Bounded sequence
		char *base_tmp = g_strdup_printf (
			"BoundedSequence < %s, %s, %s, %s, %d >",
			cpp_elem.c_str (), c_elem.c_str (),
			cpp_traits.c_str (), c_type.c_str (),
			m_length);
		
		base_type += base_tmp;
		g_free (base_tmp);
		
	} else {
		
		// Unbounded sequence
		char *base_tmp = g_strdup_printf (
			"UnboundedSequence < %s, %s, %s, %s >",
			cpp_elem.c_str (), c_elem.c_str (),
			cpp_traits.c_str (), c_type.c_str ());
		
		base_type += base_tmp;
		g_free (base_tmp);
	};
	
	ostr << indent << "class " << cpp_type << ": public " << base_type << endl
	     << indent++ << "{" << endl;
	ostr << "typedef " << base_type << " Super;" << endl;
	
	ostr << endl << --indent << "public:" << endl;
	indent++;

	ostr << indent << cpp_type << " (size_t max = 0) : Super (max) {};" << endl;

	ostr << indent << cpp_type << " (size_t max, size_t length, "
	     << "buffer_t buffer, CORBA::Boolean release = false) : "
	     << "Super (max, length, buffer, release) {};" << endl;
	
	ostr << indent << cpp_type << " (const " << cpp_type << " &other) : "
	     << "Super (other) {};" << endl;

	ostr << endl << --indent << "protected:" << endl;
	indent++;

	ostr << indent << "c_seq_t* alloc_c () const {" << endl;
	ostr << indent++ << "return " << c_type << "__alloc ();" << endl;
	ostr << --indent << "}" << endl << endl;

	ostr << indent << "c_value_t* alloc_c_buf (CORBA::ULong length) const {" << endl;
	ostr << indent++ << "return " << c_type << "_allocbuf (length);" << endl;
	ostr << --indent << "}" << endl;
	
	ostr << --indent << "};" << endl << endl;

	// Create smart pointers
	ostr << indent << "typedef " << IDL_IMPL_NS << "::Sequence_var<" << cpp_type << "> "
	     << cpp_type << "_var;" << endl;
	ostr << indent << "typedef " << IDL_IMPL_NS << "::Sequence_out<" << cpp_type << "> "
	     << cpp_type << "_out;" << endl;
}

string
IDLSequence::stub_decl_arg_get (const string     &cpp_id,
				IDL_param_attr    direction,
				const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	
	string retval;
	string cpp_type = active_typedef->get_cpp_typename ();
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "const " + cpp_type + " &" + cpp_id;
		break;
	case IDL_PARAM_INOUT:
		retval = cpp_type + " &" + cpp_id;
		break;
	case IDL_PARAM_OUT:
		retval = cpp_type + "_out " + cpp_id;
		break;
	}

	return retval;
}

void
IDLSequence::stub_impl_arg_pre (ostream          &ostr,
				Indent           &indent,
				const string     &cpp_id,
				IDL_param_attr    direction,
				const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	
	string c_id = "_c_" + cpp_id;
	string c_type = active_typedef->get_c_typename ();
	
	ostr << indent << c_type << " *" << c_id << ";" << endl;

	if (direction == IDL_PARAM_IN || direction == IDL_PARAM_INOUT)
		ostr << indent << c_id << " = " << cpp_id << "._orbitcpp_pack ();" << endl;
		
}
	
string
IDLSequence::stub_impl_arg_call (const string     &cpp_id,
				 IDL_param_attr    direction,
				 const IDLTypedef *active_typedef) const
{
	if (direction == IDL_PARAM_OUT)
		return "&_c_" + cpp_id;
	
	return "_c_" + cpp_id;
}
	
void
IDLSequence::stub_impl_arg_post (ostream          &ostr,
				 Indent           &indent,
				 const string     &cpp_id,
				 IDL_param_attr    direction,
				 const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	string cpp_type = active_typedef->get_cpp_typename ();
	
	if (direction == IDL_PARAM_INOUT)
	{
		// Load back values
		ostr << indent << cpp_id << "._orbitcpp_unpack ("
		     << "*_c_" << cpp_id << ");" << endl;
	}

	if (direction == IDL_PARAM_OUT)
	{
		ostr << indent << cpp_id << " = new " << cpp_type << ";" << endl;
		ostr << indent << cpp_id << "->_orbitcpp_unpack ("
		     << "*_c_" << cpp_id << ");" << endl;
	}
	
	ostr << indent << "CORBA_free (_c_" << cpp_id << ");" << endl;
}




string
IDLSequence::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	return active_typedef->get_cpp_typename () + "*";
}
	
void
IDLSequence::stub_impl_ret_pre (ostream &ostr,
				Indent  &indent,
				const IDLTypedef *active_typedef) const
{
	// Do nothing
}

void
IDLSequence::stub_impl_ret_call (ostream          &ostr,
				 Indent           &indent,
				 const string     &c_call_expression,
				 const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	
	ostr << indent << active_typedef->get_c_typename () << " *_c_retval"
	     << " = " << c_call_expression << ";" << endl;
}

void
IDLSequence::stub_impl_ret_post (ostream          &ostr,
				 Indent           &indent,
				 const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	string cpp_type = active_typedef->get_cpp_typename ();
	
	ostr << indent << cpp_type << " *_cpp_retval = new "
	     << cpp_type << ";" << endl;
	ostr << indent << "_cpp_retval->_orbitcpp_unpack (*_c_retval);" << endl;
	ostr << indent << "CORBA_free (_c_retval);" << endl << endl;
	ostr << indent << "return _cpp_retval;" << endl;
}
	



string
IDLSequence::skel_decl_arg_get (const string     &c_id,
				IDL_param_attr    direction,
				const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	
	string retval;
	string c_type = active_typedef->get_c_typename ();
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "const " + c_type + " *" + c_id;
		break;
	case IDL_PARAM_INOUT:
		retval = c_type + " *" + c_id;
		break;
	case IDL_PARAM_OUT:
		retval = c_type + " **" + c_id;
		break;
	}

	return retval;	
}

void
IDLSequence::skel_impl_arg_pre (ostream          &ostr,
				Indent           &indent,
				const string     &c_id,
				IDL_param_attr    direction,
				const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	
	string cpp_id = "_cpp_" + c_id;
	string cpp_type = active_typedef->get_cpp_typename ();

	switch (direction)
	{
	case IDL_PARAM_IN:
	case IDL_PARAM_INOUT:
		ostr << indent << cpp_type << " " << cpp_id << ";" << endl;
		ostr << indent << cpp_id << "._orbitcpp_unpack (*" << c_id << ");" << endl;
		break;
	case IDL_PARAM_OUT:
		ostr << indent << cpp_type << "_var " << cpp_id << ";" << endl;
		break;
	}
}
	
string
IDLSequence::skel_impl_arg_call (const string     &c_id,
				 IDL_param_attr    direction,
				 const IDLTypedef *active_typedef) const
{
	return "_cpp_" + c_id;
}
	
void
IDLSequence::skel_impl_arg_post (ostream          &ostr,
				 Indent           &indent,
				 const string     &c_id,
				 IDL_param_attr    direction,
				 const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	string cpp_type = active_typedef->get_cpp_typename ();
	
	if (direction == IDL_PARAM_INOUT)
		ostr << indent << "_cpp_" << c_id << "._orbitcpp_pack"
		     << " (*" << c_id << ");" << endl;

	if (direction == IDL_PARAM_OUT)
	{
		ostr << indent << "*" << c_id << " = "
		     << "_cpp_" << c_id << "->_orbitcpp_pack ();" << endl;
	}
}




string
IDLSequence::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);

	return active_typedef->get_c_typename () + "*";
}

void
IDLSequence::skel_impl_ret_pre (ostream          &ostr,
				Indent           &indent,
				const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);

	ostr << indent << active_typedef->get_cpp_typename ()
	     << "_var _cpp_retval;" << endl;
}

void
IDLSequence::skel_impl_ret_call (ostream          &ostr,
				 Indent           &indent,
				 const string     &cpp_call_expression,
				 const IDLTypedef *active_typedef) const
{
	ostr << indent << "_cpp_retval = " << cpp_call_expression << ";" << endl;
}

void
IDLSequence::skel_impl_ret_post (ostream          &ostr,
				 Indent           &indent,
				 const IDLTypedef *active_typedef) const
{
	ostr << indent << "return _cpp_retval->_orbitcpp_pack ();" << endl << endl;
}


string
IDLSequence::get_cpp_member_typename (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	g_assert (active_typedef);

	return active_typedef->get_cpp_typename () + "_var";
}

string
IDLSequence::get_c_member_typename (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	g_assert (active_typedef);

	return active_typedef->get_c_typename () + "*";
}

string
IDLSequence::member_decl_arg_get (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
	return "";
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
			 const IDLTypedef *active_typedef) const
{
}

void
IDLSequence::member_init_c (ostream          &ostr,
		       Indent           &indent,
		       const string     &c_id,
		       const IDLTypedef *active_typedef) const
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
