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
 *  Purpose: stub generation pass
 *
 */




#include "error.hh"
#include "pass_stubs.hh"

#include "types/IDLOperation.hh"
#include "types/IDLAttribAccessor.hh"


// IDLPassStubs --------------------------------------------------------------
void 
IDLPassStubs::runPass() {
	m_header
	<< indent << "#ifndef __ORBITCPP_IDL_" << idlUpper(m_state.m_basename) << "_STUBS" << endl
	<< indent << "#define __ORBITCPP_IDL_" << idlUpper(m_state.m_basename) << "_STUBS" << endl
	<< indent << endl << endl
	<< indent << "#include <string.h>" << endl
	<< indent << "#include \"" << m_state.m_basename << IDL_CPP_HEADER_EXT"\"" << endl
	<< indent << endl << endl
	<< indent << "// Stub declaration ------------------------------------" << endl
	<< indent << endl;

	m_module
	<< indent << "#include \"" << m_state.m_basename << IDL_CPP_STUB_HEADER_EXT"\"" << endl
	<< indent << endl << endl
	<< indent << "// Stub code -------------------------------------------" << endl << endl;

	vector<IDLInterface *>::iterator
	first = m_state.m_interfaces.begin(), last = m_state.m_interfaces.end();
	while (first != last)
	{
		IDLInterface* pInterface = *first;
		if(pInterface)
			doInterface(*pInterface);

		first++;
	}
	
	runJobs();

	m_header << endl << indent << "#endif" << endl;
}

void
IDLPassStubs::create_method_proto (const IDLMethod &method)
{
	m_header << indent << method.stub_decl_proto () << ";" << endl;
}

void
IDLPassStubs::create_method_stub (const IDLMethod &method)
{
	// print prototype
	m_module << mod_indent << method.stub_decl_impl () << endl
		 << mod_indent++ << "{" << endl;

	// Prepare parameters and return value container
	method.stub_do_pre (m_module, mod_indent);
	m_module << endl;
	
	// Do the call
	method.stub_do_call (m_module, mod_indent);
	m_module << endl;
	
	// De-init parameters and return value container,
	// FIXME: handle return value
	method.stub_do_post (m_module, mod_indent);
	
	// End of stub implementation
	m_module << --mod_indent << "}" << endl << endl;
}

void 
IDLPassStubs::doAttributePrototype (IDLInterface &iface,
				    IDLInterface &of,
				    IDL_tree      node)
{
	IDLAttribute &attr = (IDLAttribute &) *of.getItem(node);

	// Getter
	create_method_proto (IDLAttribGetter (attr));

	// Setter
	if (!attr.isReadOnly ())
		create_method_proto (IDLAttribSetter (attr));
	
	m_header << endl;
}

void 
IDLPassStubs::doAttributeStub (IDLInterface &iface,
			       IDLInterface &of,
			       IDL_tree      node)
{
	IDLAttribute &attr = (IDLAttribute &) *of.getItem(node);

	// Getter
	create_method_stub (IDLAttribGetter (attr));

	// Setter
	if (!attr.isReadOnly ())
		create_method_stub (IDLAttribSetter (attr));
}




void 
IDLPassStubs::doOperationPrototype (IDLInterface &iface,
				    IDLInterface &of,
				    IDL_tree      node)
{
	IDLOperation &op = (IDLOperation &) *of.getItem(node);

	create_method_proto (op);

	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}

void 
IDLPassStubs::doOperationStub (IDLInterface &iface,
			       IDLInterface &of,
			       IDL_tree      node)
{
	IDLOperation &op = (IDLOperation &) *of.getItem(node);

	create_method_stub (op);

	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}




void 
IDLPassStubs::doInterfaceDownCall(IDLInterface &iface,IDLInterface &of) {
	IDL_tree body_list = IDL_INTERFACE(of.getNode()).body;
	while (body_list) {
		switch (IDL_NODE_TYPE(IDL_LIST(body_list).data)) {
		case IDLN_ATTR_DCL:
			doAttributePrototype(iface,of,IDL_LIST(body_list).data);
			doAttributeStub(iface,of,IDL_LIST(body_list).data);
			break;
		case IDLN_OP_DCL:
			doOperationPrototype(iface,of,IDL_LIST(body_list).data);
			doOperationStub(iface,of,IDL_LIST(body_list).data);
			break;
		default:
			break;
		}
		body_list = IDL_LIST(body_list).next;
	}
}




