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
	<< indent << "#include <strings.h>" << endl
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
		doInterface(**first++);
	
	runJobs();

	m_header << endl << indent << "#endif" << endl;
}




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





void 
IDLPassStubs::doOperationPrototype(IDLInterface &iface,IDLInterface &of,IDL_tree node) {
	IDLOperation &op = (IDLOperation &) *of.getItem(node);

	string ret_typespec,ret_typedcl;
	op.m_returntype->getCPPStubReturnDeclarator(op.getCPPIdentifier(),ret_typespec,ret_typedcl);
	m_header <<
	indent << ret_typespec << ' ' << ret_typedcl << '('
	<< op.getCPPOpParameterList() << ");" << endl;

	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}




void 
IDLPassStubs::doAttributeStub(IDLInterface &iface,IDLInterface &of,IDL_tree node) {
	IDLAttribute &attr = (IDLAttribute &) *of.getItem(node);

	// ----------------- get method ---------------------
	string ret_typespec,ret_typedcl;
	attr.getType()->getCPPStubReturnDeclarator(
		   iface.getQualifiedCPPStub(iface.getRootScope()) + "::" + attr.getCPPIdentifier(),
		   ret_typespec,ret_typedcl);
	m_module <<
	mod_indent << ret_typespec << ' ' << ret_typedcl << "() {" << endl;
	mod_indent++;
	attr.getType()->writeCPPStubReturnPrepCode(m_module,mod_indent);

	// do the call
	m_module
	<< mod_indent << IDL_IMPL_NS "::CEnvironment _ev;" << endl;

	m_module
	<< mod_indent << attr.getType()->getCPPStubReturnAssignment()
	<< IDL_IMPL_C_NS "::" <<  iface.getQualifiedCIdentifier()<< "__get_" << attr.getCIdentifier() << "(_orbitcpp_get_c_object(),_ev);" << endl;

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
		 << iface.getQualifiedCPPStub(iface.getRootScope()) << "::" << attr.getCPPIdentifier()
		 << '(' << type << ' ' << dcl << ") {" << endl;

		mod_indent++;
		// marshal the parameter
		attr.getType()->writeCPPStubMarshalCode(IDL_PARAM_IN,"val",m_module,mod_indent);

		// do the call
		m_module
		  << mod_indent << IDL_IMPL_NS "::CEnvironment _ev;" << endl;

		m_module
			<< mod_indent << IDL_IMPL_C_NS "::" <<  iface.getQualifiedCIdentifier()
			<< "__set_" << attr.getCIdentifier() << "(_orbitcpp_get_c_object(),"
			<<attr.getType()->getCPPStubParameterTerm(IDL_PARAM_IN,"val");
		m_module
			<<",_ev);" << endl;

		m_module
		  << mod_indent << "_ev.propagate_sysex();" << endl;

		mod_indent--;
		m_module << mod_indent << "}" << endl;
	}
	

}





