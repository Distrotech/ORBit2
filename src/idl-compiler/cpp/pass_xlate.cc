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
 *  Authors:	Andreas Kloeckner <ak@ixion.net>
 *              Phil Dawes <philipd@users.sourceforge.net>
 *				John K. Luebs <jkluebs@marikarpress.com>
 *
 *  Purpose: idl -> c++ translation pass
 *
 */




#include "error.hh"
#include "pass_xlate.hh"
#include <cstdlib>
#include <strstream>


// IDLPassXlate --------------------------------------------------------------
void 
IDLPassXlate::runPass() {
	m_header
	<< indent << "#ifndef __ORBITCPP_IDL_" << idlUpper(m_state.m_basename) << "_COMMON" << endl
	<< indent << "#define __ORBITCPP_IDL_" << idlUpper(m_state.m_basename) << "_COMMON" << endl
	<< endl << endl
	<< indent << "#include <string.h>" << endl
	<< indent << "#include <orbit/orb-cpp/orbitcpp.hh>" << endl
	//<< indent << "namespace "IDL_IMPL_NS_ID" { namespace "IDL_IMPL_C_NS_ID" {" << endl;
	//indent++;
	//m_header
	<< indent << "#include \"" << m_state.m_basename << ".h\"" << endl;
	//indent--;
	//m_header
	//<< indent << "} }" << endl;

	m_module << mod_indent
	<< "#include \"" << m_state.m_basename << IDL_CPP_STUB_HEADER_EXT << "\"" << endl
	<< endl << endl;

	m_header
	<< endl << endl
	<< indent << "// Type mapping ----------------------------------------" << endl
	<< endl;

	doDefinitionList(m_state.m_rootscope.getNode(),m_state.m_rootscope);
	runJobs();

	m_header << indent << endl << "#endif" << endl;
}




void 
IDLPassXlate::doTypedef(IDL_tree node, IDLScope &scope) {
	string id;

	IDL_tree dcl_list = IDL_TYPE_DCL(node).dcls;
	bool first_dcl = true;

	while (dcl_list) {
		IDLTypedef &td = (IDLTypedef &) *scope.getItem(IDL_LIST(dcl_list).data);
		if( first_dcl ) {
			ORBITCPP_MEMCHECK( new IDLWriteCPPSpecCode(td.getAlias(), m_state, *this) );
			first_dcl = false;
		}
		const IDLType &targetType = td.getAlias();
		targetType.writeTypedef(m_header,indent,m_state,td,scope);
		m_header << indent;
		if( scope.getTopLevelInterface() )
			m_header << "static ";
		m_header
		<< "const CORBA::TypeCode_ptr _tc_" << td.getCPPIdentifier() << " = " 
		<< "(CORBA::TypeCode_ptr)TC_" + td.getQualifiedCIdentifier() + ";" << endl;
		dcl_list = IDL_LIST(dcl_list).next;
		m_header << endl;
	}
}




void 
IDLPassXlate::doStruct(IDL_tree node,IDLScope &scope) {
	IDLStruct &idlStruct = (IDLStruct &) *scope.getItem(node);

	m_header
	<< indent++ << "struct " << idlStruct.getCPPIdentifier() << " {" << endl;

	IDLStruct::const_iterator first = idlStruct.begin(),last = idlStruct.end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		string typespec,dcl;
		member.getType()->getCPPMemberDeclarator(member.getCPPIdentifier(),typespec,dcl);
		m_header << indent << typespec << ' ' << dcl << ';' << endl;
		ORBITCPP_MEMCHECK( new IDLWriteCPPSpecCode(*member.getType(), m_state, *this) );
	}
	
	if(idlStruct.isVariableLength()) {
		m_header
		<< endl
		<< indent++ << "void* operator new(size_t) {" << endl
		<< indent << "return "
		<< IDL_IMPL_C_NS_NOTUSED
		<< idlStruct.getQualifiedCIdentifier() << "__alloc();" << endl;
		m_header
		<< --indent << "};" << endl << endl;
		m_header
		<< indent++ << "void operator delete(void* c_struct) {" << endl
		<< indent <<  "::CORBA_free(c_struct);" << endl;
		m_header
		<< --indent << "};" << endl << endl;
			
	}
	
	
	m_header
	<< --indent << "};" << endl << endl; 

	if(!idlStruct.isVariableLength()){
		m_header
		<< indent << "typedef " << idlStruct.getCPPIdentifier()
		<< "& " << idlStruct.getCPP_out() << ";" << endl << endl;
	} else {
		m_header
		<< indent << "typedef "IDL_IMPL_NS "::Data_var<"
		<< idlStruct.getCPPIdentifier() << "> " 
		<< idlStruct.getCPP_var() <<";" << endl;
		m_header
		<< indent << "typedef "IDL_IMPL_NS "::Data_out<"
		<< idlStruct.getCPPIdentifier() << "> " 
		<< idlStruct.getCPP_out() <<";" << endl << endl;
	}
	
	m_header << indent;
	if( scope.getTopLevelInterface() )
		m_header << "static ";
	m_header
	<< "const CORBA::TypeCode_ptr _tc_" << idlStruct.getCPPIdentifier() << " = " 
	<< "(CORBA::TypeCode_ptr)TC_" + idlStruct.getQualifiedCIdentifier() + ";" << endl;
	ORBITCPP_MEMCHECK( new IDLWriteStructAnyFuncs(idlStruct, m_state, *this) );
}



