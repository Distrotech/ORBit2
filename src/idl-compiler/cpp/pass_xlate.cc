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
  << indent << "#ifdef _ORBITCPP_TESTCODE" << endl
	<< indent << "//The headers are in a different place than when they are installed:" << endl
	<< indent << "#include <orb-cpp/orbitcpp.hh>" << endl
  << indent << "#else" << endl
	<< indent << "#include <orbit/orb-cpp/orbitcpp.hh>" << endl
	<< indent << "#endif" << endl
	<< endl
	<< indent << "#include <string.h>" << endl
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


#if 0

void 
IDLPassXlate::doTypedef(IDL_tree  node,
			IDLScope &scope)
{
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
#endif


#if 0
void 
IDLPassXlate::doStruct (IDL_tree  node,
			IDLScope &scope)
{
	IDLStruct &idlStruct = (IDLStruct &) *scope.getItem(node);

	m_header << indent << "class " << idlStruct.getCPPIdentifier() << endl
		 << indent++ << "{" << endl;

	m_header << indent << idlStruct.getCTypeName () << " c_struct;" << endl;
	
	m_header << endl << --indent << "public:" << endl;
	indent++;

	// Method to access the underlying C object
	struct_create_wrapper (idlStruct);
	struct_create_unwrapper (idlStruct);
	
	// Member accessors
	struct_create_accessors (idlStruct);
	struct_create_constructor (idlStruct);
	
	if(idlStruct.isVariableLength()) {
		m_header << endl
			 << indent << "void* operator new(size_t)" << endl
			 << indent++ << "{" << endl
			 << indent << "return "
			 << IDL_IMPL_C_NS_NOTUSED
			 << idlStruct.getQualifiedCIdentifier() << "__alloc();" << endl;
		m_header << --indent << "};" << endl << endl;
		m_header << indent << "void operator delete(void* c_struct)" << endl
			 << indent++ << "{" << endl
			 << indent <<  "::CORBA_free(c_struct);" << endl;
		m_header << --indent << "};" << endl << endl;
			
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
	<< indent << "class " << idlUnion.getCPPIdentifier() << endl
	<< indent << "{" << endl
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
	<< indent << "void _d(" << typespec << " " << dcl	<< ")" << endl
	<< indent << "{" << endl;
	m_header	
	<< ++indent << "m_target._d = "
	<< desc.getCPPStubParameterTerm(IDL_PARAM_IN,"desc") << ";" << endl;
	m_header	
	<< --indent << "}" << endl;
	
	desc.getCPPStubReturnDeclarator("",typespec,dcl);
	m_header
	<< indent << typespec << dcl << " _d() const" << endl
	<< indent << "{" << endl;
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
		<< indent << "void _default()" << endl
		<< indent << "{" << endl;

		m_header
		<< ++indent << "_clear_member();" << endl
		<< indent << "_d("<< idlUnion.getDefaultDiscriminatorValue() << ");" << endl;
		m_header	
		<< --indent << "}" << endl;
	}
	
	if(idlUnion.isVariableLength()) {
		m_header
		<< endl
		<< indent << "void* operator new(size_t)" << endl
		<< indent++ << "{" << endl
		<< indent << "return "
		<< IDL_IMPL_C_NS_NOTUSED
		<< idlUnion.getQualifiedCIdentifier() << "__alloc();" << endl;
		m_header
		<< --indent << "};" << endl << endl;
		m_header
		<< indent << "void operator delete(void* c_union)" << endl
		<< indent++ << "{" << endl
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
#endif



void 
IDLPassXlate::doEnum (IDL_tree  node,
		      IDLScope &scope)
{
	IDLEnum &idlEnum = (IDLEnum &) *scope.getItem(node);

	string ns_iface_begin, ns_iface_end;
	idlEnum.getParentScope()->getCPPNamespaceDecl(ns_iface_begin, ns_iface_end);

	bool non_empty_ns = ns_iface_end.size () || ns_iface_begin.size ();

	if (non_empty_ns)
		m_header << indent << ns_iface_begin;
	
	m_header << indent << "enum " << idlEnum.get_cpp_identifier () << endl
		 << indent++ << "{" << endl;

	for (IDLEnum::const_iterator i = idlEnum.begin ();
	     i != idlEnum.end (); i++)
	{
		m_header << indent << (*i)->get_cpp_identifier ()
			 << ", " << endl;
	}
	
	m_header << --indent << "};" << endl << endl; 

	
	m_header << indent << "typedef " << idlEnum.get_cpp_identifier () << "& "
		 << idlEnum.get_cpp_identifier () << "_out;" << endl << endl;
	
	m_header << indent;
	if (scope.getTopLevelInterface ())
		m_header << "static ";

	m_header << "const CORBA::TypeCode_ptr _tc_" << idlEnum.get_cpp_identifier () << " = " 
		 << "(CORBA::TypeCode_ptr)TC_" << idlEnum.get_c_typename ()
		 << ';' << endl;

	if (non_empty_ns)
		m_header << indent << ns_iface_end;

#if 0 //!!!
	ORBITCPP_MEMCHECK(new IDLWriteEnumAnyFuncs (idlEnum, m_state, *this));
#endif
}


void 
IDLPassXlate::doNative (IDL_tree  node,
			IDLScope &scope)
{
	ORBITCPP_NYI("native")
}




void 
IDLPassXlate::doConstant (IDL_tree  node,
			  IDLScope &scope)
{
	IDLConstant &cns = (IDLConstant &) *scope.getItem(node);

	// undef the C constant #define
	m_header << "#undef " << cns.get_c_identifier () << endl;
	
	m_header << indent;
	if (cns.getTopLevelInterface ())
		m_header  << "static ";

	cns.getType ()->const_decl_write (m_header, indent,
					  cns.get_cpp_identifier (),
					  cns.getValue ());
}



void 
IDLPassXlate::doAttribute(IDL_tree node,IDLScope &scope) {
}



void 
IDLPassXlate::doOperation(IDL_tree node,IDLScope &scope) {
}




void 
IDLPassXlate::doException(IDL_tree  node,
			  IDLScope &scope)
{
	IDLException &except = (IDLException &) *scope.getItem(node);

	string ns_iface_begin, ns_iface_end;
	except.getParentScope()->getCPPNamespaceDecl(ns_iface_begin, ns_iface_end);

	bool non_empty_ns = ns_iface_end.size () || ns_iface_begin.size ();

	if (non_empty_ns)
		m_header << indent << ns_iface_begin;
	
#if 0 //!!!
	// spec code must be generated before the exception because the exception "uses" its
	// members
	IDLException::iterator first = except.begin(),last = except.end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		member.getType()->writeCPPSpecCode(m_header,indent,m_state);
	}
#endif

	m_header << indent << "class " << except.get_cpp_identifier ()
		 << " : public " IDL_CORBA_NS "::UserException" << endl
		 << indent++ << "{" << endl;

	m_header << --indent << "public:" << endl;
	indent++;

	m_header << indent << "// members" << endl;

	for (IDLException::iterator i = except.begin (); i != except.end (); i++)
	{
		IDLMember &member = (IDLMember &) **i;

		m_header << indent
			 << member.getType ()->get_cpp_member_typename ()
			 << " " << member.get_cpp_identifier ()
			 << ';' << endl;
	}

	m_header << endl << indent << "// methods" << endl
		 << indent << "// copy ctor, dtor and assignment op will be auto-generated" << endl
		 << indent << except.get_cpp_identifier () << "() { }" << endl;

	if (except.size ())
	{
		// Create member init constructor
		
		string constructor_args;
		for (IDLException::const_iterator i = except.begin ();
		     i != except.end (); i++)
		{
			IDLMember &member = (IDLMember &) **i;

			constructor_args += member.getType ()->member_decl_arg_get ();
			constructor_args += " _par_";
			constructor_args += member.get_cpp_identifier ();

			if (i != --except.end ())
				constructor_args += ", ";
		}

		m_header << indent << except.get_cpp_identifier ()
			 << " (" << constructor_args << ")"
			 << ';' << endl << endl;
		
		m_module << mod_indent << except.get_cpp_typename ()
			 << "::" << except.get_cpp_identifier ()
			 << " (" << constructor_args << ")" << endl
			 << mod_indent++ << '{' << endl;

		for (IDLException::const_iterator i = except.begin ();
		     i != except.end (); i++)
		{
			IDLMember &member = (IDLMember &) **i;

			member.getType ()->member_impl_arg_copy (m_module, mod_indent,
								 member.get_cpp_identifier ());
		}
			
		m_module << --mod_indent << '}' << endl << endl;
	}

	m_header << indent << "void _raise ()" << endl
		 << indent++ << "{" << endl;
	m_header << indent << "throw *this;" << endl;
	m_header << --indent << '}' << endl << endl;

	m_header << indent << "void _orbitcpp_set (::CORBA_Environment *ev)" << endl
		 << indent++ << "{" << endl;
	if (except.size ())
	{
		m_header << indent <<  "::CORBA_exception_set (ev, ::CORBA_USER_EXCEPTION, "
			 << '"' << except.getRepositoryId() << '"'
			 << ", _orbitcpp_pack ())"
			 << ';' << endl;
	} else {
		m_header << indent <<  "::CORBA_exception_set (ev, ::CORBA_USER_EXCEPTION, "
			 << '"' << except.getRepositoryId() << '"'
			 << ", 0)"
			 << ';' << endl;
	}
	m_header << --indent << '}' << endl << endl;

	m_header << indent << "static " << except.get_cpp_identifier () << " *_narrow "
		 << "(" << IDL_CORBA_NS "::Exception *ex)" << endl
		 << indent++ << "{" << endl;
	m_header << indent << "return dynamic_cast"
		 << "<" << except.get_cpp_identifier () << "*> (ex)"
		 << ';' << endl;
	m_header << --indent << '}' << endl;

	except.write_packing_decl (m_header, indent);
	except.write_packing_impl (m_module, mod_indent);

	m_header << --indent << "};" << endl << endl;
	
	m_header << indent;
	if (scope.getTopLevelInterface ())
		m_header << "static ";

	m_header << "const CORBA::TypeCode_ptr _tc_" << except.get_cpp_identifier () << " = " 
		 << "(CORBA::TypeCode_ptr)TC_" + except.get_c_typename ()
		 << ';' << endl;

	if (non_empty_ns)
		m_header << ns_iface_end;

#if 0 //!!!
	ORBITCPP_MEMCHECK( new IDLWriteExceptionAnyFuncs(except, m_state, *this) );
#endif
}




void 
IDLPassXlate::doInterface (IDL_tree  node,
			   IDLScope &scope)
{
	IDLInterface &iface = (IDLInterface &) *scope.getItem(node);

	string ns_iface_begin, ns_iface_end;
	iface.getParentScope()->getCPPNamespaceDecl(ns_iface_begin, ns_iface_end);

	bool non_empty_ns = ns_iface_end.size () || ns_iface_begin.size ();

	string ifname = iface.get_cpp_identifier();
	string _ptrname = iface.get_cpp_identifier_ptr();

	if (non_empty_ns)
		m_header << indent++ << ns_iface_begin;
	
	m_header << indent << "class " << ifname << ';' << endl;
	
	if (non_empty_ns)
		m_header << --indent << ns_iface_end;

	m_header << indent << "namespace " IDL_IMPL_NS_ID << endl
		 << indent << "{" << endl
		 << indent << "namespace " IDL_IMPL_STUB_NS_ID << endl
		 << indent << "{" << endl << endl
		 << indent << ns_iface_begin << endl;
	
	m_header << indent << "class " << ifname << ";" << endl << endl;

	m_header << indent << ns_iface_end << endl
		 << indent << "}} //namespaces" << endl << endl;


	
	if (non_empty_ns)
		m_header << ns_iface_begin;
	
	iface.common_write_typedefs (m_header, indent);
	m_header << endl;
	
	if (non_empty_ns)
		m_header << ns_iface_end << endl;

	
	// get poa namespace info
	string ns_poa_begin, ns_poa_end;
	iface.get_cpp_poa_namespace (ns_poa_begin, ns_poa_end);

	// predeclare POA type (necessary for typedef'ing)	
	if (non_empty_ns)
		m_header << ns_poa_begin << endl << endl;
	
	m_header << indent << "class " << iface.get_cpp_poa_identifier () << ';' << endl;
	
	if (non_empty_ns)
		m_header << ns_poa_end;
	
	// generate type container
	if (non_empty_ns)
		m_header << ns_iface_begin << endl;

	m_header << indent << "class " << iface.get_cpp_identifier ();

	if (iface.m_bases.size ())
	{
		m_header << " : ";
		for (IDLInterface::BaseList::const_iterator i = iface.m_bases.begin ();
		     i != iface.m_bases.end (); i++)
		{
			m_header << "public " << (*i)->get_cpp_typename ();
			if (i != --iface.m_bases.end ())
				m_header << ", ";
		}
	}
	
	m_header << endl << indent++ <<" {" << endl;
	
	m_header << --indent << "public:" << endl;
	indent++;
	Super::doInterface(node, iface);
	doInterfaceStaticMethodDeclarations (iface);

	m_header << --indent << "};" << endl;

	m_header << indent << "const CORBA::TypeCode_ptr _tc_"
		 << iface.get_cpp_identifier () << " = " 
		 << "(CORBA::TypeCode_ptr)TC_" + iface.get_c_typename () + ";" << endl;

#if 0 //!!!
	ORBITCPP_MEMCHECK( new IDLWriteIfaceAnyFuncs(iface, m_state, *this) );
#endif

	if (non_empty_ns)
		m_header << ns_iface_end << endl;
	
	// _duplicate() and _narrow implementations:
	// write the static method definitions
	doInterfaceStaticMethodDefinitions(iface);
}


void
IDLPassXlate::doInterfaceStaticMethodDeclarations (IDLInterface &iface)
{
	string ptr_name = iface.get_cpp_typename_ptr ();

	m_header << indent << "static " << ptr_name
		 << " _duplicate (" << ptr_name << " obj);" << endl;

	m_header << indent << "static " << ptr_name
		 << " _narrow (CORBA::Object_ptr obj);" << endl;

	m_header << indent << "static " << ptr_name << " _nil()" << endl
		 << indent++ << '{' << endl
		 << indent << "return CORBA_OBJECT_NIL;" << endl
		 << --indent << '}' << endl;
}

void IDLPassXlate::doInterfaceStaticMethodDefinitions (IDLInterface &iface)
{
	// *** FIXME try _is_a query before narrowing
	
	string if_name = iface.get_cpp_typename ();
	string ptr_name = iface.get_cpp_typename_ptr ();

	// _duplicate()
	m_module << mod_indent << ptr_name << " "
		 << iface.get_cpp_method_prefix () << "::_duplicate(" << ptr_name << " obj)" << endl
		 << mod_indent++ << "{" << endl;
	
 	m_module << mod_indent << "CORBA::Object_ptr ptr = obj;" << endl;

 	m_module << mod_indent << iface.get_c_typename ()
		 << " cobj = ptr->_orbitcpp_get_c_object ();" << endl;
	m_module << mod_indent << "cobj = ::_orbitcpp::duplicate_guarded (cobj);" << endl;
	m_module << mod_indent << "return "
		 << iface.get_cpp_stub_typename () << "::_orbitcpp_wrap (cobj);" << endl;
	
	m_module << --mod_indent << '}' << endl << endl;

	// _narrow()
	m_module << mod_indent << ptr_name << " "
		 << iface.get_cpp_method_prefix () << "::_narrow (CORBA::Object_ptr obj)" << endl
		 << mod_indent++ << '{' << endl;

	m_module << mod_indent << "return _duplicate ("
		 << "reinterpret_cast < " << iface.get_cpp_stub_typename ()
		 << "* > (obj));" << endl;

	m_module << --mod_indent << '}' << endl << endl;
}


void 
IDLPassXlate::doModule (IDL_tree  node,
			IDLScope &scope)
{
	IDLScope *module = (IDLScope *) scope.getItem(node);

#if 0
	string id = module->get_cpp_identifier ();
	m_header << indent << "namespace " << id << endl
		 << indent++ << '{' << endl;
#endif

	Super::doModule(node, *module);

#if 0
	m_header << --indent << '}'
		 << "//namespace " << id << endl << endl;
#endif
}

#if 0 //!!!
void IDLPassXlate::enumHook(IDL_tree next,IDLScope &scope) {
	if (!scope.getTopLevelInterface())
		runJobs(IDL_EV_TOPLEVEL);
}


// IDLWriteArrayProps -------------------------------------------------------
void IDLWriteArrayProps::run()
{
	string ident = m_dest.getQualifiedCPPIdentifier(m_dest.getRootScope());
	m_header
	<< indent << "inline " << ident << "_slice *" << ident << "Props::alloc()" << endl
	<< indent << "{" << endl;

	m_header
	<< ++indent << "return " << ident << "_alloc();\n";
	m_header
	<< --indent  << "}" << endl << endl;
	

	m_header
	<< indent << "inline void " << ident << "Props::free(" << ident << "_slice * target)" << endl
	<< indent << "{" << endl;

	m_header
	<< ++indent << ident << "_free(target);\n";
	m_header
	<< --indent  << "}\n\n";

	m_header
	<< indent << "inline void " << ident << "Props::copy(" << ident << "_slice * m_dest, " << ident << "_slice const * src)" << endl
	<< indent << "{" << endl;

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

	ostr
  << indent << "inline void operator<<=(CORBA::Any& the_any, " << ident << " val)" << endl
	<< indent++ << "{" << endl;

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
	<< "inline CORBA::Boolean operator>>=(const CORBA::Any& the_any, " << ident << " val)" << endl
	<< indent << "{" << endl;

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
	<< m_element.getQualifiedCPPIdentifier() << " const & val)" << endl
	<< indent << "{" << endl;

	m_header << ++indent
	<< "the_any.insert_simple( (CORBA::TypeCode_ptr)TC_"
	<< m_element.getQualifiedCIdentifier() << ", const_cast< "
	<< m_element.getQualifiedCPPIdentifier() << "&>(val)._orbitcpp_pack(), CORBA_FALSE);" << endl;
	m_header
	  << --indent << endl << "}" << endl;

	m_header << indent
	<< "inline CORBA::Boolean operator>>=(const CORBA::Any& the_any, " 
	<< m_element.getQualifiedCPPIdentifier() << " & val)" << endl
	<< indent << "{" << endl;
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
	<< --indent << "}" << endl << endl;
}


void
IDLWriteArrayAnyFuncs::run()
{
	m_header << indent
	<< "inline void operator <<=(CORBA::Any& the_any, " 
	<< m_dest.getQualifiedCPPIdentifier() << "_forany &_arr)" << endl
	<< indent << "{" << endl;

	m_header << ++indent
	<< "the_any.insert_simple( (CORBA::TypeCode_ptr)TC_"
	<< m_dest.getQualifiedCIdentifier() << ", ("
	<< m_dest.getQualifiedCPPIdentifier() << "_slice*)_arr, "
	<< "!_arr._nocopy());" << --indent << endl << "}" << endl;
	
	m_header << indent
	<< "inline CORBA::Boolean operator >>=(CORBA::Any& the_any, " 
	<< m_dest.getQualifiedCPPIdentifier() << "_forany &_arr)" << endl
	<< indent << "{" << endl;

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
#endif
