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

#include "types/IDLTypedef.hh"


// IDLPassXlate --------------------------------------------------------------
void 
IDLPassXlate::runPass() {
	m_header << indent << "#ifndef __ORBITCPP_IDL_" << idlUpper(m_state.m_basename) << "_COMMON" << endl
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

	//m_header << -- indent << "} }" << endl;
	
	m_module << mod_indent
		 << "#include \"" << m_state.m_basename << IDL_CPP_STUB_HEADER_EXT << "\"" << endl
		 << endl << endl;
	
	m_header << endl << endl
		 << indent << "// Type mapping ----------------------------------------" << endl
		 << endl;

	doDefinitionList(m_state.m_rootscope.getNode(),m_state.m_rootscope);
	runJobs();
	
	m_header << indent << endl << "#endif" << endl;
}


void 
IDLPassXlate::doTypedef (IDL_tree  node,
			 IDLScope &scope)
{
	string id;

	bool first_dcl = true;
	
	for (IDL_tree dcl_list = IDL_TYPE_DCL(node).dcls;
	     dcl_list; dcl_list = IDL_LIST(dcl_list).next)
	{
		IDLTypedef &td = (IDLTypedef &) *scope.getItem(IDL_LIST(dcl_list).data);
		
#if 0 //!!!
		if (first_dcl)
		{
			ORBITCPP_MEMCHECK( new IDLWriteCPPSpecCode(td.getAlias(), m_state, *this) );
			first_dcl = false;
		}
#endif
		const IDLType &targetType = td.getAlias();
		targetType.typedef_decl_write (m_header, indent, m_state, td);

		m_header << indent;
		if (scope.getTopLevelInterface ())
			m_header << "static ";
		
		m_header << "const CORBA::TypeCode_ptr "
			 << "_tc_" << td.get_cpp_identifier () << " = " 
			 << "(CORBA::TypeCode_ptr)TC_" << td.get_c_typename ()
			 << ';' << endl << endl;

		// Create traits classes
		ORBITCPP_MEMCHECK(new IDLWriteCPPTraits (td, m_state, *this));
	}
}

void 
IDLPassXlate::doStruct (IDL_tree  node,
			IDLScope &scope)
{
	IDLStruct &strct = (IDLStruct &) *scope.getItem(node);

	m_header << indent << "struct " << strct.get_cpp_identifier () << endl
		 << indent++ << "{" << endl;

	// Create members
	struct_create_members (strct);

	// Create conversion methods
	struct_create_converters (strct);
	
	// End struct definition
	m_header << --indent << "};" << endl;
	
	// Create _out and _var typedef
	struct_create_typedefs (strct);

	// Create typecode and Any stuff
	struct_create_any (strct);

	// Create traits classes
	ORBITCPP_MEMCHECK(new IDLWriteCPPTraits (strct, m_state, *this));

	m_header << endl;
}

void IDLPassXlate::struct_create_members (const IDLStruct &strct)
{
	for (IDLStruct::const_iterator i = strct.begin (); i != strct.end (); i++)
	{
		IDLMember &member = (IDLMember &) **i;
		
		m_header << indent << member.getType ()->get_cpp_member_typename ()
			 << " " << member.get_cpp_identifier ()
			 << ";" << endl;
	}
	m_header << endl;

	// Create default constructor
	m_header << indent << strct.get_cpp_identifier () << "();" << endl;

	m_module << mod_indent << strct.get_cpp_method_prefix ()
		 << "::" << strct.get_cpp_identifier () << "()" << endl
		 << mod_indent++ << "{" << endl;
	
	for (IDLStruct::const_iterator i = strct.begin (); i != strct.end (); i++)
	{
		IDLMember &member = (IDLMember &) **i;

		member.getType ()->member_init_cpp (m_module, mod_indent,
						    member.get_cpp_identifier ());
	}

	m_module << --mod_indent << "}" << endl << endl;
}

void IDLPassXlate::struct_create_converters (const IDLStruct &strct)
{
	// Create C->C++ constructor
	string construct_arg = "const " + strct.get_c_typename () + " &_c_struct";
	
	m_header << indent << "explicit " << strct.get_cpp_identifier ()
		 << " (" << construct_arg << ");" << endl << endl;

	m_module << mod_indent << strct.get_cpp_method_prefix ()
		 << "::" << strct.get_c_identifier ()
		 << " (" << construct_arg << ")" << endl;
	m_module << mod_indent++ << "{" << endl;
	m_module << mod_indent << "_orbitcpp_unpack (_c_struct);" << endl;
	m_module << --mod_indent << "}" << endl;

	// Create packers
	strct.write_packing_decl (m_header, indent);
	strct.write_packing_impl (m_module, mod_indent);
}

