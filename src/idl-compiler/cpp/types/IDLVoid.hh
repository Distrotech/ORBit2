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


#ifndef ORBITCPP_TYPES_IDLVOID
#define ORBITCPP_TYPES_IDLVOID

#include "IDLType.hh"

class IDLVoid : public IDLType
{
public:

	string getNSScopedCTypeName() const {
		return getCTypeName();
	}

	string getNSScopedCPPTypeName() const {
		throw IDLExVoid();
		return "";
	}
	
	virtual bool isVariableLength() const {
		return false;  // default case for most types
	}

	// misc stuff
	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}

	// struct / exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec, string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	
	// sequence stuff
	string getCTypeName() const {
		throw IDLExVoid();
	}

	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,
							  string &typespec,string &dcl,
							  IDLTypedef const* activeTypedef=NULL) const {
		throw IDLExVoid();
	}
	void writeCPPStubMarshalCode(IDL_param_attr attr,string const &id,
								 ostream &ostr, Indent &indent,
								 IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPStubDemarshalCode(IDL_param_attr attr,string const &id,
								   ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}

	// stub return stuff
	void getCPPStubReturnDeclarator(string const &id,string &typespec, string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = "void";
		dcl = id;
	}
	void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									IDLTypedef const *activeTypedef = NULL) const {
	}
	string getCPPStubReturnAssignment() const {
		return "";
	}
	void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const {
	}

	// skel stuff
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								 IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}

	// skel return stuff
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
		typespec = "void";
		dcl = id;
	}
	void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									IDLTypedef const *activeTypedef = NULL) const {
	}
	string getCPPSkelReturnAssignment(bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
		return "";
	}
	void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
									   IDLTypedef const *activeTypedef = NULL) const {
	}
	string getInvalidReturn() const {
		return "return;";
	}
};

#endif //ORBITCPP_TYPES_IDLVOID
