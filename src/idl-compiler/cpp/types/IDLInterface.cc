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

#include "IDLInterface.hh"


/***************************************************
 * Internal
 ***************************************************/

namespace
{
	
string get_c_id (const string &cpp_id)
{
	return "_c_" + cpp_id;
}

string get_cpp_id (const string &c_id)
{
	return "_cpp_" + c_id;
}

} // Anonymous namespace

bool
IDLInterface::isBaseClass (IDLInterface *iface)
{
	for (BaseList::const_iterator i = m_allbases.begin ();
	     i != m_allbases.end (); i++)
	{
		if (*i == iface)
			return true;
	}
	
        return false;
}

string
IDLInterface::get_cpp_typename_ptr () const
{
	return get_cpp_typename () + "_ptr";
}

string
IDLInterface::get_cpp_typename_var () const
{
	return get_cpp_typename () + "_var";
}

string
IDLInterface::get_cpp_typename_mgr () const
{
	return get_cpp_typename () + "_mgr";
}

string
IDLInterface::get_cpp_typename_out () const
{
	return get_cpp_typename () + "_out";
}

string
IDLInterface::get_cpp_identifier_ptr () const
{
	return get_cpp_identifier () + "_ptr";
}

string
IDLInterface::get_cpp_identifier_var () const
{
	return get_cpp_identifier () + "_var";
}

string
IDLInterface::get_cpp_identifier_mgr () const
{
	return get_cpp_identifier () + "_mgr";
}

string
IDLInterface::get_cpp_identifier_out () const
{
	return get_cpp_identifier () + "_out";
}

string
IDLInterface::get_cpp_stub_identifier () const
{
	return get_cpp_identifier ();
}

string
IDLInterface::get_cpp_stub_typename () const
{
	return IDL_IMPL_STUB_NS + get_cpp_typename ();
}

string
IDLInterface::get_cpp_stub_method_prefix () const
{
	string retval = get_cpp_stub_typename ();

	// Remove :: from head
	string::iterator i = retval.begin ();
	while (i != retval.end () && *i == ':')
		i = retval.erase (i);
	
	return retval;
}


string
IDLInterface::get_cpp_poa_identifier () const
{
	return get_cpp_identifier ();
}

string
IDLInterface::get_cpp_poa_typename () const
{
	return "::" + get_cpp_poa_method_prefix ();
}

string
IDLInterface::get_cpp_poa_method_prefix () const
{
	string cpp_typename = get_cpp_typename ();

	// Remove :: from head
	string::iterator i = cpp_typename.begin ();
	while (i != cpp_typename.end () && *i == ':')
		i = cpp_typename.erase (i);
	
	return "POA_" + cpp_typename;	
}

string
IDLInterface::get_c_poa_typename () const
{
	return "POA_" + get_c_typename ();
}

string
IDLInterface::get_c_poa_epv () const
{
	return get_c_poa_typename () + "__epv";
}

string
IDLInterface::get_c_poa_vepv () const
{
	return get_c_poa_typename () + "__vepv";
}

void
IDLInterface::get_cpp_poa_namespace (string &ns_begin,
				     string &ns_end) const
{
	getParentScope()->getCPPNamespaceDecl (ns_begin, ns_end, "POA_");
}

/***************************************************
 * Stub
 ***************************************************/

string
IDLInterface::stub_decl_arg_get (const string     &cpp_id,
				 IDL_param_attr    direction,
				 const IDLTypedef *active_typedef) const
{
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_IN:
		retval = get_cpp_typename_ptr () + " " + cpp_id;
		break;
	case IDL_PARAM_INOUT:
		retval = get_cpp_typename_ptr () + " &" + cpp_id;
		break;
	case IDL_PARAM_OUT:
		retval = get_cpp_typename_out () + " " + cpp_id;
		break;
	}

	return retval;
}

void
IDLInterface::stub_impl_arg_pre (ostream        &ostr,
				 Indent         &indent,
				 const string   &cpp_id,
				 IDL_param_attr  direction) const
{
	switch (direction)
	{
	case IDL_PARAM_IN:
		ostr << indent << "const " << get_c_typename () << " "
		     << get_c_id (cpp_id)
		     << " = " << cpp_id << "->_orbitcpp_get_c_object ();"
		     << endl;
		break;

	case IDL_PARAM_INOUT:
		ostr << indent << get_c_typename () << " "
		     << get_c_id (cpp_id)
		     << " = " << cpp_id << "->_orbitcpp_get_object ();"
		     << endl;
		break;

	case IDL_PARAM_OUT:
		ostr << indent << get_c_typename () << " "
		     << get_c_id (cpp_id)
		     << " = " << "CORBA_OBJECT_NIL;"
		     << endl;
		break;
	}
}
	