void IDLPassXlate::struct_create_typedefs (const IDLStruct &strct)
{
	if (strct.is_fixed ())
	{
		m_header << indent << "typedef "
			 << strct.get_cpp_identifier () << "& "
			 << strct.get_cpp_identifier () << "_out;"
			 << endl;
	} else {
		string data_prefix = IDL_IMPL_NS "::Data";
		string data_var = data_prefix + "_var< " + strct.get_cpp_identifier () + ">";
		string data_out = data_prefix + "_out< " + strct.get_cpp_identifier () + ">";
		
		m_header << indent << "typedef " << data_var << " "
			 << strct.get_cpp_identifier () << "_var;"
			 << endl;

		m_header << indent << "typedef " << data_out << " "
			 << strct.get_cpp_identifier () << "_out;"
			 << endl;
	}
}

void IDLPassXlate::struct_create_any (const IDLStruct &strct)
{
	m_header << indent;
	if (strct.getTopLevelInterface())
		m_header << "static ";

	string cpp_typecode = "_tc_" + strct.get_c_identifier ();
	string c_typecode = "TC_" + strct.get_c_typename ();
	
	m_header << "const CORBA::TypeCode_ptr " << cpp_typecode << " = "
		 << "(CORBA::TypeCode_ptr)" << c_typecode << ";" << endl;

	ORBITCPP_MEMCHECK (new IDLWriteStructAnyFuncs (strct, m_state, *this));
}


#if 0
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
	
	ORBITCPP_MEMCHECK(new IDLWriteCPPTraits (idlEnum, m_state, *this));
	ORBITCPP_MEMCHECK(new IDLWriteEnumAnyFuncs (idlEnum, m_state, *this));
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
IDLPassXlate::doException (IDL_tree  node,
			   IDLScope &scope)
{
	IDLException &except = (IDLException &) *scope.getItem(node);

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

	exception_create_members (except);

	m_header << endl << indent << "// methods" << endl;

	exception_create_constructors (except);
	
	m_header << indent << "void _raise ()" << endl
		 << indent++ << "{" << endl;
	m_header << indent << "throw *this;" << endl;
	m_header << --indent << '}' << endl << endl;

	m_header << indent << "static " << except.get_cpp_identifier () << " *_narrow "
		 << "(" << IDL_CORBA_NS "::Exception *ex)" << endl
		 << indent++ << "{" << endl;
	m_header << indent << "return dynamic_cast"
		 << "<" << except.get_cpp_identifier () << "*> (ex)"
		 << ';' << endl;
	m_header << --indent << '}' << endl;

	exception_create_converters (except);
	
	m_header << --indent << "};" << endl << endl;

	exception_create_any (except);	
}

void
IDLPassXlate::exception_create_members (const IDLException &ex)
{
	m_header << indent << "// members" << endl;

	for (IDLException::const_iterator i = ex.begin (); i != ex.end (); i++)
	{
		IDLMember &member = (IDLMember &) **i;

		m_header << indent
			 << member.getType ()->get_cpp_member_typename ()
			 << " " << member.get_cpp_identifier ()
			 << ';' << endl;
	}
}

void
IDLPassXlate::exception_create_constructors (const IDLException &ex)
{
	m_header << indent << "// copy ctor, dtor and assignment op will be auto-generated" << endl
		 << indent << ex.get_cpp_identifier () << "() { }" << endl;

	if (ex.size ())
	{
		// Create member init constructor
		
		string constructor_args;
		IDLException::const_iterator back = ex.end ();
		back--;
		for (IDLException::const_iterator i = ex.begin ();
		     i != ex.end (); i++)
		{
			IDLMember &member = (IDLMember &) **i;

			constructor_args += member.getType ()->member_decl_arg_get ();
			constructor_args += " _par_";
			constructor_args += member.get_cpp_identifier ();

			if (i != back)
				constructor_args += ", ";
		}

		m_header << indent << ex.get_cpp_identifier ()
			 << " (" << constructor_args << ")"
			 << ';' << endl << endl;
		
		m_module << mod_indent << ex.get_cpp_typename ()
			 << "::" << ex.get_cpp_identifier ()
			 << " (" << constructor_args << ")" << endl
			 << mod_indent++ << '{' << endl;

		for (IDLException::const_iterator i = ex.begin ();
		     i != ex.end (); i++)
		{
			IDLMember &member = (IDLMember &) **i;

			member.getType ()->member_impl_arg_copy (m_module, mod_indent,
								 member.get_cpp_identifier ());
		}
			
		m_module << --mod_indent << '}' << endl << endl;
	}
}

