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


#ifndef ORBITCPP_TYPES_IDLANY
#define ORBITCPP_TYPES_IDLANY

#include "IDLSimpleType.hh"

class IDLAny : public IDLSimpleType
{
	string getTypeName() const;
	string getCTypeName() const ;
	virtual bool isVariableLength() const ;
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl, IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id, IDLTypedef const *activeTypedef = NULL) const;
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl, IDLTypedef const *activeTypedef=NULL) const;
	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id, IDLTypedef const *activeTypedef = NULL) const;
	// stub return stuff
	void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl, IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent, IDLTypedef const *activeTypedef = NULL) const;
	string IDLAny::getCPPStubReturnAssignment() const;
	void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent, IDLTypedef const *activeTypedef = NULL) const ;

	// skel stuff
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl, IDLTypedef const *activeTypedef = NULL) const;
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id, IDLTypedef const *activeTypedef = NULL) const;

	// skel return stuff
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl, IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru, IDLTypedef const *activeTypedef = NULL) const;
	string getCPPSkelReturnAssignment(bool passthru, IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru, IDLTypedef const *activeTypedef = NULL) const;
	string getInvalidReturn() const;
};

#endif //ORBITCPP_TYPES_IDLANY