void 
IDLPassXlate::doUnion(IDL_tree node,IDLScope &scope) {
	IDLUnion &idlUnion = (IDLUnion &) *scope.getItem(node);

	m_header
	<< indent << "class " << idlUnion.getCPPIdentifier() << "{" << endl
	<< indent << "private:" << endl;

	m_header
	<< ++indent << idlUnion.getNSScopedCTypeName() << " m_target;" << endl << endl;

	m_header	
	<< indent << "void _clear_member() {" << endl;
	m_header	
	  << ++indent << idlUnion.getNSScopedCTypeName() << "__freekids(&m_target, NULL);  // actually frees the children, but not the union itself" << endl;
	m_header	
	<< --indent << "}" << endl << endl;

	m_header
	<< --indent << "public:" << endl;
	indent++;

	m_header	
	<< indent << idlUnion.getCPPIdentifier() << "() {" << endl;
	m_header
	<< ++indent << "// This is a hack to ensure that freeing variable length members of a union" << endl
	<< indent <<   "// allocated on the stack doesn't result in a crash" << endl
	<< indent << "memset(this, \'\\0\', sizeof(" << idlUnion.getNSScopedCTypeName() << "));" << endl;
	m_header	
	<< --indent << "}" << endl << endl;

	m_header	
	<< indent << "~" << idlUnion.getCPPIdentifier() << "() {" << endl;
	m_header
	<< ++indent << "_clear_member();" << endl
	<< indent << "// 0 the buffer so that CORBA_free (called by operator delete) doesn't free this again" << endl
	<< indent << "memset(this, \'\\0\', sizeof(" << idlUnion.getNSScopedCTypeName() << "));" << endl;
	m_header
	<< --indent << "}" << endl << endl;


	m_header	
	  << indent++ << idlUnion.getCPPIdentifier()
	  << "("<<idlUnion.getCPPIdentifier() <<" const &u) {" << endl;
	idlUnion.writeCDeepCopyCode(m_header,indent,"m_target","u.m_target");
	m_header	
	<< --indent << "}" << endl << endl;


	m_header	
	<< indent++ << idlUnion.getCPPIdentifier() << " &operator=("<<idlUnion.getCPPIdentifier() <<" const &u) {" << endl;
	idlUnion.writeCDeepCopyCode(m_header,indent,"m_target","u.m_target");
	m_header	
	<< indent << "return *this;" << endl;
	m_header	
	<< --indent << "}" << endl << endl;

	
	const IDLType &desc = idlUnion.getDiscriminatorType();
	string dcl, typespec;
	
	desc.getCPPStubDeclarator(IDL_PARAM_IN,"desc",typespec,dcl);
	m_header
	<< indent << "void _d(" << typespec << " " << dcl	<< "){" << endl;
	m_header	
	<< ++indent << "m_target._d = "
	<< desc.getCPPStubParameterTerm(IDL_PARAM_IN,"desc") << ";" << endl;
	m_header	
	<< --indent << "}" << endl;
	
	desc.getCPPStubReturnDeclarator("",typespec,dcl);
	m_header
	<< indent << typespec << dcl << " _d() const {" << endl;
	m_header	
	<< ++indent << "return " << desc.getCPPSkelParameterTerm(IDL_PARAM_IN,"m_target._d")
	<< ";" << endl;
	m_header	
	<< --indent << "}" << endl;
	
	IDLUnion::const_iterator first = idlUnion.begin(),last = idlUnion.end();

	while (first != last) {
		IDLCaseStmt &casestmt = (IDLCaseStmt &) **first++;
		IDLMember const &member = casestmt.getMember();
		const IDLType &type = *member.getType();

		string descVal;
		if(casestmt.isDefault() == true){
			descVal = idlUnion.getDefaultDiscriminatorValue();
		} else {
			descVal = *(casestmt.labelsBegin());
		}
		
		type.writeUnionAccessors(m_header,indent,member.getCPPIdentifier(),descVal);
		type.writeUnionModifiers(m_header,indent,member.getCPPIdentifier(),descVal);
		type.writeUnionReferents(m_header,indent,member.getCPPIdentifier(),descVal);
		ORBITCPP_MEMCHECK( new IDLWriteCPPSpecCode(type, m_state, *this) );
	}

	if(idlUnion.hasExplicitDefault() == false){
		m_header
		<< indent << "void _default() {" << endl;
		m_header
		<< ++indent << "_clear_member();" << endl
		<< indent << "_d("<< idlUnion.getDefaultDiscriminatorValue() << ");" << endl;
		m_header	
		<< --indent << "}" << endl;
	}
	
	if(idlUnion.isVariableLength()) {
		m_header
		<< endl
		<< indent++ << "void* operator new(size_t) {" << endl
		<< indent << "return "
		<< IDL_IMPL_C_NS_NOTUSED
		<< idlUnion.getQualifiedCIdentifier() << "__alloc();" << endl;
		m_header
		<< --indent << "};" << endl << endl;
		m_header
		<< indent++ << "void operator delete(void* c_union) {" << endl
		<< indent <<  "::CORBA_free(c_union);" << endl;
		m_header
		  << --indent << "};" << endl << endl;
	}
	
	m_header
	  << --indent << "};" << endl << endl;

	if(!idlUnion.isVariableLength()){
		m_header
		<< indent << "typedef " << idlUnion.getCPPIdentifier()
		<< "& " << idlUnion.getCPP_out() << ";" << endl << endl;
	} else {
		m_header
		<< indent << "typedef "IDL_IMPL_NS "::Data_var<"
		<< idlUnion.getCPPIdentifier() << "> " 
		<< idlUnion.getCPP_var() <<";" << endl;
		m_header
		<< indent << "typedef "IDL_IMPL_NS "::Data_out<"
		<< idlUnion.getCPPIdentifier() << "> " 
		<< idlUnion.getCPP_out() <<";" << endl << endl;
	}
	
	m_header
	<< indent << "const CORBA::TypeCode_ptr _tc_" << idlUnion.getCPPIdentifier() << " = " 
	<< "(CORBA::TypeCode_ptr)TC_" + idlUnion.getQualifiedCIdentifier() + ";" << endl;

	ORBITCPP_MEMCHECK( new IDLWriteUnionAnyFuncs(idlUnion, m_state, *this) );
}




