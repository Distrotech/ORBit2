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

#include "IDLArray.hh"

#include "IDLTypedef.hh"

IDLArray::IDLArray (IDLType const &element_type,
		    string const  &id,
		    IDL_tree       node,
		    IDLScope      *parentscope = 0) :
	IDLElement  (id, node, parentscope),
	m_element_type (element_type)
{
	for (IDL_tree curdim = IDL_TYPE_ARRAY (node).size_list;
	     curdim; curdim = IDL_LIST (curdim).next)
	{
		m_dims.push_back (IDL_INTEGER (IDL_LIST (curdim).data).value);
	}
}

void
IDLArray::fill_c_array (ostream      &ostr,
			Indent       &indent,
			const string &cpp_id,
			const string &c_id) const
{
	unsigned int depth = 0;
	string array_pos;
	for (const_iterator i = begin (); i != end (); i++, depth++)
	{
		gchar *iterator_name = g_strdup_printf ("i_%d", depth);
		array_pos += "[";
		array_pos += iterator_name;
		array_pos += "]";

		ostr << indent
		     << "for (CORBA::ULong " << iterator_name << " = 0; "
		     << iterator_name << " < " << *i << "; "
		     << iterator_name << "++)" << endl;
		ostr << indent++ << "{" << endl;

		g_free (iterator_name);
	}

	string cpp_member = cpp_id + array_pos;
	string c_member = c_id + array_pos;
	m_element_type.member_pack_to_c (ostr, indent, cpp_member, c_member);

	for (; depth; depth--)
	{
		ostr << --indent << "}" << endl;
	}
}

void
IDLArray::fill_cpp_array (ostream      &ostr,
			  Indent       &indent,
			  const string &cpp_id,
			  const string &c_id) const
{
	unsigned int depth = 0;
	string array_pos;
	for (const_iterator i = begin (); i != end (); i++, depth++)
	{
		gchar *iterator_name = g_strdup_printf ("i_%d", depth);
		array_pos += "[";
		array_pos += iterator_name;
		array_pos += "]";

		ostr << indent
		     << "for (CORBA::ULong " << iterator_name << " = 0; "
		     << iterator_name << " < " << *i << "; "
		     << iterator_name << "++)" << endl;
		ostr << indent++ << "{" << endl;

		g_free (iterator_name);
	}

	string cpp_member = cpp_id + array_pos;
	string c_member = c_id + array_pos;
	m_element_type.member_unpack_from_c (ostr, indent, cpp_member, c_member);

	for (; depth; depth--)
	{
		ostr << --indent << "}" << endl;
	}
}

void
IDLArray::typedef_decl_write (ostream          &ostr,
			      Indent           &indent,
			      const IDLTypedef &target,
			      const IDLTypedef *active_typedef = 0) const
{
#warning "WRITE ME"
	// Create array typedef
	ostr << indent << "typedef "
	     << m_element_type.get_cpp_member_typename ()
	     << ' ' << target.get_cpp_identifier ();

	// Write dimensions
	for (const_iterator i = begin (); i != end (); i++)
		ostr << '[' << *i << ']';
	ostr << ';' << endl;
	

	// Create slice typedef
	ostr << indent << "typedef "
	     << m_element_type.get_cpp_member_typename ()
	     << ' ' << target.get_cpp_identifier () << "_slice";

	// An array slice is all the dims except the first one
	for (const_iterator i = ++begin (); i != end (); i++)
		ostr << '[' << *i << ']';
	ostr << ';' << endl;

	// Create _out typedef
	if (!m_element_type.conversion_required ())
	{
		// _out
		ostr << indent << "typedef " << target.get_cpp_identifier ()
		     << " " << target.get_cpp_identifier () << "_out;"
		     << endl << endl;

		// Allocator
		ostr << indent << "inline " << target.get_cpp_identifier () << "_slice * " 
		     << target.get_cpp_identifier () << "_alloc ()" << endl
		     << indent++ << "{" << endl;
		ostr << indent << "return " << target.get_c_typename () << "__alloc ();" << endl;
		ostr << --indent << "}" << endl << endl;
			
		return;
	}

	ostr << indent << "typedef " << IDL_IMPL_NS "::ArrayVariable_out< "
	     << target.get_cpp_identifier () << "_slice, " << *(begin ()) << " > "
	     << target.get_cpp_identifier () << "_out;" << endl;
	ostr << indent << "typedef " << IDL_IMPL_NS "::ArrayVariable_var< "
	     << target.get_cpp_identifier () << "_slice, " << *(begin ()) << " > "
	     << target.get_cpp_identifier () << "_var;" << endl;
}

