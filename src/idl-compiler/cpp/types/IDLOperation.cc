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
 *  Purpose:	IDL compiler language representation
 *
 */

#include "IDLOperation.hh"

#include "IDLException.hh"
#include "IDLInterface.hh"

string
IDLOperation::stub_ret_get () const
{
	return m_returntype->stub_decl_ret_get ();
}

string 
IDLOperation::stub_arglist_get () const
{
	string retval = "";

	for (ParameterList::const_iterator i = m_parameterinfo.begin ();
	     i != m_parameterinfo.end (); i++)
	{
		retval += i->type->stub_decl_arg_get (i->id, i->direction);

		if (i != --m_parameterinfo.end ())
			retval += ',';
	}

	return retval;
}

string
IDLOperation::stub_decl_proto () const
{
	return stub_ret_get () + " " + get_cpp_identifier () +
		" (" + stub_arglist_get () + ")";
}

string
IDLOperation::stub_decl_impl () const
{
	return stub_ret_get () + " " + get_cpp_typename () +
		" (" + stub_arglist_get () + ")";
}

void
IDLOperation::stub_do_pre (ostream &ostr,
			   Indent  &indent) const
{
	// Prepare parameters
	for (ParameterList::const_iterator i = m_parameterinfo.begin ();
	     i != m_parameterinfo.end (); i++)
	{
		i->type->stub_impl_arg_pre (ostr, indent,
					    i->id, i->direction);
	}

	// Prepare return value container
	m_returntype->stub_impl_ret_pre (ostr, indent);
}

void
IDLOperation::stub_do_call (ostream &ostr,
			    Indent  &indent) const
{
	// Create C exception context
	ostr << indent << IDL_IMPL_NS "::CEnvironment _ev;" << endl;

	// Create argument list
	string argument_list;
		
	for (ParameterList::const_iterator i = m_parameterinfo.begin ();
	     i != m_parameterinfo.end (); i++)
	{
		argument_list += i->type->stub_impl_arg_call (i->id, i->direction);
		argument_list += ", ";
	}

	// Create C call expression
	string c_call_expression =
		get_c_typename () + " (_orbitcpp_get_c_object (), " +
		argument_list + " _ev._orbitcpp_get_c_object ())";
	
	// Do the call
	m_returntype->stub_impl_ret_call (ostr, indent, c_call_expression);

	// Propagate sys exceptions
	ostr << indent << "_ev.propagate_sysex ();" << endl;

	// Handle user exceptions
	ostr << indent << "if (_ev->major == ::CORBA_USER_EXCEPTION)"
	     << indent++ << "{" << endl;

	if (m_raises.size ()) // Are there any known user exceptions?
	{
		ostr << indent << IDL_CORBA_NS "::RepositoryID const repo_id = "
		     << "::CORBA_exception_id (_ev._orbitcpp_get_c_object ());" << endl;
		ostr << indent << "void *value = "
		     << "::CORBA_exception_value (_ev._orbitcpp_get_c_Object ());" << endl
		     << endl;
		
		for (ExceptionList::const_iterator i = m_raises.begin ();
		     i != m_raises.end (); i++)
			{
				(*i)->stub_check_and_propagate (ostr, indent);
			}
	}
		
	// Handle unknown exceptions
	// *** FIXME transfer an any into the exception when any support is here
	ostr << indent << "throw " IDL_CORBA_NS "::UnknownUserException();" << endl;

	ostr << --indent << "}" << endl;
}

void
IDLOperation::stub_do_post (ostream &ostr,
			    Indent  &indent) const
{
	// De-init parameters
	for (ParameterList::const_iterator i = m_parameterinfo.begin ();
	     i != m_parameterinfo.end (); i++)
	{
		i->type->stub_impl_arg_post (ostr, indent,
					    i->id, i->direction);
	}

	// De-init return value container
	m_returntype->stub_impl_ret_post (ostr, indent);
}

string
IDLOperation::skel_ret_get () const
{
	return m_returntype->skel_decl_ret_get ();
}

string 
IDLOperation::skel_arglist_get () const
{
	string retval = "::PortableServer_Servant _servant,";

	for (ParameterList::const_iterator i = m_parameterinfo.begin ();
	     i != m_parameterinfo.end (); i++)
	{
		retval += i->type->skel_decl_arg_get (i->id, i->direction);
		retval += ", ";
	}

	retval += "::CORBA_Environment *_ev";

	return retval;
}

string
IDLOperation::skel_decl_proto () const
{
	return skel_ret_get () + " _skel_" + get_c_identifier () +
		" (" + skel_arglist_get () + ")";
}

void
IDLOperation::skel_do_pre (ostream &ostr,
			   Indent  &indent) const
{
	// Prepare parameters
	for (ParameterList::const_iterator i = m_parameterinfo.begin ();
	     i != m_parameterinfo.end (); i++)
	{
		i->type->skel_impl_arg_pre (ostr, indent,
					    i->id, i->direction);
	}

	// Prepare return value container
	m_returntype->skel_impl_ret_pre (ostr, indent);
}

void
IDLOperation::skel_do_call (ostream &ostr,
			    Indent  &indent) const
{
	ostr << indent << "bool _results_valid = true;" << endl;
	
	// Create C++ exception context
	ostr << indent++ << "try {" << endl;

	// Get C++ instance pointer
	IDLInterface *iface = static_cast<IDLInterface*>(getParentScope ());

	ostr << indent << iface->get_cpp_poa_typename () << " *_self"
	     << " = ((_orbitcpp_Servant *)_servant)->m_cppimpl"
	     << ";" << endl;
	
	// Create argument list
	string argument_list;
		
	for (ParameterList::const_iterator i = m_parameterinfo.begin ();
	     i != m_parameterinfo.end (); i++)
	{
		argument_list += i->type->skel_impl_arg_call (i->id, i->direction);
		if (i != --m_parameterinfo.end ())
			argument_list += ", ";
	}

	// Create C++ call expression
	string cpp_call_expression =
		"_self->" + 
		get_cpp_identifier () + " (" + argument_list + ")";
	
	// Do the call
	m_returntype->skel_impl_ret_call (ostr, indent, cpp_call_expression);

	ostr << --indent << "}" << endl;

	// Catch CORBA exceptions
	ostr << indent++ << "catch (CORBA::Exception &_ex) {" << endl;
	ostr << indent << "_results_valid = false;" << endl
	     << indent << "_ex._orbitcpp_set (_ev);" << endl;
	ostr << --indent << "}";

	// Catch and report unknown exceptions
	ostr << indent++ << "catch (...) {" << endl;
	ostr << indent << "::orbitcpp::error (\"unknown exception in skeleton\");" << endl;
	ostr << --indent << "}" << endl;
}

void
IDLOperation::skel_do_post (ostream &ostr,
			    Indent  &indent) const
{
	// De-init parameters
	for (ParameterList::const_iterator i = m_parameterinfo.begin ();
	     i != m_parameterinfo.end (); i++)
	{
		i->type->skel_impl_arg_post (ostr, indent,
					     i->id, i->direction);
	}

	// De-init return value container
	m_returntype->skel_impl_ret_post (ostr, indent);
}