void
IDLPassXlate::exception_create_converters (const IDLException &ex)
{
	// _orbitcpp_set
	m_header << indent << "void _orbitcpp_set (::CORBA_Environment *ev);" << endl;
	
	m_module << mod_indent << "void " << ex.get_cpp_typename ()
		 << "::_orbitcpp_set (::CORBA_Environment *ev)"
		 << mod_indent++ << '{' << endl;
	if (ex.size ())
	{
		m_module << mod_indent <<  "::CORBA_exception_set (ev, ::CORBA_USER_EXCEPTION, "
			 << '"' << ex.getRepositoryId() << '"'
			 << ", _orbitcpp_pack ())"
			 << ';' << endl;
	} else {
		m_module << mod_indent <<  "::CORBA_exception_set (ev, ::CORBA_USER_EXCEPTION, "
			 << '"' << ex.getRepositoryId() << '"'
			 << ", 0)"
			 << ';' << endl;
	}
	m_module << --mod_indent << '}' << endl << endl;

	// _orbitcpp_pack
	ex.write_packing_decl (m_header, indent);
	ex.write_packing_impl (m_module, mod_indent);
}

void
IDLPassXlate::exception_create_any (const IDLException &ex)
{
	m_header << indent;
	if (ex.getTopLevelInterface())
		m_header << "static ";

	m_header << "const CORBA::TypeCode_ptr _tc_" << ex.get_cpp_identifier () << " = " 
		 << "(CORBA::TypeCode_ptr)TC_" + ex.get_c_typename ()
		 << ';' << endl << endl;
	
	ORBITCPP_MEMCHECK (new IDLWriteExceptionAnyFuncs(ex, m_state, *this));
}