void 
IDLPassXlate::doEnum(IDL_tree node,IDLScope &scope) {
	IDLEnum &idlEnum = (IDLEnum &) *scope.getItem(node);

	m_header
	<< indent++ << "enum " << idlEnum.getCPPIdentifier() << "{" << endl;

	IDLEnum::const_iterator first = idlEnum.begin(),last = idlEnum.end();
	for(IDLEnum::const_iterator it=first; it != last;it++)
		m_header << indent << (*it)->getCPPIdentifier() << "," << endl;

	m_header
	<< --indent << "};" << endl << endl; 

	
	m_header
	<< indent << "typedef " << idlEnum.getCPPIdentifier() << "& "
	<< idlEnum.getCPPIdentifier() << "_out;" << endl << endl;
	
	m_header << indent;
	if( scope.getTopLevelInterface() )
		m_header << "static ";
	m_header
	<< "const CORBA::TypeCode_ptr _tc_" << idlEnum.getCPPIdentifier() << " = " 
	<< "(CORBA::TypeCode_ptr)TC_" + idlEnum.getQualifiedCIdentifier() + ";" << endl;
	ORBITCPP_MEMCHECK( new IDLWriteEnumAnyFuncs(idlEnum, m_state, *this) );
}




void 
IDLPassXlate::doNative(IDL_tree node,IDLScope &scope) {
	ORBITCPP_NYI("native")
}




