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
 *  Purpose:	IDL compiler language representation
 *
 */

#include "IDLMethod.hh"

#include "IDLException.hh"
#include "IDLInterface.hh"

IDLMethod::IDLMethod (IDLInterface &parent_interface_):
	parent_interface (parent_interface_)
{
}

string
IDLMethod::stub_ret_get () const
{
	return m_returntype->stub_decl_ret_get ();
}

string 
IDLMethod::stub_arglist_get () const
{
	string retval = "";

	ParameterList::const_iterator back = m_parameterinfo.end ();
	back--;
	for (ParameterList::const_iterator i = m_parameterinfo.begin ();
	     i != m_parameterinfo.end (); i++)
	{
		retval += i->type->stub_decl_arg_get (i->id, i->direction);

		if (i != back)
			retval += ',';
	}

	return retval;
}

string
IDLMethod::stub_decl_proto () const
{
	return stub_ret_get () + " " + get_cpp_methodname () +
		" (" + stub_arglist_get () + ")";
}

string
IDLMethod::stub_decl_impl () const
{
	string stub_name = parent_interface.get_cpp_stub_method_prefix ();
	
	return stub_ret_get () + " " + stub_name + "::" + get_cpp_methodname () +
	    " (" + stub_arglist_get () + ")";
}

void
IDLMethod::stub_do_pre (ostream &ostr,
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
IDLMethod::stub_do_call (ostream &ostr,
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
		get_c_methodname () + " (_orbitcpp_cobj (), " +
		argument_list + " _ev._orbitcpp_cobj ())";
	
	// Do the call
	m_returntype->stub_impl_ret_call (ostr, indent, c_call_expression);

	// Propagate sys exceptions
	ostr << indent << "_ev.propagate_sysex ();" << endl;

	// Handle user exceptions
	ostr << indent << "if (_ev->_major == ::CORBA_USER_EXCEPTION)" << endl
	     << indent++ << "{" << endl;

	if (m_raises.size ()) // Are there any known user exceptions?
	{
		ostr << indent << IDL_CORBA_NS "::RepositoryId const repo_id = "
		     << "::CORBA_exception_id (_ev._orbitcpp_cobj ());" << endl;
		ostr << indent << "void *value = "
		     << "::CORBA_exception_value (_ev._orbitcpp_cobj ());" << endl
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

	ostr << --indent << "}" << endl << endl;
}

void
IDLMethod::stub_do_post (ostream &ostr,
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
IDLMethod::skel_ret_get () const
{
	return m_returntype->skel_decl_ret_get ();
}

string 
IDLMethod::skel_arglist_get () const
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
IDLMethod::skel_decl_proto () const
{
	return skel_ret_get () + " _skel_" + get_cpp_methodname () +
		" (" + skel_arglist_get () + ")";
}

string
IDLMethod::skel_decl_impl () const
{
	string skel_name = parent_interface.get_cpp_poa_method_prefix ();

	return skel_ret_get () + " " + skel_name + "::" +
		"_skel_" + get_cpp_methodname () +
		" (" + skel_arglist_get () + ")";
}

void
IDLMethod::skel_do_pre (ostream &ostr,
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

	ostr << endl;
	ostr << indent << "bool _results_valid = true;" << endl << endl;
}

void
IDLMethod::skel_do_call (ostream &ostr,
			 Indent  &indent) const
{
	// Create C++ exception context
	ostr << indent++ << "try {" << endl;

	// Get C++ instance pointer
	ostr << indent << parent_interface.get_cpp_poa_typename () << " *_self"
	     << " = ((_orbitcpp_Servant *)_servant)->m_cppimpl"
	     << ";" << endl;
	
	// Create argument list
	string argument_list;
		
	ParameterList::const_iterator back = m_parameterinfo.end ();
	back--;
	for (ParameterList::const_iterator i = m_parameterinfo.begin ();
	     i != m_parameterinfo.end (); i++)
	{
		argument_list += i->type->skel_impl_arg_call (i->id, i->direction);
		if (i != back)
			argument_list += ", ";
	}

	// Create C++ call expression
	string cpp_call_expression =
		"_self->" + 
		get_cpp_methodname () + " (" + argument_list + ")";
	
	// Do the call
	m_returntype->skel_impl_ret_call (ostr, indent, cpp_call_expression);

	ostr << --indent << "}" << endl;

	// Catch CORBA exceptions
	ostr << indent++ << "catch (CORBA::Exception &_ex) {" << endl;
	ostr << indent << "_results_valid = false;" << endl
	     << indent << "_ex._orbitcpp_set (_ev);" << endl;
	ostr << --indent << "}" << endl;

	// Catch and report unknown exceptions
	ostr << indent++ << "catch (...) {" << endl;
	ostr << indent << "::_orbitcpp::error (\"unknown exception in skeleton\");" << endl;
	ostr << --indent << "}" << endl << endl;
}

void
IDLMethod::skel_do_post (ostream &ostr,
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