string
IDLInterface::stub_impl_arg_call (const string   &cpp_id,
				  IDL_param_attr  direction) const
{
	string retval;
	
	switch (direction)
	{
	case IDL_PARAM_INOUT:
		retval = get_c_id (cpp_id);
		break;
	case IDL_PARAM_OUT:
		retval = "&" + get_c_id (cpp_id);
		break;
	}

	return retval;
}
	
void
IDLInterface::stub_impl_arg_post (ostream        &ostr,
				  Indent         &indent,
				  const string   &cpp_id,
				  IDL_param_attr  direction) const
{
	switch (direction)
	{
	case IDL_PARAM_IN:
		// Do nothing
		break;
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		ostr << indent << cpp_id << " = "
		     << get_cpp_stub_typename () << "::wrap"
		     << " (" << get_c_id (cpp_id) << ");"
		     << endl;
	}
}




string
IDLInterface::stub_decl_ret_get (const IDLTypedef *active_typedef) const
{
}
	
void
IDLInterface::stub_impl_ret_pre (ostream &ostr,
				 Indent  &indent) const
{
	// Do nothing
}

void
IDLInterface::stub_impl_ret_call (ostream      &ostr,
				  Indent       &indent,
				  const string &c_call_expression) const
{
	ostr << indent << get_c_typename () << " _retval = "
	     << c_call_expression << ";" << endl;
}

void
IDLInterface::stub_impl_ret_post (ostream &ostr,
				  Indent  &indent) const
{
	// WRITE ME
}
	



/***************************************************
 * Skel
 ***************************************************/

string
IDLInterface::skel_decl_arg_get (const string     &c_id,
				 IDL_param_attr    direction,
				 const IDLTypedef *active_typedef) const
{
	switch (direction)
	{
	case IDL_PARAM_IN:
		return get_c_typename () + " " + c_id;
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		return get_c_typename () + " *" + c_id;
	}
}

void
IDLInterface::skel_impl_arg_pre (ostream        &ostr,
				 Indent         &indent,
				 const string   &c_id,
				 IDL_param_attr  direction) const
{
	// WRITE ME
}
	
string
IDLInterface::skel_impl_arg_call (const string   &c_id,
				  IDL_param_attr  direction) const
{
	// WRITE ME
}
	
void
IDLInterface::skel_impl_arg_post (ostream        &ostr,
				  Indent         &indent,
				  const string   &c_id,
				  IDL_param_attr  direction) const
{
	// WRITE ME
}




string
IDLInterface::skel_decl_ret_get (const IDLTypedef *active_typedef) const
{
	// WRITE ME
}

void
IDLInterface::skel_impl_ret_pre (ostream &ostr,
				 Indent  &indent) const
{
	ostr << indent << get_cpp_typename_ptr () << " _retval"
	     << ';' << endl;
}

void
IDLInterface::skel_impl_ret_call (ostream      &ostr,
				  Indent       &indent,
				  const string &cpp_call_expression) const
{
	ostr << indent << " _retval = " << cpp_call_expression
	     << ';' << endl;
}

void
IDLInterface::skel_impl_ret_post (ostream &ostr,
				  Indent  &indent) const
{
	// WRITE ME
}


/***************************************************
 * Member
 ***************************************************/

string
IDLInterface::get_cpp_member_typename () const
{
	return get_cpp_typename_mgr ();
}

string
IDLInterface::member_decl_arg_get () const
{
	return get_cpp_typename_ptr ();
}
	
void
IDLInterface::member_impl_arg_copy (ostream      &ostr,
				    Indent       &indent,
				    const string &cpp_id) const
{
	// WRITE ME
	ostr << indent << cpp_id << " = _par_" << cpp_id
	     << ';' << endl;
}

void
IDLInterface::member_pack_to_c_pre  (ostream      &ostr,
				     Indent       &indent,
				     const string &member_id,
				     const string &c_struct_id) const
{
	// Do nothing
}

void
IDLInterface::member_pack_to_c_pack (ostream      &ostr,
				     Indent       &indent,
				     const string &member_id,
				     const string &c_struct_id) const
{
	string c_id = c_struct_id + '.' + member_id;
	string cpp_id = member_id;
	
	ostr << indent << c_id << " = " << cpp_id << "->_orbitcpp_get_object ()"
	     << ';' << endl;
}

void
IDLInterface::member_pack_to_c_post (ostream      &ostr,
				Indent       &indent,
				const string &member_id,
				const string &c_struct_id) const
{
	// Do nothing
}

void
IDLInterface::member_unpack_from_c_pre  (ostream      &ostr,
				    Indent       &indent,
				    const string &member_id,
				    const string &c_struct_id) const
{
	// Do nothing
}