string
IDLArray::stub_decl_arg_get (const string     &cpp_id,
			     IDL_param_attr    direction,
			     const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	
	string retval;

	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "const " + active_typedef->get_cpp_typename () + 
			" " + cpp_id;	
		break;
	case IDL_PARAM_INOUT:
		retval = active_typedef->get_cpp_typename () + 
			" " + cpp_id;	
		break;
	case IDL_PARAM_OUT:
		retval = active_typedef->get_cpp_typename () + 
			"_out " + cpp_id;	
		break;
	}

	return retval;
}

void
IDLArray::stub_impl_arg_pre (ostream          &ostr,
			     Indent           &indent,
			     const string     &cpp_id,
			     IDL_param_attr    direction,
			     const IDLTypedef *active_typedef = 0) const
{
	g_assert (active_typedef);
	
	if (!m_element_type.conversion_required ())
		return;

	if (direction == IDL_PARAM_OUT)
	{
		// Create slice pointer and do nothing with it
		ostr << indent << active_typedef->get_c_typename ()
		     << "_slice *_c_" << cpp_id << ";" << endl;
		return;
	}
	
	// Create and fill C array
	ostr << indent << active_typedef->get_c_typename ()
	     << " _c_" << cpp_id << ';' << endl;

	fill_c_array (ostr, indent, cpp_id, "_c_" + cpp_id);
}
	
string
IDLArray::stub_impl_arg_call (const string   &cpp_id,
			      IDL_param_attr  direction,
			      const IDLTypedef *active_typedef = 0) const
{
	if (!m_element_type.conversion_required ())
		return cpp_id;

	string retval;

	switch (direction)
	{
	case IDL_PARAM_IN:
	case IDL_PARAM_INOUT:
		retval = "_c_" + cpp_id;
		break;
	case IDL_PARAM_OUT:
		retval = "&_c_" + cpp_id;
		break;
	}

	return retval;
}
	
void
IDLArray::stub_impl_arg_post (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef = 0) const
{
	if (!m_element_type.conversion_required ())
		return;

	if (direction == IDL_PARAM_IN)
		// No need to load IN parameters back
		return;
	
	// Re-load from C array
	fill_cpp_array (ostr, indent, cpp_id, "_c_" + cpp_id);
}




string
IDLArray::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	
	return active_typedef->get_cpp_typename () + "_slice *";
}
	
void
IDLArray::stub_impl_ret_pre (ostream          &ostr,
			     Indent           &indent,
			     const IDLTypedef *active_typedef = 0) const
{
	// Do nothing
}

void
IDLArray::stub_impl_ret_call (ostream          &ostr,
			      Indent           &indent,
			      const string     &c_call_expression,
			      const IDLTypedef *active_typedef = 0) const
{
	g_assert (active_typedef);
	
	ostr << indent << active_typedef->get_c_typename () << "_slice *_retval = "
	     << c_call_expression << ";" << endl;
}

void
IDLArray::stub_impl_ret_post (ostream          &ostr,
			      Indent           &indent,
			      const IDLTypedef *active_typedef = 0) const
{
	g_assert (active_typedef);
	
	if (!m_element_type.conversion_required ())
	{
		ostr << indent << "return _retval;" << endl;
		return;
	}
	
	// Create and fill C++ array
	ostr << indent << active_typedef->get_cpp_typename ()
	     << " _cpp_retval;" << endl;

	fill_c_array (ostr, indent, "_cpp_retval", "_retval");
	
	ostr << indent << active_typedef->get_c_typename ()
	     << "__freekids (_retval, 0);" << endl;

	ostr << indent << "return _cpp_retval;" << endl;
}
	