void 
IDLPassStubs::doInterface(IDLInterface &iface)
{
	string ns_begin, ns_end;
	iface.getParentScope()->getCPPNamespaceDecl (ns_begin, ns_end);

	// begin namespace
	m_header << indent << "namespace " IDL_IMPL_NS_ID " { "
		 << "namespace " IDL_IMPL_STUB_NS_ID " { " << endl << endl
		 << indent << ns_begin << endl << endl;
	

	// class declaration
	m_header << indent << "class " << iface.get_cpp_stub_identifier () << ": ";

	string base_name;
	if (iface.m_bases.size () > 0)
		base_name = iface.m_bases[0]->get_cpp_stub_typename ();
	else
		base_name = IDL_CORBA_NS "::Object";
	

	m_header << "public " << base_name;
	
	// order matters! with gcc, even the empty type container classes
	// have a non-empty binary footprint (size 1). thus they must go at
	// the end, or else all we get is segfaults.
	m_header << ", public " << iface.get_cpp_typename () << endl
		 << indent++ << "{" << endl;
	
	// begin orbitcpp internal section
	m_header << indent << "// These should really be private, but we make them protected" << endl;
	m_header << indent << "//  to stop the compiler from generating warnings" << endl;
	m_header << --indent << "protected:" << endl;
	indent++;

	// constructors
	m_header << indent << iface.get_cpp_stub_identifier () << " ();" << endl;
	m_header << indent << iface.get_cpp_stub_identifier ()
		 << " (" << iface.get_cpp_identifier () << " const &src);" << endl;
	m_header << indent << iface.get_cpp_stub_identifier ()
		 << " (" << iface.get_c_typename () << " cobject, bool take_copy = false); //orbitcpp-specific" << endl << endl;
		
	// begin public section
	m_header << --indent << "public:" << endl;
	indent++;

	// _orbitcpp wrap method:
	m_header << indent << "static " << iface.get_cpp_stub_identifier ()
		 << "* _orbitcpp_wrap (" << iface.get_c_typename () << " cobject, bool take_copy = false);" << endl << endl;

	// end orbitcpp internal section


	// Methods
	for (IDLInterface::BaseList::const_iterator i = iface.m_all_mi_bases.begin ();
	     i != iface.m_all_mi_bases.end (); i++)
	{
		doInterfaceDownCall (iface, *(*i));
	}
	doInterfaceDownCall (iface, iface);

#if 0
	// make cast operators for all mi base classes
 	IDLInterface::BaseList::const_iterator
	first = iface.m_all_mi_bases.begin(),last = iface.m_all_mi_bases.end();

	// I've taken this code out because the cast operators are never
	// called - all casts are done with stub ptrs, not stub
	// values. (See the new MI _ptr smart ptr classes for the casts
	// that get used) - PD
	/*
	while (first != last) {
		m_header 
		<< indent << "operator " << (*first)->getQualifiedCPPIdentifier() << " &() {" << endl;
		m_header
		<< ++indent << "return *" << (*first)->getQualifiedCPPCast("this") << ';' << endl;
		m_header
		<< --indent << '}' << endl;
		first++;
	}
	*/
#endif

	// end main class
	m_header << --indent << "};" << endl << endl;

	// end namespace
	m_header << indent << ns_end << "}} //namespaces" << endl << endl;
	
	//Implementation:
	
	//Constructor implemention:
	m_module << mod_indent
		 << iface.get_cpp_stub_typename () << "::"
		 << iface.get_cpp_stub_identifier ()
		 << " (" << iface.get_c_typename () << " cobject, bool take_copy /*= false */):" << endl;
	m_module << mod_indent << base_name << "(cobject, take_copy)" << endl
		 << mod_indent << "{}" << endl << endl;

	// _orbitcpp wrap method:
	m_module << mod_indent
		 << iface.get_cpp_stub_typename () << " * "
		 << iface.get_cpp_stub_typename () << "::"
		 << "_orbitcpp_wrap"
		 << " (" << iface.get_c_typename () << " cobject, bool take_copy /* = false */)" << endl
		 << mod_indent++ << "{" << endl;
	m_module << mod_indent << "return new " << iface.get_cpp_stub_typename ()
		 << " (cobject, take_copy);" << endl;
	m_module << --mod_indent << "}" << endl << endl;
}