void
IDLInterface::member_unpack_from_c_pack (ostream      &ostr,
				    Indent       &indent,
				    const string &member_id,
				    const string &c_struct_id) const
{
	string c_id = c_struct_id + '.' + member_id;
	string cpp_id = member_id;
	
	ostr << indent << cpp_id << " = "
	     << get_cpp_stub_typename () << "::_orbitcpp_wrap ("
	     << "::_orbitcpp::duplicate_guarded (" << c_id << ")"
	     << ")"
	     << ';' << endl;
}

void
IDLInterface::member_unpack_from_c_post  (ostream      &ostr,
				     Indent       &indent,
				     const string &member_id,
				     const string &c_struct_id) const
{
	// Do nothing
}

bool
IDLInterface::need_smartptr () const
{
	for (IDLInterface::BaseList::const_iterator i = m_allbases.begin ();
	     i != m_allbases.end (); i++)
	{
                if ((*i)->m_all_mi_bases.size())
                        return true;
	}
	
        return false;
}

void
IDLInterface::create_smartptr (ostream &ostr,
			       Indent  &indent) const
{
	string ptr_name = get_cpp_identifier_ptr ();
	string stub_name = get_cpp_stub_typename ();
	
	ostr << indent << "class " << ptr_name
	     << indent++ << "{" << endl;



	ostr << indent << stub_name << " *m_target;" << endl;

	ostr << --indent++ << "public:" << endl;

	ostr << indent << ptr_name << "() : m_target (0) {};" << endl;
	ostr << indent << ptr_name << "(" << stub_name <<" *ptr) : m_target(ptr) {};" << endl;

	ostr << indent << ptr_name << " &operator =(" << stub_name << " *ptr)" << endl
	     << indent++ << '{' << endl;
	ostr << "m_target = ptr;" << endl
	     << "return *this;" << endl
	     << --indent << '}' << endl;

	ostr << indent << stub_name << " * operator-> ()" << endl
	     << indent++ << '{' << endl;
	ostr << "return m_target;" << endl
	     << --indent << '}' << endl;

#if 0 // FIXME: Is this necessary?
	// Commented out
	ostr << indent << "// operator CORBA::Object * () " << endl
	     << indent++ << "// {" << endl;
	ostr << indent << "// return reinterpret_cast<CORBA::Object*>(m_target);"
	     << --indent << "// }" << endl;


	IDLInterface::BaseList::const_iterator
	first, last = iface.m_allbases.end();

	for (first = iface.m_allbases.begin();first != last;first++) {
		m_header
		<< indent << "//operator " << (*first)->getQualifiedCPPStub() << " *() { return reinterpret_cast< " << (*first)->getQualifiedCPPStub() << " *>(m_target);}" << endl;
	}
	
	m_header
	<< indent <<"// These shouldn't be necessary, but gcc 2.95.2 barfs without them"<<endl;
	for (first = iface.m_allbases.begin();first != last;first++) {
		m_header
		<< indent << "//operator " << (*first)->getQualifiedCPP_var() << "() { return reinterpret_cast< " << (*first)->getQualifiedCPPStub() << " *>(m_target);}" << endl;
	}
	m_header
		<< --indent << "};" << endl;
#endif
}

void
IDLInterface::common_write_typedefs (ostream &ostr,
				     Indent  &indent) const
{
	// check the MI situation and write a smartptr if required.
	if (need_smartptr ())
	{
		create_smartptr (ostr, indent);

		ostr << indent << "typedef " IDL_IMPL_NS "::ObjectSmartPtr_var"
		     << "<" << get_cpp_identifier () << ", " << get_cpp_identifier_ptr () << "> "
		     << get_cpp_identifier () << "_var;" << endl;

	} else {
		
		ostr << indent << "typedef " << get_cpp_stub_typename ()
		     << "* " << get_cpp_identifier_ptr ()
		     << ';' << endl;
		
		ostr << indent << "typedef "IDL_IMPL_NS "::ObjectPtr_var"
		     << "<" << get_cpp_identifier () << ", " << get_cpp_identifier_ptr () << "> "
		     << get_cpp_identifier_var ()
		     << ';' << endl;
	}
	
	
	ostr << indent << "typedef " << get_cpp_identifier_var ()
	     << " " << get_cpp_identifier_mgr ()
	     << ';' << endl;

	ostr << indent << "typedef " IDL_IMPL_NS "::ObjectPtr_out"
	     << "<" << get_cpp_identifier () << ", " << get_cpp_identifier_ptr () << "> "
	     << get_cpp_identifier_out ()
	     << ';' << endl;

	ostr << indent << "typedef " << get_cpp_identifier_ptr ()
	     << " " << get_cpp_identifier () << "Ref"
	     << ';' << endl;
	
}
