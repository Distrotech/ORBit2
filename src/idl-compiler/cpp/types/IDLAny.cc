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

#include "IDLAny.hh"

string IDLAny::getTypeName() const
{
		return IDL_CORBA_NS "::Any";
}

string IDLAny::getCTypeName() const
{
		return "CORBA_any";
}

bool IDLAny::isVariableLength() const
{
	return true;
}

void IDLAny::getCPPStructCtorDeclarator(string const &id, string &typespec, string &dcl, IDLTypedef const *activeTypedef = NULL) const
{
	typespec = "const " + getTypeName() + "&";
	dcl = "_par_" + id;
}

void IDLAny::writeCPPStructCtor(ostream &ostr, Indent &indent, string const &id, IDLTypedef const *activeTypedef = NULL) const
{
	ostr << indent << id << " = _par_" << id << ';' << endl;
}

void IDLAny::getCPPStubDeclarator(IDL_param_attr attr, string const &id, string &typespec, string &dcl, IDLTypedef const *activeTypedef=NULL) const
{
	switch (attr) {
	case IDL_PARAM_IN:
		typespec = "const " IDL_CORBA_NS "::Any";
		dcl = "&" + id;
		break;
	case IDL_PARAM_INOUT:
		typespec = IDL_CORBA_NS "::Any";
		dcl = "&" + id;
		break;
	case IDL_PARAM_OUT:
		typespec = IDL_CORBA_NS "::Any_out";
		dcl = id;
		break;
	}
}
string IDLAny::getCPPStubParameterTerm(IDL_param_attr attr, string const &id, IDLTypedef const *activeTypedef = NULL) const
{
	string retval;
	switch (attr) {
	case IDL_PARAM_IN:
	case IDL_PARAM_INOUT:
		retval = id + "._orbitcpp_get_c_any_ptr()";
		break;
	case IDL_PARAM_OUT:
		retval = idlGetCast("&" + id + ".ptr()", "CORBA_any**");
		break;
	}
	return retval;
}

// stub return stuff
void IDLAny::getCPPStubReturnDeclarator(string const &id, string &typespec, string &dcl, IDLTypedef const *activeTypedef = NULL) const
{
	typespec = IDL_CORBA_NS "::Any";
	dcl = "*" + id;
}

void IDLAny::writeCPPStubReturnPrepCode(ostream &ostr, Indent &indent, IDLTypedef const *activeTypedef = NULL) const
{
	ostr << indent << "CORBA_any *_retval = NULL;" << endl;
}

string IDLAny::getCPPStubReturnAssignment() const {
	return "_retval = ";
}

void IDLAny::writeCPPStubReturnDemarshalCode(ostream &ostr, Indent &indent, IDLTypedef const *activeTypedef = NULL) const
{
	ostr << indent << "return "
		<< idlGetCast("_retval", IDL_CORBA_NS "::Any*" ) << ";" << endl;
}

// skel stuff
void IDLAny::getCSkelDeclarator(IDL_param_attr attr, string const &id, string &typespec, string &dcl, IDLTypedef const *activeTypedef = NULL) const
{
	switch( attr ) {
	case IDL_PARAM_IN:
		typespec = "const CORBA_any";
		dcl = "*" + id;
		break;
	case IDL_PARAM_INOUT:
		typespec = "CORBA_any";
		dcl = "*" + id;
		break;
	default:
		typespec = "CORBA_any";
		dcl = "**"+id;
	}
}

string IDLAny::getCPPSkelParameterTerm(IDL_param_attr attr, string const &id, IDLTypedef const *activeTypedef = NULL) const
{
	switch(attr) {
	case IDL_PARAM_IN:
		return idlGetCast("*"+id, "const " IDL_CORBA_NS "::Any&");
	case IDL_PARAM_INOUT:
		return idlGetCast("*"+id, IDL_CORBA_NS "::Any&");
	default:
		return idlGetCast("*"+id, IDL_CORBA_NS "::Any*&");
	}
}

// skel return stuff
void IDLAny::getCSkelReturnDeclarator(string const &id, string &typespec, string &dcl,
							  IDLTypedef const *activeTypedef = NULL) const
{
	typespec = "CORBA_any";
	dcl = "*"+id;
}

void IDLAny::writeCPPSkelReturnPrepCode(ostream &ostr, Indent &indent,bool passthru, IDLTypedef const *activeTypedef = NULL) const
{
	ostr << indent << IDL_CORBA_NS "::Any *_retval = NULL;" << endl;
}

string IDLAny::getCPPSkelReturnAssignment(bool passthru, IDLTypedef const *activeTypedef = NULL) const
{
	return "_retval = ";
}

void IDLAny::writeCPPSkelReturnMarshalCode(ostream &ostr, Indent &indent,bool passthru, IDLTypedef const *activeTypedef = NULL) const
{
	ostr << indent << "return "
		<< idlGetCast( "_retval", "CORBA_any*") << ";" << endl;
}

string IDLAny::getInvalidReturn() const
{
	return "return NULL;";
}
