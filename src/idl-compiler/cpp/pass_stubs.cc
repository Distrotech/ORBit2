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
	<< indent << "// Stub code -------------------------------------------" << endl
	<< indent << endl;

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



#if 0 //!!!
void 
IDLPassStubs::doAttributePrototype(IDLInterface &iface,IDLInterface &of,IDL_tree node) {

	IDLAttribute &attr = (IDLAttribute &) *of.getItem(node);

	string ret_typespec,ret_typedcl;
	attr.getType()->getCPPStubReturnDeclarator(attr.getCPPIdentifier(),ret_typespec,ret_typedcl);
	m_header <<
	indent << ret_typespec << ' ' << ret_typedcl << '('
			 << ");" << endl;

	if(!attr.isReadOnly()){
		string type,dcl;
		attr.getType()->getCPPStubDeclarator(IDL_PARAM_IN,"val",type,dcl);
		m_header <<
		indent << "void " << attr.getCPPIdentifier() << '(' << type << ' ' << dcl 
			   << ");" << endl;
	}
}
#endif




void 
IDLPassStubs::doOperationPrototype (IDLInterface &iface,
				    IDLInterface &of,
				    IDL_tree      node)
{
	IDLOperation &op = (IDLOperation &) *of.getItem(node);

	m_header << indent << op.stub_decl_proto () << ";" << endl;

	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}



#if 0 //!!!
void 
IDLPassStubs::doAttributeStub(IDLInterface &iface,IDLInterface &of,IDL_tree node) {
	IDLAttribute &attr = (IDLAttribute &) *of.getItem(node);

	// ----------------- get method ---------------------
	string ret_typespec,ret_typedcl;
	attr.getType()->getCPPStubReturnDeclarator(
		   iface.getQualifiedCPPStub(iface.getRootScope()) + "::" + attr.getCPPIdentifier(),
		   ret_typespec,ret_typedcl);
	m_module
	<< mod_indent << ret_typespec << ' ' << ret_typedcl << "()" << endl
	<< mod_indent << "{" << endl;
	mod_indent++;
	attr.getType()->writeCPPStubReturnPrepCode(m_module,mod_indent);

	// do the call
	m_module
	<< mod_indent << IDL_IMPL_NS "::CEnvironment _ev;" << endl;

	m_module
	<< mod_indent << attr.getType()->getCPPStubReturnAssignment()
	<< IDL_IMPL_C_NS_NOTUSED <<  iface.getQualifiedCIdentifier()<< "__get_" << attr.getCIdentifier() << "(_orbitcpp_get_c_object(),_ev._orbitcpp_get_c_object());" << endl;

	m_module
	<< mod_indent << "_ev.propagate_sysex();" << endl;

	attr.getType()->writeCPPStubReturnDemarshalCode(m_module,mod_indent);

	mod_indent--;
	m_module << mod_indent << "}" << endl;
	
	// ----------------- set method ---------------------

	
	if(!attr.isReadOnly()){
		string type,dcl;

		attr.getType()->getCPPStubDeclarator(IDL_PARAM_IN,"val",type,dcl);
		
		m_module
		 <<	mod_indent << "void "
		 << iface.getQualifiedCPPStub(iface.getRootScope()) << "::" << attr.getCPPIdentifier() << '(' << type << ' ' << dcl << ")" << endl
		 << mod_indent << "{" << endl;

		mod_indent++;
		// marshal the parameter
		attr.getType()->writeCPPStubMarshalCode(IDL_PARAM_IN,"val",m_module,mod_indent);

		// do the call
		m_module
		  << mod_indent << IDL_IMPL_NS "::CEnvironment _ev;" << endl;

		m_module
			<< mod_indent << IDL_IMPL_C_NS_NOTUSED <<  iface.getQualifiedCIdentifier()
			<< "__set_" << attr.getCIdentifier() << "(_orbitcpp_get_c_object(), "
			<<attr.getType()->getCPPStubParameterTerm(IDL_PARAM_IN,"val");
		m_module
			<<", _ev._orbitcpp_get_c_object());" << endl;

		m_module
		  << mod_indent << "_ev.propagate_sysex();" << endl;

		mod_indent--;
		m_module << mod_indent << "}" << endl;
	}
	

}
#endif




void 
IDLPassStubs::doOperationStub (IDLInterface &iface,
			       IDLInterface &of,
			       IDL_tree      node)
{
	IDLOperation &op = (IDLOperation &) *of.getItem(node);

	// print prototype
	m_module << mod_indent << op.stub_decl_impl () << endl
		 << mod_indent++ << "{" << endl;

	// Prepare parameters and return value container
	op.stub_do_pre (m_module, mod_indent);

	// Do the call
	op.stub_do_call (m_module, mod_indent);
	
	// De-init parameters and return value container,
	// FIXME: handle return value
	op.stub_do_post (m_module, mod_indent);
	
	// End of stub implementation
	m_module << --mod_indent << "}" << endl;

	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}




void 
IDLPassStubs::doInterfaceDownCall(IDLInterface &iface,IDLInterface &of) {
	IDL_tree body_list = IDL_INTERFACE(of.getNode()).body;
	while (body_list) {
		switch (IDL_NODE_TYPE(IDL_LIST(body_list).data)) {
#if 0 //!!!
		case IDLN_ATTR_DCL:
			doAttributePrototype(iface,of,IDL_LIST(body_list).data);
			doAttributeStub(iface,of,IDL_LIST(body_list).data);
			break;
#endif
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
	m_header << --indent++ << "protected:" << endl;

	// constructors
	m_header << indent << iface.get_cpp_stub_identifier () << " ();" << endl;
	m_header << indent << iface.get_cpp_stub_identifier ()
		 << " (" << iface.get_cpp_identifier () << " const &src);" << endl;
	m_header << indent << iface.get_cpp_stub_identifier ()
		 << " (" << iface.get_c_typename () << " cobject, bool take_copy = false); //orbitcpp-specific" << endl << endl;
		
	// begin public section
	m_header << --indent++ << "public:" << endl;

	// _orbitcpp wrap method:
	m_header << indent << "static " << iface.get_cpp_stub_identifier ()
		 << "* _orbitcpp_wrap (" << iface.get_c_typename () << " cobject, bool take_copy = false);" << endl << endl;

	// end orbitcpp internal section


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

	// Methods
	for (IDLInterface::BaseList::const_iterator i = iface.m_all_mi_bases.begin ();
	     i != iface.m_all_mi_bases.end (); i++)
	{
		doInterfaceDownCall (iface, *(*i));
	}
	doInterfaceDownCall (iface, iface);
}

