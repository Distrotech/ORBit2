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

bool
IDLInterface::isBaseClass(IDLInterface *iface) {
	BaseList::const_iterator first = m_allbases.begin(),last = m_allbases.end();
	while (first != last)
		if (*first++ == iface) return true;
	return false;
}

bool
IDLInterface::requiresSmartPtr() const
{
	for(IDLInterface::BaseList::const_iterator it = m_allbases.begin(); it != m_allbases.end(); it++)
	{
		if( (*it)->m_all_mi_bases.begin() != (*it)->m_all_mi_bases.end() )
		{
			return true;
		}
	}

	return false;
}



void
IDLInterface::writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
						   IDLElement &dest,IDLScope const &scope,
						   IDLTypedef const *activeTypedef = NULL) const {
	string id = dest.getCPPIdentifier();
	ostr
	<< indent << "typedef " << getCPPIdentifier() << ' ' << id << ';' << endl
	<< indent << "typedef " << getCPP_ptr() << ' ' << id << "_ptr" << ';' <<  endl
	<< indent << "typedef " << getCPP_mgr() << ' ' << id << "_mgr" << ';' <<  endl
	<< indent << "typedef " << getCPP_var() << ' ' << id << "_var" << ';' <<  endl
	<< indent << "typedef " << getCPP_out() << ' ' << id << "_out" << ';' <<  endl
	<< indent << "typedef " << getCPPIdentifier() << "Ref " << id << "Ref" << ';' <<  endl;

	// extra effort to typedef POA_ type
	string ns_outer_begin,ns_outer_end,ns_inner_begin,ns_inner_end;
	dest.getParentScope()->getCPPNamespaceDecl(ns_outer_begin,ns_outer_end);
	dest.getParentScope()->getCPPNamespaceDecl(ns_inner_begin,ns_inner_end,"POA_");

	ostr
	<< indent << ns_outer_end << ns_inner_begin << endl;
	indent++;
	ostr
	<< indent << "typedef " << getQualifiedCPP_POA() << ' ';
	if (dest.getParentScope() == getRootScope())
		ostr << "POA_";
	ostr << id << ';' << endl;
	
	// *** FIXME what about the _tie class?

	indent--;
	ostr
	<< indent << ns_inner_end << ns_outer_begin << endl;
}




void
IDLInterface::writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
								      IDLTypedef const *activeTypedef = NULL) const {
	ostr
	<< indent << IDL_IMPL_NS "::release_guarded(_cstruct." << id << ");" << endl
	<< indent << "_cstruct." << id << " = "
	<< IDL_IMPL_NS "::duplicate_guarded(*" << id << ".in());" << endl;
}




void
IDLInterface::writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
									    IDLTypedef const *activeTypedef = NULL) const {
	ostr
	<< id << " = "
	<< getQualifiedCPPCast(IDL_IMPL_NS "::duplicate_guarded(_cstruct."+id+")")
	<< ';' << endl;
}




void
IDLInterface::getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
								   IDLTypedef const *activeTypedef=NULL) const {
	dcl = id;

	switch (attr) {
	case IDL_PARAM_IN:
		typespec = getQualifiedCPP_ptr();
		break;
	case IDL_PARAM_INOUT:
		typespec = getQualifiedCPP_ptr();
		dcl = '&' + dcl;
		break;
	case IDL_PARAM_OUT:
		typespec = getQualifiedCPP_out();
		break;
	}
}



string
IDLInterface::getCPPStubParameterTerm(IDL_param_attr    attr,
				      string const     &id,
				      IDLTypedef const *activeTypedef = NULL) const
{
	string ctype = getNSScopedCTypeName();

	switch (attr) {
	case IDL_PARAM_IN:
		return id + "->_orbitcpp_get_c_object ()";
		return id + "->_orbitcpp_get_c_object()";
	case IDL_PARAM_INOUT:
		return "&(" + id + "->_orbitcpp_get_c_object ())";
	case IDL_PARAM_OUT:
		return "&(" + id + "->_orbitcpp_get_c_object ())";
	}
	return "";
}


void
IDLInterface::writeCPPStubReturnDemarshalCode (ostream          &ostr,
					       Indent           &indent,
					       IDLTypedef const *activeTypedef = NULL) const
{
	ostr << indent << "return " << getQualifiedCPPStub ()
	     << "::_orbitcpp_wrap (_retval, false);"
	     << endl;
}



void
IDLInterface::getCSkelDeclarator (IDL_param_attr    attr,
				  string const     &id,
				  string           &typespec,
				  string           &dcl,
				  IDLTypedef const *activeTypedef = NULL) const
{
	typespec = getNSScopedCTypeName();

	switch (attr) {
	case IDL_PARAM_IN:
		dcl = id;
		break;
	case IDL_PARAM_INOUT:
		dcl = '*' + id;
		break;
	case IDL_PARAM_OUT:
		dcl = '*' + id;
		break;
	}
}




void
IDLInterface::writeCPPSkelDemarshalCode (IDL_param_attr    attr,
					 string const     &id,
					 ostream          &ostr,
					 Indent           &indent,
					 IDLTypedef const *activeTypedef = NULL) const
{
	string ptr_name = "_" + id + "_ptr";
	string ptr_decl = getQualifiedCPP_var () + " " + ptr_name;
	
	switch (attr) {
		
	case IDL_PARAM_IN:
		ostr << indent << ptr_decl << " = "
		     << getQualifiedCPPCast (IDL_IMPL_NS "::duplicate_guarded (" + id + ")")
		     << ';' << endl;
		break;
		
	case IDL_PARAM_INOUT:
		ostr << indent << ptr_decl << " = "
		     << getQualifiedCPPCast (IDL_IMPL_NS "::duplicate_guarded (*" + id +")")
		     << ';' << endl;
		break;
		
	case IDL_PARAM_OUT:
		ostr << indent << ptr_decl << ';' << endl;
		break;
	}
}




void
IDLInterface::writeCPPSkelMarshalCode (IDL_param_attr    attr,
				       string const     &id,
				       ostream          &ostr,
				       Indent           &indent,
				       IDLTypedef const *activeTypedef = NULL) const
{
	string ptrname = "_" + id + "_ptr";
	switch (attr) {
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		ostr << indent << "*" << id << " = "
		     << ptrname << "._retn ()->_orbitcpp_get_c_object ();" << endl;
	default:
		break;
	}
}


void
IDLInterface::writeCPPSkelReturnMarshalCode (ostream          &ostr,
					     Indent           &indent,
					     bool              passthru,
					     IDLTypedef const *activeTypedef = NULL) const
{
	if (passthru)
		ostr << indent << "return _retval;" << endl;
	else 
		ostr << indent << "return " << getQualifiedCPPCast ("_retval")
		     << ";" << endl;
}

string
IDLInterface::getQualifiedCPPCast (string const &expr) const
{
	return getQualifiedCPPStub () + "::_orbitcpp_wrap ((" + expr + "), false)";
}
