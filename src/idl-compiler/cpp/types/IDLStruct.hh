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


#ifndef ORBITCPP_TYPES_IDLSTRUCT
#define ORBITCPP_TYPES_IDLSTRUCT

#include "IDLCompound.hh"
#include "IDLType.hh"


class IDLStruct
 : public IDLCompound,
   public IDLType
{
public:
	IDLStruct(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLCompound(id,node,parentscope) {
	}
	bool isType() {
		return true;
	}

	virtual bool isVariableLength() const;
	
	string getCPP_var() const {
		return getCPPIdentifier() + "_var";
	}

	string getQualifiedCPP_var(IDLScope const *up_to = NULL) const {
		return getQualifiedCPPIdentifier(up_to) + "_var";
	}

	string getCPP_out() const {
		return getCPPIdentifier() + "_out";
	}

	string getQualifiedCPP_out(IDLScope const *up_to = NULL) const {
		return getQualifiedCPPIdentifier(up_to) + "_out";
	}

	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const;
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const;

	// struct / exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const;

	// sequence stuff
	string getCTypeName() const {
		return  getQualifiedCIdentifier();
	}

	string getNSScopedCTypeName() const {
		return IDL_IMPL_C_NS_NOTUSED + getCTypeName();
	}

	string getNSScopedCPPTypeName() const {
		return getQualifiedCPPIdentifier();
	}
	
	// union stuff
	virtual void writeUnionReferents(ostream &ostr,Indent &indent, string const &id,
									 string const &discriminatorVal,
									 IDLTypedef const *activeTypedef = NULL) const;
	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							  IDLTypedef const* activeTypedef=NULL) const;
	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const;

	// stub return stuff
	void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									IDLTypedef const *activeTypedef = NULL) const;
	string getCPPStubReturnAssignment() const;	
	void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
												 IDLTypedef const *activeTypedef = NULL) const;
	
	// skel stuff
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,
								   ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const;
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								 IDLTypedef const *activeTypedef = NULL) const;

	// skel return stuff
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									IDLTypedef const *activeTypedef = NULL) const;
	string getCPPSkelReturnAssignment(bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
									   IDLTypedef const *activeTypedef = NULL) const;
	string getInvalidReturn() const;

	// see IDLType for doc
	void writeInitCode(ostream &ostr, Indent &indent, string const &ident) const;
	void writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
	void writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
};

#endif //ORBITCPP_TYPES_IDLSTRUCT

