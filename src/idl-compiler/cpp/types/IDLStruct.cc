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

#include "IDLStruct.hh"
#include "IDLTypedef.hh"

void
IDLStruct::writeInitCode(ostream &ostr, Indent &indent, string const &ident) const {
	IDLStruct::const_iterator first = begin(),last = end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		member.getType()->writeInitCode(ostr,indent,ident+"."+member.getCPPIdentifier());
	}
}

void IDLStruct::writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
	IDLStruct::const_iterator first = begin(),last = end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		member.getType()->writeCPPDeepCopyCode(ostr,indent,ident+"."+member.getCPPIdentifier(),target+"."+member.getCPPIdentifier());
	}
}

void IDLStruct::writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
	IDLStruct::const_iterator first = begin(),last = end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		member.getType()->writeCDeepCopyCode(ostr,indent,ident+"."+member.getCPPIdentifier(),target+"."+member.getCPPIdentifier());
	}
}

bool
IDLStruct::isVariableLength() const {
	IDLStruct::const_iterator first = begin(), last = end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		if(member.getType()->isVariableLength()){
			return true;
		}
	}
	return false;
}


void
IDLStruct::getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
	typespec = getQualifiedCPPIdentifier(getRootScope());
	dcl = id;
};

void
IDLStruct::writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
						IDLElement &dest,IDLScope const &scope,
						IDLTypedef const *activeTypedef = NULL) const {
	ostr
	<< indent << "typedef " << getQualifiedCPPIdentifier()
	<< " " << dest.getCPPIdentifier() << ";" << endl;

	if(isVariableLength()){
	  	ostr
		<< indent << "typedef " << getQualifiedCPPIdentifier()
		<< "_var " << dest.getCPPIdentifier() << "_var;" << endl;
	}

	ostr
	<< indent << "typedef " << getQualifiedCPPIdentifier()
	<< "_out " << dest.getCPPIdentifier() << "_out;" << endl;
}


void
IDLStruct::getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									  IDLTypedef const *activeTypedef = NULL) const {
	ORBITCPP_NYI("struct getCPPStructCtorDeclarator");
}

void
IDLStruct::writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const {
	ORBITCPP_NYI("struct writeCPPStructCtor");
}

void
IDLStruct::writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const {
	ORBITCPP_NYI("struct writeCPPStructPacker");
}

void
IDLStruct::writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								  IDLTypedef const *activeTypedef = NULL) const {
	ORBITCPP_NYI("struct writeCPPStructUnpacker");
}


void
IDLStruct::writeUnionReferents(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
							IDLTypedef const *activeTypedef = NULL) const{
	ostr
	<< indent << getQualifiedCPPIdentifier() << " &" << id << "() {" << endl;
	ostr	
	<< ++indent << "return reinterpret_cast< " << getQualifiedCPPIdentifier()
	<< "&>(m_target._u." << id << ");" << endl;
	ostr
	<< --indent << "}" << endl;
}


void IDLStruct::getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
									 IDLTypedef const *activeTypedef=NULL) const {
	dcl = id;

	string name = activeTypedef ?
		activeTypedef->getQualifiedCPPIdentifier() : getQualifiedCPPIdentifier();
	switch (attr) {
	case IDL_PARAM_IN:
		typespec = "const " + name;
		dcl = '&' + dcl;
		break;
	case IDL_PARAM_INOUT:
		typespec = name;
		dcl = '&' + dcl;
		break;
	case IDL_PARAM_OUT:
		typespec = name + "_out";
		break;
	}
}

string
IDLStruct::getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {

	string typespec,dcl;
	getCSkelDeclarator(attr,"",typespec,dcl,activeTypedef);

	switch (attr) {
	case IDL_PARAM_IN:
	case IDL_PARAM_INOUT:
		return idlGetCast("&"+id,typespec+dcl);
	case IDL_PARAM_OUT:
		if(isVariableLength())
			return idlGetCast("&"+id+".ptr()",typespec+dcl);
		else
			return idlGetCast("&"+id,typespec+dcl);
	}
	return "";
}

void
IDLStruct::getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									  IDLTypedef const *activeTypedef = NULL) const {
	typespec = getQualifiedCPPIdentifier();
	if(isVariableLength())
		dcl = "*" + id;
	else
		dcl = id;
}

void
IDLStruct::writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									  IDLTypedef const *activeTypedef = NULL) const {

	ostr
	<< indent << getNSScopedCTypeName();
	if(isVariableLength())
		ostr << " *_retval = NULL;" << endl;
	else
		ostr << " _retval;" << endl;
}


string
IDLStruct::getCPPStubReturnAssignment() const {
	return "_retval = ";		
}

void
IDLStruct::writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
										   IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << "return reinterpret_cast< " << getQualifiedCPPIdentifier();
	if(isVariableLength())
		ostr << "*";
	else
		ostr << "&";
	ostr << ">(_retval);" << endl;
}		


void
IDLStruct::getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef = NULL) const {
	typespec = getNSScopedCTypeName();
	
	switch (attr) {
	case IDL_PARAM_IN:
		typespec = "const " + typespec;
		dcl = '*' + id;
		break;
	case IDL_PARAM_INOUT:
		dcl = '*' + id;
		break;
	case IDL_PARAM_OUT:
		if(isVariableLength())
			dcl = "**" + id;
		else
			dcl = '*' + id;
		break;
	}
}




void
IDLStruct::writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
									 IDLTypedef const *activeTypedef = NULL) const {
	// no demarshalling code required
}




string
IDLStruct::getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
	string typespec,dcl;
	getCPPStubDeclarator(attr,"",typespec,dcl);

	switch (attr) {
	case IDL_PARAM_IN:
	case IDL_PARAM_INOUT:
		return idlGetCast("*"+id,typespec+dcl);
	case IDL_PARAM_OUT:
		if(isVariableLength())
			return idlGetCast("*"+id,getQualifiedCPPIdentifier()+"*&");
		else
			return idlGetCast("*"+id,typespec+dcl);
	}
	return "";
}



void
IDLStruct::writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const {
	// no marshalling code required
}


void
IDLStruct::getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
	typespec = getNSScopedCTypeName();
	if (isVariableLength())
		dcl = "*" + id;
	else
		dcl = id;
}

void
IDLStruct::writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << getQualifiedCPPIdentifier();
	if (isVariableLength())
		ostr << " *_retval = NULL;" << endl;
	else
		ostr << " _retval;" << endl;
}

string
IDLStruct::getCPPSkelReturnAssignment(bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
	return "_retval = ";		
}

void
IDLStruct::writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
										 IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << "return reinterpret_cast< "
	<< getNSScopedCTypeName();

	if(isVariableLength())
		ostr << "*";
	else
		ostr << "&";
	ostr
		<< ">(_retval);" << endl;
}


string
IDLStruct::getInvalidReturn() const {
	if(isVariableLength())
		return "return NULL;";
	else
		return "return reinterpret_cast< "
			+ getNSScopedCTypeName() + "&>(_retval);\n";
}