void 
IDLPassXlate::doInterface (IDL_tree  node,
			   IDLScope &scope)
{
	IDLInterface &iface = (IDLInterface &) *scope.getItem(node);

	// Get namespace information
	string ns_iface_begin, ns_iface_end;
	iface.getParentScope()->getCPPNamespaceDecl(ns_iface_begin, ns_iface_end);

	bool non_empty_ns = ns_iface_end.size () || ns_iface_begin.size ();

	string ifname = iface.get_cpp_identifier();
	string _ptrname = iface.get_cpp_identifier_ptr();
	
	// Create forward declaration
	m_header << indent << "class " << ifname << ';' << endl;

	// Leave module namespace
	if (non_empty_ns)
		m_header << indent << ns_iface_end;

	// Enter stub namespace
	m_header << indent << "namespace " IDL_IMPL_NS_ID << endl
		 << indent << "{" << endl
		 << indent << "namespace " IDL_IMPL_STUB_NS_ID << endl
		 << indent << "{" << endl << endl
		 << indent << ns_iface_begin << endl;
	
	m_header << indent << "class " << ifname << ";" << endl << endl;

	// Leave stub namespace
	m_header << indent << ns_iface_end << endl
		 << indent << "}} //namespaces" << endl << endl;
	

	// Enter POA namespace
	string ns_poa_begin, ns_poa_end;
	iface.get_cpp_poa_namespace (ns_poa_begin, ns_poa_end);

	// predeclare POA type (necessary for typedef'ing)	
	if (non_empty_ns)
		m_header << ns_poa_begin << endl << endl;
	
	m_header << indent << "class " << iface.get_cpp_poa_identifier () << ';' << endl;
	
	if (non_empty_ns)
		m_header << ns_poa_end;
	
	// Re-enter module namespace
	if (non_empty_ns)
		m_header << ns_iface_begin;

	// Write typedefs
	iface.common_write_typedefs (m_header, indent);
	m_header << endl;

	// Write sequence elem traits
	ORBITCPP_MEMCHECK (new IDLWriteCPPTraits (iface, m_state, *this));
	
	// generate type container
	m_header << indent << "class " << iface.get_cpp_identifier ();

	if (iface.m_bases.size ())
	{
		m_header << " : ";
		IDLInterface::BaseList::const_iterator back = iface.m_bases.end ();
		back--;
		for (IDLInterface::BaseList::const_iterator i = iface.m_bases.begin ();
		     i != iface.m_bases.end (); i++)
		{
			m_header << "public " << (*i)->get_cpp_typename ();
			if (i != back)
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
		 << "(CORBA::TypeCode_ptr)TC_" + iface.get_c_typename () + ";" << endl << endl;

	ORBITCPP_MEMCHECK (new IDLWriteIfaceAnyFuncs (iface, m_state, *this));

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
		 << indent << '{' << endl;
	indent++;
	m_header << indent << "return CORBA_OBJECT_NIL;" << endl
		 << indent << '}' << endl;
	indent--;
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
		 << " cobj = ptr->_orbitcpp_cobj ();" << endl;
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

	string id = module->get_cpp_identifier ();
	m_header << indent << "namespace " << id << endl
		 << indent << '{' << endl;

	Super::doModule(node, *module);

	m_header << indent << "} //namespace " << id << endl << endl;
}

void IDLPassXlate::enumHook(IDL_tree next,IDLScope &scope) {
	if (!scope.getTopLevelInterface())
		runJobs(IDL_EV_TOPLEVEL);
}

// IDLWriteCPPTraits -------------------------------------------------------
IDLWriteCPPTraits::IDLWriteCPPTraits (const IDLElement &element,
				      IDLCompilerState &state,
				      IDLOutputPass    &pass) :
	IDLOutputJob (IDL_EV_TOPLEVEL, state, pass),
	m_element (element)
{
}

void
IDLWriteCPPTraits::run ()
{
	g_assert (dynamic_cast<const IDLType*>(&m_element));
	const IDLType &type = *dynamic_cast<const IDLType*>(&m_element);
	
	string cpp_id = m_element.get_cpp_identifier ();
	string c_id = m_element.get_c_typename ();
	
	// Write sequence elem traits
	string elem_cpp = type.get_cpp_member_typename ();
	string elem_c = type.get_c_member_typename ();

	string seq_elem_traits_id = cpp_id + "_seq_elem_traits";
	string seq_elem_traits_prefix = m_element.get_cpp_method_prefix () + "_seq_elem_traits";
	
	m_header << indent << "struct " << seq_elem_traits_id << endl
		 << indent++ << "{" << endl;
	
	// pack_elem (C++ -> C)
	m_header << indent << "void pack_elem (" << elem_cpp << " &cpp_value, "
		 << elem_c << " &c_value);" << endl;
	m_module << mod_indent << "void " << seq_elem_traits_prefix
		 << "::pack_elem (" << elem_cpp << " &cpp_value, "
		 << elem_c << " &c_value)" << endl
		 << mod_indent++ << "{" << endl;
	type.member_pack_to_c (m_module, mod_indent, "cpp_value", "c_value");
	m_module << --mod_indent << "}" << endl << endl;

	// unpack_elem (C -> C++)
	m_header << indent << "void unpack_elem (" << elem_cpp << " &cpp_value, "
		 << elem_c << " &c_value);" << endl;
	m_module << mod_indent << "void " << seq_elem_traits_prefix
		 << "::unpack_elem (" << elem_cpp << " &cpp_value, "
		 << elem_c << " &c_value)" << endl
		 << mod_indent++ << "{" << endl;
	type.member_unpack_from_c (m_module, mod_indent, "cpp_value", "c_value");
	m_module << --mod_indent << "}" << endl << endl;
	
	m_header << --indent << "};" << endl << endl;
}

// IDLWriteArrayProps -------------------------------------------------------
void IDLWriteArrayProps::run()
{
	string array_id = m_dest.get_cpp_typename ();
	string slice_id = array_id + "_slice";
	string props_id = m_dest.get_cpp_identifier () + "Props";

	int length = 1;
	for (IDLArray::const_iterator i = m_array.begin ();
	     i != m_array.end (); i++)
	{
		length *= *i;
	}

	// Alloc
	m_header << indent << "template<>" << endl;
	m_header << indent << "inline " << slice_id + " * " << props_id << "::alloc ()" << endl
		 << indent++ << "{" << endl;
	m_header << indent << "return " << array_id + "_alloc ();" << endl;
	m_header << --indent << "}" << endl << endl;

	// Free
	m_header << indent << "template<>" << endl;
	m_header << indent << "inline void " << props_id << "::free ("
		 << slice_id << " * target)" << endl
		 << indent++ << "{" << endl;
	m_header << indent << array_id + "_free (target);" << endl;
	m_header << --indent << "}" << endl << endl;

	// Copy
	m_header << indent << "template<>" << endl;
	m_header << indent << "inline void " << props_id << "::copy ("
		 << slice_id << " * dest, "
		 << "const " << slice_id << " * source)" << endl
		 << indent++ << "{" << endl;
	m_header << indent << array_id + "_copy (dest, source);" << endl;
	m_header << --indent << "}" << endl << endl;
}

// IDLWriteAnyFuncs -------------------------------------------------------
IDLWriteAnyFuncs::IDLWriteAnyFuncs (IDLCompilerState &state,
				    IDLOutputPass    &pass):
	IDLOutputJob ("", state, pass)
{
}


void IDLWriteAnyFuncs::writeAnyFuncs (bool          pass_value,
				      const string &cpptype, 
				      const string &ctype)
{
	if (pass_value)
	{
		writeInsertFunc (m_header, indent, FUNC_VALUE, cpptype, ctype);
		writeExtractFunc (m_header, indent, FUNC_VALUE, cpptype, ctype);
	} else {
		writeInsertFunc (m_header, indent, FUNC_COPY, cpptype, ctype);
		writeInsertFunc (m_header, indent, FUNC_NOCOPY, cpptype, ctype);
		writeExtractFunc (m_header, indent, FUNC_NOCOPY, cpptype, ctype);
	}
}

void IDLWriteAnyFuncs::writeInsertFunc (ostream      &ostr,
					Indent       &indent,
					FuncType      func, 
					string        ident,
					const string &ctype)
{
	string any_func, any_arg;
	any_func = "insert_simple";
	any_arg = "&val";
	if (func == FUNC_COPY) {
		ident += " const &";
	}
	else if (func == FUNC_NOCOPY) {
		ident += "*";
		any_arg = "val, CORBA_FALSE";
	}

	ostr << indent << "inline void operator <<= "
	     <<	"(CORBA::Any& the_any, " << ident << " val)" << endl
	     << indent++ << "{" << endl;
	
	ostr << indent << "the_any." << any_func 
	     <<" ((CORBA::TypeCode_ptr)TC_" << ctype << ", "
	     << any_arg << ");" << endl;
	
	ostr << --indent << endl << "}" << endl << endl;
}

void IDLWriteAnyFuncs::writeExtractFunc (ostream      &ostr,
					 Indent       &indent,
					 FuncType      func, 
					 string        ident,
					 const string &ctype)
{
	string any_func, any_arg;
	any_arg = "val";
	if (func == FUNC_VALUE) {
		ident += "&";
		any_func = "extract";
	} else {
		ident += " const *&";
		any_func = "extract_ptr";
	}
	
	ostr << indent << "inline CORBA::Boolean operator >>= "
	     << "(const CORBA::Any& the_any, " << ident << " val)" << endl
	     << indent++ << "{" << endl;
	
	ostr << indent << "return the_any." << any_func
	     << " ((CORBA::TypeCode_ptr)TC_" << ctype << ", "
	     << any_arg << ");" << endl;

	ostr << --indent << endl << "}" << endl << endl;
}

// IDLWriteIFaceAnyFuncs -------------------------------------------------------
IDLWriteIfaceAnyFuncs::IDLWriteIfaceAnyFuncs (const IDLInterface &_iface,
					      IDLCompilerState   &state,
					      IDLOutputPass      &pass) :
	IDLWriteAnyFuncs(state, pass),
	m_iface(_iface)
{
}

void
IDLWriteIfaceAnyFuncs::run()
{
	string cpptype = m_iface.get_cpp_typename ()+ "_ptr";
	string ctype = m_iface.get_c_typename ();
	writeInsertFunc(m_header, indent, FUNC_NOCOPY, cpptype, ctype);
	writeAnyFuncs(true, cpptype, ctype );
}

// IDLWriteEnumAnyFuncs -------------------------------------------------------

IDLWriteEnumAnyFuncs::IDLWriteEnumAnyFuncs (const IDLEnum    &_enum,
					    IDLCompilerState &state,
					    IDLOutputPass    &pass) :
	IDLWriteAnyFuncs (state, pass),
	m_enum(_enum)
{
}

void IDLWriteEnumAnyFuncs::run()
{
	writeAnyFuncs(true,
		      m_enum.get_cpp_typename (),
		      m_enum.get_c_typename ());
}


// IDLWriteStructAnyFuncs -------------------------------------------------------
IDLWriteStructAnyFuncs::IDLWriteStructAnyFuncs (const IDLStruct  &_struct,
						IDLCompilerState &state,
						IDLOutputPass    &pass):
	IDLWriteAnyFuncs (state, pass),
	m_element (_struct)
{
}

#if 0 //!!!
IDLWriteStructAnyFuncs::IDLWriteStructAnyFuncs (const IDLUnion   &_union,
						IDLCompilerState &state,
						IDLOutputPass    &pass):
	IDLWriteAnyFuncs (state, pass),
	m_element (_union)
{
}
#endif

void
IDLWriteStructAnyFuncs::run()
{
	writeAnyFuncs (false,
		       m_element.get_cpp_typename (),
		       m_element.get_c_typename ());
}

void
IDLWriteExceptionAnyFuncs::run ()
{
	string tc = "(CORBA::TypeCode_ptr)TC_" + m_element.get_c_typename ();
	string cpp_id = m_element.get_cpp_typename ();
	string c_id = m_element.get_c_typename ();
	
	// Operator <<=
	m_header << indent << "inline void operator <<= "
		 << "(CORBA::Any& the_any, " << cpp_id << " const &val)" << endl;
	m_header << indent++ << "{" << endl;

	m_header << indent << "the_any.insert_simple (" << tc << ", "
		 << "const_cast< " << cpp_id << " &>(val)._orbitcpp_pack(), CORBA_FALSE);" << endl;

	m_header << --indent << endl << "}" << endl << endl;

	// Operator >>=
	m_header << indent << "inline CORBA::Boolean operator >>= "
		 << "(const CORBA::Any& the_any, "<< cpp_id << " &val)" << endl;
	m_header << indent++ << "{" << endl;

	
	m_header << indent << "const " << c_id << " *ex;" << endl;
	m_header << indent << "if (the_any.extract_ptr (" << tc << ", ex))" << endl
		 << indent++ << "{" << endl;

	m_header << indent << "val._orbitcpp_unpack (*ex);"
		 << indent << "return true;" << endl;

	m_header << --indent << "} else {" << endl;
	indent++;
	
	m_header << indent << "return false;" << endl;
	m_header << --indent << "}" << endl;

	m_header << --indent << "}" << endl << endl;
}

// IDLWriteArrayAnyFuncs -------------------------------------------------------
IDLWriteArrayAnyFuncs::IDLWriteArrayAnyFuncs (const IDLArray   &_array,
					      const IDLElement &_dest, 
					      IDLCompilerState &state,
					      IDLOutputPass    &pass) :
	IDLWriteAnyFuncs(state, pass),
	m_array(_array),
	m_dest(_dest)
{
}


void
IDLWriteArrayAnyFuncs::run()
{
	string array_id = m_dest.get_cpp_typename ();
	string slice_id = array_id + "_slice";
	string forany_id = array_id + "_forany";
	string tc =  m_dest.getParentScope ()->get_cpp_typename () +
		"::_tc_" + m_dest.get_cpp_identifier ();

	
	// Operator <<=
	m_header << indent << "inline void operator <<= "
		 << "(CORBA::Any& the_any, " << forany_id << " &_arr)" << endl
		 << indent++ << "{" << endl;

	m_header << indent
		 << "the_any.insert_simple (" << tc << ", "
		 << "(" << slice_id << "*)_arr, !_arr._nocopy());" << endl;

	m_header << --indent << endl << "}" << endl;

	
	// Operator >>=
	m_header << indent << "inline CORBA::Boolean operator >>= "
		 << "(CORBA::Any& the_any, " << forany_id << " &_arr)" << endl
		 << indent++ << "{" << endl;

	m_header << indent << "const " << slice_id << " *tmp;" << endl
		 << indent << "CORBA::Boolean _retval;" << endl << endl;

	m_header << indent << "_retval = the_any.extract_ptr "
		 << "(" << tc << ", tmp);" << endl;
	m_header << indent << "_arr = (" << slice_id << "*)tmp;" << endl;
	m_header << indent << "return _retval;" << endl;

	m_header << --indent << "}" << endl << endl;
}