void 
IDLPassXlate::doConstant(IDL_tree node,IDLScope &scope) {
	IDLConstant &cns = (IDLConstant &) *scope.getItem(node);
	string typespec,dcl;
	cns.getType()->getCPPConstantDeclarator(cns.getCPPIdentifier(),typespec,dcl);
	
	// undef the C constant #define
	m_header
	<< indent << "#undef " << cns.getCIdentifier() << endl;
	
	if (cns.getTopLevelInterface()) 
		m_header
		<< indent << "static ";
	else
		m_header
		<< indent;
	m_header
	<< "const " << typespec << ' ' << dcl << " = "
	<< cns.getValue() << ';' << endl;
}




void 
IDLPassXlate::doAttribute(IDL_tree node,IDLScope &scope) {
}




void 
IDLPassXlate::doOperation(IDL_tree node,IDLScope &scope) {
}




void 
IDLPassXlate::doException(IDL_tree node,IDLScope &scope) {
	IDLException &except = (IDLException &) *scope.getItem(node);

	// spec code must be generated before the exception because the exception "uses" its
	// members
	IDLException::iterator first = except.begin(),last = except.end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		member.getType()->writeCPPSpecCode(m_header,indent,m_state);
	}

	m_header
	<< indent << "class " << except.getCPPIdentifier()
	<< " : public "IDL_CORBA_NS "::UserException {" << endl;
	indent++;

	m_header
	<< indent << "public:" << endl;
	indent++;
	m_header
	<< indent << "// members" << endl;

	first = except.begin();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		string typespec,dcl;
		member.getType()->getCPPMemberDeclarator(member.getCPPIdentifier(),typespec,dcl);
		m_header << indent << typespec << ' ' << dcl << ';' << endl;
	}

	m_header
	<< endl << indent << "// methods" << endl
	<< indent << "// copy ctor, dtor and assignment op will be auto-generated" << endl
	<< indent << except.getCPPIdentifier() << "() { }" << endl;

	first = except.begin();
	last = except.end();

	if (first != last) { // no members => no member init constructor
		m_header
		<< indent << except.getCPPIdentifier() << '(';
		m_module
		<< mod_indent << except.getQualifiedCPPIdentifier(&m_state.m_rootscope)
		<< "::" << except.getCPPIdentifier() << '(';
		while (first != last) {
			IDLMember &member = (IDLMember &) **first++;
			string typespec,dcl;
			member.getType()->getCPPStructCtorDeclarator(member.getCPPIdentifier(),typespec,dcl);
			m_header << typespec << ' ' << dcl;
			m_module << typespec << ' ' << dcl;
			if (first != last) {
				m_header << ',';
				m_module << ',';
			}
		}
		m_header << ");" << endl;
		m_module << ") {" << endl;
		mod_indent++;

		first = except.begin();
		last = except.end();

		while (first != last) {
			IDLMember &member = (IDLMember &) **first++;
			member.getType()->writeCPPStructCtor(m_module,mod_indent,member.getCPPIdentifier());
		}
		m_module << mod_indent << '}' << endl;
		mod_indent--;
	}

	m_header
	<< indent << "void _raise() {" << endl;
	indent++;
	m_header
	<< indent << "throw *this;" << endl
	<< indent << '}' << endl;
	indent--;

	m_header
	<< indent << "void _orbitcpp_set("  "::CORBA_Environment *ev) {" << endl;
	indent++;
	m_header
	<< indent <<  "::CORBA_exception_set(ev,"
	<<  "::CORBA_USER_EXCEPTION,\""
	<< except.getRepositoryId() << "\",_orbitcpp_pack());" << endl
	<< indent << '}' << endl;
	indent--;

	m_header
	<< indent << "static " << except.getCPPIdentifier() << " *_narrow("
	<< IDL_CORBA_NS "::Exception *ex) {" << endl;
	indent++;
	m_header
	<< indent << "return dynamic_cast<" << except.getCPPIdentifier() <<
	"*>(ex);" << endl 
	<< indent << '}' << endl;
	indent--;

	except.writeCPackingCode(m_header,indent,m_module,mod_indent);

	indent--;
	m_header
	<< indent << "};" << endl << endl;
	indent--;
	
	m_header << indent;
	if( scope.getTopLevelInterface() )
		m_header << "static ";
	m_header
	<< "const CORBA::TypeCode_ptr _tc_" << except.getCPPIdentifier() << " = " 
	<< "(CORBA::TypeCode_ptr)TC_" + except.getQualifiedCIdentifier() + ";" << endl;

	ORBITCPP_MEMCHECK( new IDLWriteExceptionAnyFuncs(except, m_state, *this) );
}




