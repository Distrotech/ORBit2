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


#ifndef ORBITCPP_TYPES_IDLTYPE
#define ORBITCPP_TYPES_IDLTYPE

//#include "IDLTypedef.hh"
#include "error.hh"
#include "language.hh"

class IDLTypedef;
class IDLScope;
class IDLCompilerState;

class IDLType
{
public:
	virtual ~IDLType() {}
	
	// misc stuff
	virtual bool isVariableLength() const = 0;
	virtual void getCPPMemberDeclarator(string const &id, string &typespec, string &dcl, IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeTypedef(ostream &ostr, Indent &indent, IDLCompilerState &state, IDLElement &dest, IDLScope const &scope, IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPSpecCode(ostream &ostr, Indent &indent, IDLCompilerState &state) const {
	}
	virtual void getCPPConstantDeclarator(string const &id, string &typespec, string &dcl) {
		throw IDLExNoConstantOfThisType(getCTypeName()); // hack
	}

	// Container accessors
	// Get forwarder type's name
	virtual string getQualifiedForwarder () const;
	// Create typename_forward type
	virtual void writeForwarder (ostream &header_ostr,
				     Indent  &header_indent,
				     ostream &impl_ostr,
				     Indent  &impl_indent) const = 0;
	
	// struct / exception stuff
	virtual void getCPPStructCtorDeclarator(string const &id, string &typespec, string &dcl,
											IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPStructCtor(ostream &ostr, Indent &indent, string const &id,
									IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPStructPacker(ostream &ostr, Indent &indent, string const &id,
									  IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPStructUnpacker(ostream &ostr, Indent &indent, string const &id,
										IDLTypedef const *activeTypedef = NULL) const = 0;

	// union stuff
	virtual void writeUnionAccessors(ostream &ostr, Indent &indent, string const &id,
									string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const;
	virtual void writeUnionModifiers(ostream &ostr, Indent &indent, string const &id,
									string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const;
	virtual void writeUnionReferents(ostream &ostr, Indent &indent, string const &id,
									string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const;


	// sequence stuff
	virtual string getCTypeName() const = 0;         // e.g. blah_foo
	virtual string getNSScopedCTypeName() const = 0; // e.g. _orbitcpp::c::blah_foo
	virtual string getNSScopedCPPTypeName() const = 0; // e.g. blah::foo

	// stub stuff
	virtual void getCPPStubDeclarator(IDL_param_attr attr, string const &id, string &typespec,
									  string &dcl,const IDLTypedef *activeTypedef = NULL) const = 0;
	virtual void writeCPPStubMarshalCode(IDL_param_attr attr, string const &id,ostream &ostr,
										 Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const {
	}
	virtual string getCPPStubParameterTerm(IDL_param_attr attr, string const &id,
										   IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPStubDemarshalCode(IDL_param_attr attr, string const &id,ostream &ostr,
										   Indent &indent,
										   IDLTypedef const *activeTypedef = NULL) const {
	}

	// stub return stuff

	virtual void getCPPStubReturnDeclarator(string const &id, string &typespec, string &dcl,
											IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPStubReturnPrepCode(ostream &ostr, Indent &indent,
											IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual string getCPPStubReturnAssignment() const = 0;
	virtual void writeCPPStubReturnDemarshalCode(ostream &ostr, Indent &indent,
												 IDLTypedef const *activeTypedef = NULL) const = 0;

	// skel stuff
	virtual void getCSkelDeclarator(IDL_param_attr attr, string const &id,
									string &typespec, string &dcl,
									IDLTypedef const *activeTypedef = NULL) const = 0;

	virtual void writeCPPSkelDemarshalCode(IDL_param_attr attr, string const &id,
										   ostream &ostr, Indent &indent,
										   IDLTypedef const *activeTypedef = NULL) const {
	}

	virtual string getCPPSkelParameterTerm(IDL_param_attr attr, string const &id,
										   IDLTypedef const *activeTypedef = NULL) const = 0;

	virtual void writeCPPSkelMarshalCode(IDL_param_attr attr, string const &id,
										 ostream &ostr, Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const {
	}

	// skel return stuff
	// "passthru" needed for fake skeletons - see pass_skel.cc, find "passthru"
	virtual void getCSkelReturnDeclarator(string const &id, string &typespec, string &dcl,
										  IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPSkelReturnPrepCode(ostream &ostr, Indent &indent, bool passthru = false,
											IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual string getCPPSkelReturnAssignment(bool passthru = false,
											  IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPSkelReturnMarshalCode(ostream &ostr, Indent &indent, bool passthru = false,
											   IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual string getInvalidReturn() const = 0;

  	// CORBA C++ 2.3 write code to initialise type for use in structs, sequences
	// and arrays. (e.g. in the above, strings must be init to "", not NULL)
	virtual void writeInitCode(ostream &ostr, Indent &indent, string const &ident) const;
	virtual void writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident, string const &target) const;
	virtual void writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident, string const &target) const;
	
	// if this type is really an alias, this gets the aliased type
	IDLType const &getResolvedType() const;
};

#endif //ORBITCPP_TYPES_IDLTYPE