string
IDLArray::skel_decl_arg_get (const string     &c_id,
			     IDL_param_attr    direction,
			     const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	
	string retval;

	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = "const " + active_typedef->get_c_typename () + 
			" " + c_id;	
		break;
	case IDL_PARAM_INOUT:
		retval = active_typedef->get_c_typename () + 
			" " + c_id;	
		break;
	case IDL_PARAM_OUT:
		if (m_element_type.conversion_required ())
			retval = active_typedef->get_c_typename () + "_slice" +
				" **" + c_id;	
		else
			retval = active_typedef->get_c_typename () +
				" " + c_id;
		break;
	}

	return retval;
}

void
IDLArray::skel_impl_arg_pre (ostream          &ostr,
			     Indent           &indent,
			     const string     &c_id,
			     IDL_param_attr    direction,
			     const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	
	if (!m_element_type.conversion_required ())
		// Do nothing
		return;

	switch (direction)
	{
	case IDL_PARAM_IN:
		ostr << indent << active_typedef->get_cpp_typename ()
		     << " _cpp_" << c_id << ";" << endl;
		fill_cpp_array (ostr, indent, "_cpp_" + c_id, c_id);
		break;
		
	case IDL_PARAM_INOUT:
		ostr << indent << active_typedef->get_cpp_typename ()
		     << "_slice *_cpp_" << c_id << ";" << endl;
		fill_cpp_array (ostr, indent, "_cpp_" + c_id, c_id);
		break;

	case IDL_PARAM_OUT:
		ostr << indent << active_typedef->get_cpp_typename ()
		     << "_slice *_cpp_" << c_id << ";" << endl;
		break;
	}

	ostr << endl;
}
	
string
IDLArray::skel_impl_arg_call (const string     &c_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"

	if (!m_element_type.conversion_required ())
		return c_id;
	else
		return "_cpp_" + c_id;
}
	
void
IDLArray::skel_impl_arg_post (ostream          &ostr,
			      Indent           &indent,
			      const string     &c_id,
			      IDL_param_attr    direction,
			      const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);

	if (!m_element_type.conversion_required ())
		// Do nothing
		return;
	
	if (direction == IDL_PARAM_IN)
		// No need to reload IN parameters
		return;
		
	// Load C array from C++
	string cpp_root = "_cpp_" + c_id;
	string c_root = c_id;
	if (direction == IDL_PARAM_OUT)
	{
		// We need to allocate OUT parameters ourselves
		c_root = "*" + c_root;
		ostr << indent << c_root << " = "
		     << active_typedef->get_c_typename () << "__alloc ()"
		     << ";" << endl;
	}

	fill_c_array (ostr, indent, cpp_root, c_root);

	ostr << endl;
}




string
IDLArray::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);

	return active_typedef->get_c_typename () + "_slice *";
}

void
IDLArray::skel_impl_ret_pre (ostream          &ostr,
			     Indent           &indent,
			     const IDLTypedef *active_typedef) const
{
	ostr << indent << active_typedef->get_cpp_typename ()
	     << "_slice *_retval = 0;" << endl;
}

void
IDLArray::skel_impl_ret_call (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_call_expression,
			      const IDLTypedef *active_typedef) const
{
	ostr << indent << "_retval = " << cpp_call_expression << ";" << endl;
}

void
IDLArray::skel_impl_ret_post (ostream          &ostr,
			      Indent           &indent,
			      const IDLTypedef *active_typedef) const
{
	g_assert (active_typedef);
	
	if (!m_element_type.conversion_required ())
	{
		ostr << indent << "return _retval;" << endl;
		return;
	}

	// Create C return value and fill it
	ostr << indent << active_typedef->get_c_typename ()
	     << "_slice *_c_retval = "
	     << active_typedef->get_c_typename () << "__alloc ()"
	     << ";" << endl;

	fill_c_array (ostr, indent, "_retval", "_c_retval");

	ostr << indent << "return _c_retval;" << endl;
}


string
IDLArray::get_cpp_member_typename (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

string
IDLArray::get_c_member_typename (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

string
IDLArray::member_decl_arg_get (const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLArray::member_impl_arg_copy (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_id,
			      const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLArray::member_pack_to_c (ostream          &ostr,
			    Indent           &indent,
			    const string     &cpp_id,
			    const string     &c_id,
			    const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}

void
IDLArray::member_unpack_from_c  (ostream          &ostr,
				 Indent           &indent,
				 const string     &cpp_id,
				 const string     &c_id,
				 const IDLTypedef *active_typedef) const
{
#warning "WRITE ME"
}