void 
IDLPassXlate::doInterface(IDL_tree node,IDLScope &scope) {
	IDLInterface &iface = (IDLInterface &) *scope.getItem(node);

	string ns_iface_begin,ns_iface_end;
	iface.getParentScope()->getCPPNamespaceDecl(ns_iface_begin,ns_iface_end);

	bool non_empty_ns = ns_iface_end.size() || ns_iface_begin.size();

	string ifname = iface.getCPPIdentifier();
	string _ptrname = iface.getCPP_ptr();

	m_header 
	<< indent << "class " << ifname << ';' << endl;

	if(non_empty_ns){
		m_header
		<< --indent << ns_iface_end;
	}

	m_header << indent++ << "namespace " IDL_IMPL_NS_ID " { "
	<< "namespace " IDL_IMPL_STUB_NS_ID " { "
			 << ns_iface_begin << endl;
	m_header
	<< indent << "class " << ifname << ";" << endl;
	m_header
	<< --indent << ns_iface_end << "}}";

	if(non_empty_ns){
		m_header
		<< ns_iface_begin;
		indent++;
	}
	
	m_header << endl;

	// check the MI situation and write a smartptr if required.
	if (iface.requiresSmartPtr())
	{
		doInterfacePtrClass(iface);
		m_header
		<< indent << "typedef "IDL_IMPL_NS "::ObjectSmartPtr_var<" << ifname << ',' << _ptrname << "> "
		<< ifname << "_var;" << endl;
	}
	else
	{
		m_header
		<< indent << "typedef " << iface.getQualifiedCPPStub() << " *" << _ptrname << ';' << endl;
		m_header
		<< indent << "typedef "IDL_IMPL_NS "::ObjectPtr_var<" << ifname << ',' << _ptrname << "> "
		<< ifname << "_var;" << endl;
	}
	
	m_header
	<< indent << "typedef " << iface.getCPP_var() << " " << iface.getCPP_mgr() << ";" << endl
	<< indent << "typedef "IDL_IMPL_NS "::ObjectPtr_out<" << ifname << ',' << _ptrname << "> "
	<< ifname << "_out;" << endl
	<< indent << "typedef " << _ptrname << ' '<< ifname << "Ref;" << endl;

	if(non_empty_ns){
		m_header
		<< --indent << ns_iface_end << endl;
	}
	
	// get poa namespace info
	string ns_poa_begin,ns_poa_end;
	iface.getCPPpoaNamespaceDecl(ns_poa_begin,ns_poa_end);

	// predeclare POA type (necessary for typedef'ing)	
	if (non_empty_ns) {
		m_header
		<< indent <<  ns_poa_begin << endl;
		indent++;
	}
	m_header
	<< indent << "class " << iface.getCPPpoaIdentifier() << ';' << endl;
	if (non_empty_ns) {
		m_header
		<< --indent << ns_poa_end;
	}
	else m_header << indent;
	
	// generate type container
	if(non_empty_ns){
		m_header
		<< indent++ << ns_iface_begin << endl;
	}
	m_header
	<< indent << "class " << ifname;
	
	IDLInterface::BaseList::const_iterator
	first = iface.m_bases.begin(), last = iface.m_bases.end();
	
	if (first != last) {
		m_header 
		<< " : public " << (*first)->getQualifiedCPPIdentifier();
		first++;
	}
	while (first != last) {
		m_header
		<< ", public " << (*first++)->getQualifiedCPPIdentifier();
	}
	m_header
	<< " {" << endl
	<< indent << "public:" << endl;
	indent++;
	
	Super::doInterface(node,iface);

	doInterfaceStaticMethodDeclarations(iface);

	m_header
	<< --indent << "};" << endl;

	m_header
	<< indent << "const CORBA::TypeCode_ptr _tc_" << iface.getCPPIdentifier() << " = " 
	<< "(CORBA::TypeCode_ptr)TC_" + iface.getQualifiedCIdentifier() + ";" << endl;

	ORBITCPP_MEMCHECK( new IDLWriteIfaceAnyFuncs(iface, m_state, *this) );
}


