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

#include "IDLSimpleType.hh"

string IDLSimpleType::getNSScopedCTypeName() const
{
	return getCTypeName(); // simple types are not scoped, hence same as CTypeName
}

string IDLSimpleType::getNSScopedCPPTypeName() const
{
	return getTypeName();
}
		
bool IDLSimpleType::isVariableLength() const
{
	return false;
}

void IDLSimpleType::getCPPConstantDeclarator(string const &id,string &typespec,string &dcl)
{
	typespec = getTypeName();
	dcl = id;
}

// misc stuff
void IDLSimpleType::getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const
{
	typespec = getTypeName();
	dcl = "&" + id;
}

void IDLSimpleType::writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
				  IDLElement &dest,IDLScope const &scope,
				  IDLTypedef const *activeTypedef = NULL) const
{
	ostr
	<< indent << "typedef " << getTypeName() << ' '
	<< dest.getCPPIdentifier() << ';' << endl
	<< indent << "typedef " << getTypeName() << "_out "
	<< dest.getCPPIdentifier() << "_out;" << endl;
}

// struct / exception stuff
void IDLSimpleType::getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const
{
	typespec = getTypeName();
	dcl = "_par_" + id;
}

void IDLSimpleType::writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
						IDLTypedef const *activeTypedef = NULL) const
{
	ostr << indent << id << " = _par_" << id << ';' << endl;
}

void IDLSimpleType::writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
						  IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << idlGetCast("_cstruct."+id,getTypeName()+"&") << " = "
		 << id << ';' << endl;
}

void IDLSimpleType::writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const
{
	ostr << indent << id << " = "
		 << idlGetCast("_cstruct."+id,"const " +getTypeName()+"&") << ';' << endl;
}

// stub stuff
void IDLSimpleType::getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
						  IDLTypedef const *activeTypedef=NULL) const
{
	typespec = activeTypedef ?
		activeTypedef->getQualifiedCPPIdentifier() : getTypeName();
	if (attr == IDL_PARAM_OUT) typespec += "_out";
	dcl = attr == IDL_PARAM_INOUT ? "&"+id : id;
}

string IDLSimpleType::getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
							   IDLTypedef const *activeTypedef = NULL) const
{
	string typespec,dcl;
	getCSkelDeclarator(attr,"",typespec,dcl,activeTypedef);
	return idlGetCast((attr == IDL_PARAM_IN ? id : "&"+id),
					  typespec+dcl + (attr == IDL_PARAM_IN ? "&" : ""));
}

// stub return stuff
void IDLSimpleType::getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const
{
	typespec = activeTypedef ?
		activeTypedef->getQualifiedCPPIdentifier() : getTypeName();
	dcl = id;
}
void IDLSimpleType::writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
								IDLTypedef const *activeTypedef = NULL) const
{
	string typespec,dcl;
	getCSkelReturnDeclarator("_retval",typespec,dcl,activeTypedef);
	ostr << indent << typespec << " " << dcl << ";" << endl;
}

string IDLSimpleType::getCPPStubReturnAssignment() const
{
	return "_retval = ";
}

void IDLSimpleType::writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
									 IDLTypedef const *activeTypedef = NULL) const
{
	string typespec,dcl;
	getCPPStubReturnDeclarator("",typespec,dcl,activeTypedef);	
	ostr << indent << "return " << idlGetCast("_retval",typespec+dcl+"&") << ";" << endl;
}

// skel stuff
void IDLSimpleType::getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
						IDLTypedef const *activeTypedef = NULL) const {
	typespec = attr == IDL_PARAM_IN ? "const " : "";
	typespec += activeTypedef ? activeTypedef->getNSScopedCTypeName() : getNSScopedCTypeName();
	dcl = attr == IDL_PARAM_IN ? id : "*"+id;
}
	
string IDLSimpleType::getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
							   IDLTypedef const *activeTypedef = NULL) const {
	string typespec,dcl;
	getCPPStubDeclarator(attr,"",typespec,dcl,activeTypedef);
	string targetType = (attr == IDL_PARAM_IN ? "const " : "");
	targetType += typespec+dcl + (attr == IDL_PARAM_IN ? "&" : "");
	return idlGetCast((attr == IDL_PARAM_IN ? id : "*"+id),targetType);
}

// skel return stuff
void IDLSimpleType::getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef = NULL) const {
	typespec = activeTypedef ? activeTypedef->getNSScopedCTypeName() : getNSScopedCTypeName();
	dcl = id;
}

void IDLSimpleType::writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
								IDLTypedef const *activeTypedef = NULL) const
{
	string typespec,dcl;
	getCPPStubReturnDeclarator("_retval",typespec,dcl,activeTypedef);
	ostr << indent << typespec << " " << dcl << ";" << endl;
}

string IDLSimpleType::getCPPSkelReturnAssignment(bool passthru,
								  IDLTypedef const *activeTypedef = NULL) const
{
	return "_retval = ";
}
void IDLSimpleType::writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
								   IDLTypedef const *activeTypedef = NULL) const
{

	string typespec,dcl;
	getCSkelReturnDeclarator("",typespec,dcl,activeTypedef);	
	ostr << indent << "return " << idlGetCast("_retval",typespec+dcl+"&") << ";" << endl;
}

string IDLSimpleType::getInvalidReturn() const
{
	return "return 0;";
}

string
IDLSimpleType::getQualifiedForwarder () const
{
	return getTypeName () + "&";
}

void
IDLSimpleType::writeForwarder (ostream &header_ostr,
			       Indent  &header_indent,
			       ostream &impl_ostr,
			       Indent  &impl_indent) const
{
}