void 
IDLPassStubs::doOperationStub(IDLInterface &iface,IDLInterface &of,IDL_tree node) {
	IDLOperation &op = (IDLOperation &) *of.getItem(node);

	// print prototype
	string ret_typespec,ret_typedcl;
	op.m_returntype->getCPPStubReturnDeclarator(
		iface.getQualifiedCPPStub(iface.getRootScope()) + "::" + op.getCPPIdentifier(),
		ret_typespec,ret_typedcl);

	m_module
	<< mod_indent << ret_typespec << ' ' << ret_typedcl
	<< '(' << op.getCPPOpParameterList() << ") {" << endl;
	mod_indent++;


	// marshal parameters
	vector<IDLOperation::ParameterInfo>::const_iterator
	first = op.m_parameterinfo.begin(),last = op.m_parameterinfo.end();

	while (first != last) {
		first->Type->writeCPPStubMarshalCode(first->Direction,first->Identifier,m_module,mod_indent);
		first++;
	}

	op.m_returntype->writeCPPStubReturnPrepCode(m_module,mod_indent);

	// do the call
	m_module
	<< mod_indent << IDL_IMPL_NS "::CEnvironment _ev;" << endl;

	m_module
	<< mod_indent << op.m_returntype->getCPPStubReturnAssignment()
	<< IDL_IMPL_C_NS "::" <<  op.getQualifiedCIdentifier() << "(_orbitcpp_get_c_object(),";

	first = op.m_parameterinfo.begin();
	last = op.m_parameterinfo.end();

	while (first != last) {
		m_module << first->Type->getCPPStubParameterTerm(first->Direction,first->Identifier) << ',';
		first++;
	}
	m_module << "_ev);" << endl;

	// handle exceptions
	m_module
	<< mod_indent << "_ev.propagate_sysex();" << endl;

	m_module
	<< mod_indent << "if (_ev->_major == "
	<<  "::CORBA_USER_EXCEPTION) {" << endl;
	mod_indent++;

	vector<IDLException *>::const_iterator
	exfirst = op.m_raises.begin(),exlast = op.m_raises.end();

	if (exfirst != exlast) { // no exceptions => every user ex is unknown
		m_module
		<< mod_indent << IDL_CORBA_NS "::RepositoryId const repo_id = "
		<<  "::CORBA_exception_id(_ev);" << endl
		<< mod_indent << "void *value = "
		<<  "::CORBA_exception_value(_ev);" << endl << endl;

		while (exfirst != exlast && *exfirst) {
			m_module
			<< mod_indent << "if (strcmp(repo_id,ex_" << (*exfirst)->getQualifiedCIdentifier()
			<< ") == 0) {" << endl;
			mod_indent++;

			m_module
			<< mod_indent << (*exfirst)->getQualifiedCPPIdentifier() << " ex;" << endl
			<< mod_indent << "ex._orbitcpp_unpack(*(("
			<< IDL_IMPL_C_NS "::" << (*exfirst)->getQualifiedCIdentifier() <<" *) value));" << endl
			<< mod_indent << "throw ex;" << endl
			<< mod_indent << '}' << endl;
			mod_indent--;

			exfirst++;
		}
	}

	// *** FIXME transfer an any into the exception when any support is here
	m_module
	<< mod_indent << "throw " IDL_CORBA_NS "::UnknownUserException();" << endl;
	mod_indent--;
	m_module
	<< mod_indent << '}' << endl;

	// demarshal parameters
	first = op.m_parameterinfo.begin();
	last = op.m_parameterinfo.end();

	while (first != last) {
		first->Type->writeCPPStubDemarshalCode(first->Direction,first->Identifier,m_module,mod_indent);
		first++;
	}

	op.m_returntype->writeCPPStubReturnDemarshalCode(m_module,mod_indent);

	// end stub
	mod_indent--;
	m_module
	<< mod_indent << '}' << endl << endl;

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
IDLPassStubs::doInterface(IDLInterface &iface) {	
	string ifname = iface.getCPPIdentifier();

	string ns_begin,ns_end;
	iface.getParentScope()->getCPPNamespaceDecl(ns_begin,ns_end);

	// begin namespace
	m_header
	<< indent << "namespace " IDL_IMPL_NS_ID " { "
	<< "namespace " IDL_IMPL_STUB_NS_ID " { "
	<< ns_begin << endl;
	indent++;

	// class declaration
	m_header
	<< indent << "class " << iface.getCPPStub() << " : ";
	
	if (iface.m_bases.size() > 0) 
		m_header
		<< "public " << iface.m_bases[0]->getQualifiedCPPStub();
	else
		m_header
		<< "public "IDL_CORBA_NS "::Object";
	
	// order matters! with gcc, even the empty type container classes
	// have a non-empty binary footprint (size 1). thus they must go at
	// the end, or else all we get is segfaults.
    m_header
	<< ", public " << iface.getQualifiedCPPIdentifier() << " {" << endl;

	// begin orbitcpp internal section
	m_header << indent << "// These should really be private, but we make them protected" << endl;
	m_header << indent << "//  to stop the compiler from generating warnings" << endl;
	m_header << indent << "protected:" << endl;
	indent++;

	// constructors (no public ones, esp. no copy constructor)
	m_header 
	<< indent << iface.getCPPStub() << "();" << endl
	<< indent << iface.getCPPStub() << "(" << iface.getCPPStub() << " const &src);" << endl;

	// end orbitcpp internal section
	indent--;

	// begin public section
	m_header << indent << "public:" << endl;
	indent++;
		
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

	
	// translate operations (same thing as above)
	first = iface.m_all_mi_bases.begin();
	last = iface.m_all_mi_bases.end();

	while (first != last)
		doInterfaceDownCall(iface,**first++);
	doInterfaceDownCall(iface,iface);

	// end public section
	indent--;

	// end main class
	m_header << indent << "};" << endl;

	// end namespace
	indent--;
	m_header << indent << ns_end << "}}" << endl;

	// write the static method definitions
	doInterfaceStaticMethodDefinitions(iface);
}


void IDLPassStubs::doInterfaceStaticMethodDefinitions(IDLInterface &iface) {
	// *** FIXME try _is_a query before narrowing
	
  	string ifname = iface.getCPPIdentifier();

	m_header
	<< indent << "inline " << iface.getQualifiedCPP_ptr() << " "
	<< iface.getQualifiedCPPIdentifier(iface.getRootScope()) << "::_duplicate("
	<< iface.getQualifiedCPP_ptr() << " obj) {" << endl;

	if(iface.requiresSmartPtr()) {
		m_header
		<< ++indent << "CORBA::Object_ptr ptr = obj;" << endl;
		m_header
		<< indent << iface.getNSScopedCTypeName()
		<< " cobj = reinterpret_cast<CORBA_Object>(ptr);" << endl
		<< indent << "cobj = ::_orbitcpp::duplicate_guarded(cobj);" << endl
		<< indent << "return "
		<< indent << "reinterpret_cast< " << iface.getQualifiedCPP_ptr()+"&>(cobj);" << endl;
	} else {	
		m_header
		<< ++indent << "return "
		<< iface.getQualifiedCPPCast(IDL_IMPL_NS "::duplicate_guarded(*obj)") << ';' << endl;
	}
	m_header
	<< --indent << '}' << endl
	  
	<< indent << "inline " << iface.getQualifiedCPP_ptr() << " "
	<< iface.getQualifiedCPPIdentifier(iface.getRootScope())
	<< "::_narrow(CORBA::Object_ptr obj) {" << endl;

	// Are we using smart pointers for _ptrs?
	if(iface.requiresSmartPtr()) {
		m_header
		<< ++indent << "return _duplicate(reinterpret_cast< "
		<< iface.getQualifiedCPPStub() << " *>(obj));" << endl;
	} else {
		m_header
		<< ++indent << "return _duplicate(static_cast< "
		<< iface.getQualifiedCPP_ptr() << ">(obj));" << endl;
	}
	m_header
	<< --indent << '}' << endl << endl;
}