void
IDLPassXlate::doInterfaceStaticMethodDeclarations(IDLInterface &iface) {
  	string ifname = iface.getCPPIdentifier();

	m_header 
	<< indent << "static " << iface.getQualifiedCPP_ptr() << " _duplicate("
	<< iface.getQualifiedCPP_ptr() << " obj);" << endl

	<< indent << "static " << iface.getQualifiedCPP_ptr()
	<< " _narrow(CORBA::Object_ptr obj);" << endl

	<< indent << "static " << iface.getQualifiedCPP_ptr() << " _nil() {" << endl;
	m_header
	<< ++indent << "return CORBA_OBJECT_NIL;" << endl;
	m_header
	<< --indent << '}' << endl;
}

void
IDLPassXlate::doInterfacePtrClass(IDLInterface &iface) {
	m_header
		<< indent << "class " << iface.getCPP_ptr() << "{" << endl;
	m_header
		<< ++indent << iface.getQualifiedCPPStub() << " *m_target;" << endl
		<< indent << "public:" << endl
		<< indent << iface.getCPP_ptr() << "():m_target(NULL) {}" << endl
		<< indent << iface.getCPP_ptr() << "(" <<iface.getQualifiedCPPStub() <<" *ptr):m_target(ptr){}" << endl
		<< indent << iface.getCPP_ptr() << " &operator =(" <<iface.getQualifiedCPPStub() <<" *ptr){" << endl;
	m_header
		<< ++indent << "m_target = ptr;" << endl
		<< indent << "return *this;" << endl;
	m_header
		<< --indent << "}" << endl;
	m_header
		<< indent << iface.getQualifiedCPPStub() << " *operator ->(){" << endl;
	m_header
		<< ++indent << "return m_target;" << endl;
	m_header
		<< --indent << "}" << endl;
	m_header
		<< indent << "//operator CORBA::Object *() {return reinterpret_cast<CORBA::Object *>(m_target);}" << endl;


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
}



void 
IDLPassXlate::doModule(IDL_tree node,IDLScope &scope) {
	IDLScope *module = (IDLScope *) scope.getItem(node);

	m_header << indent << "namespace " << module->getCPPIdentifier() << " {" << endl;
	indent++;
	Super::doModule(node,*module);
	indent--;
	m_header << indent << "}" << endl;
}




void IDLPassXlate::enumHook(IDL_tree next,IDLScope &scope) {
	if (!scope.getTopLevelInterface())
		runJobs(IDL_EV_TOPLEVEL);
}

// IDLWriteArrayProps -------------------------------------------------------
void IDLWriteArrayProps::run()
{
	string ident = m_dest.getQualifiedCPPIdentifier(m_dest.getRootScope());
	m_header
	<< indent << "inline " << ident << "_slice *"
	<< ident << "Props::alloc() {\n";
	m_header
	<< ++indent << "return " << ident << "_alloc();\n";
	m_header
	<< --indent  << "}\n\n";
	

	m_header
	<< indent << "inline void "
	<< ident << "Props::free("
	<< ident << "_slice * target) {\n";
	m_header
	<< ++indent << ident << "_free(target);\n";
	m_header
	<< --indent  << "}\n\n";

	m_header
	<< indent << "inline void "
	<< ident << "Props::copy("
	<< ident << "_slice * m_dest, "
	<< ident << "_slice const * src) {\n";
	m_header
	<< ++indent << ident << "_copy(m_dest,src);\n";
	m_header
	<< --indent  << "}\n\n";
}

void IDLWriteAnyFuncs::writeAnyFuncs(bool pass_value, string const& cpptype, 
	string const& ctype)
{
	if( pass_value ) {
		writeInsertFunc(m_header, indent, FUNC_VALUE, cpptype, ctype);
		writeExtractFunc(m_header, indent, FUNC_VALUE, cpptype, ctype);
	} else {
		writeInsertFunc(m_header, indent, FUNC_COPY, cpptype, ctype);
		writeInsertFunc(m_header, indent, FUNC_NOCOPY, cpptype, ctype);
		writeExtractFunc(m_header, indent, FUNC_NOCOPY, cpptype, ctype);
	}
}

void IDLWriteAnyFuncs::writeInsertFunc(ostream& ostr, Indent &indent, FuncType func, 
										string ident, string const& ctype)
{
	string any_func, any_arg;
	any_func = "insert_simple";
	any_arg = "&val";
	if( func == FUNC_COPY ) {
		ident += " const &";
	}
	else if( func == FUNC_NOCOPY ) {
		ident += "*";
		any_arg = "val, CORBA_FALSE";
	}
	ostr << indent++
	<< "inline void operator<<=(CORBA::Any& the_any, " << ident
	<< " val) {" << endl;
	ostr << indent << "the_any." << any_func 
	<< "( (CORBA::TypeCode_ptr)TC_"
	<< ctype << ", "
	<< any_arg << ");" ;
	ostr << --indent << endl << "}" << endl << endl;
}

void IDLWriteAnyFuncs::writeExtractFunc(ostream& ostr, Indent &indent, FuncType func, 
										string ident, string const& ctype)
{
	string any_func, any_arg;
	any_arg = "val";
	if( func == FUNC_VALUE ) {
		ident += "&";
		any_func = "extract";
	}
	else {
		ident += " const *&";
		any_func = "extract_ptr";
	}
	ostr << indent
	<< "inline CORBA::Boolean operator>>=(const CORBA::Any& the_any, " << ident
	<< " val) {" << endl;
	ostr << ++indent << "return the_any." << any_func 
	<< "( (CORBA::TypeCode_ptr)TC_"
	<< ctype << ", "
	<< any_arg << ");" ;
	ostr << --indent << endl << "}" << endl << endl;
}

void
IDLWriteExceptionAnyFuncs::run() {
	m_header << indent
	<< "inline void operator <<=(CORBA::Any& the_any, " 
	<< m_element.getQualifiedCPPIdentifier() << " const & val) {" << endl;
	m_header << ++indent
	<< "the_any.insert_simple( (CORBA::TypeCode_ptr)TC_"
	<< m_element.getQualifiedCIdentifier() << ", const_cast< "
	<< m_element.getQualifiedCPPIdentifier() << "&>(val)._orbitcpp_pack(), CORBA_FALSE);" << endl;
	m_header
	  << --indent << endl << "}" << endl;

	m_header << indent
	<< "inline CORBA::Boolean operator>>=(const CORBA::Any& the_any, " 
	<< m_element.getQualifiedCPPIdentifier() << " & val) {" << endl;
	m_header
	<< ++indent << "const " << m_element.getQualifiedCIdentifier() << " *ex;" << endl;
	m_header
	<< indent << "if( the_any.extract_ptr( (CORBA::TypeCode_ptr)TC_"
	<< m_element.getQualifiedCIdentifier() << ", ex)){" << endl;
	m_header
	<< ++indent << "val._orbitcpp_unpack(*ex);" << endl;
	m_header
	<< indent << "return true;" << endl;
	m_header
	<< --indent << "} else {" << endl;	
	m_header
	<< ++indent << "return false;" << endl;
	m_header
	<< --indent << "}" << endl;
	m_header
	<< --indent << "}" << endl;
}


void
IDLWriteArrayAnyFuncs::run()
{
	m_header << indent
	<< "inline void operator <<=(CORBA::Any& the_any, " 
	<< m_dest.getQualifiedCPPIdentifier() << "_forany &_arr) {" << endl;
	m_header << ++indent
	<< "the_any.insert_simple( (CORBA::TypeCode_ptr)TC_"
	<< m_dest.getQualifiedCIdentifier() << ", ("
	<< m_dest.getQualifiedCPPIdentifier() << "_slice*)_arr, "
	<< "!_arr._nocopy());" << --indent << endl << "}" << endl;
	
	m_header << indent
	<< "inline CORBA::Boolean operator >>=(CORBA::Any& the_any, " 
	<< m_dest.getQualifiedCPPIdentifier() << "_forany &_arr) {" << endl;

	m_header
	<< ++indent
	<< m_dest.getQualifiedCPPIdentifier() << "_slice const* tmp;" << endl 
	<< indent << "CORBA::Boolean _retval;" << endl
	<< indent << "_retval = the_any.extract_ptr( (CORBA::TypeCode_ptr)TC_"
	<< m_dest.getQualifiedCIdentifier() << ", tmp);" << endl
	<< indent << "_arr = (" << m_dest.getQualifiedCPPIdentifier() 
    << "_slice*)tmp;" << endl << indent << "return _retval;" << endl;
	m_header
	<< --indent << "}" << endl << endl;
}	
