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


#ifndef ORBITCPP_TYPES_IDLTYPEDEF
#define ORBITCPP_TYPES_IDLTYPEDEF

#include "IDLUserDefType.hh"

class IDLTypedef : public IDLUserDefType
{
protected:
	IDLType	&m_alias;

public:
	IDLTypedef(IDLType &alias,string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLUserDefType(id,node,parentscope),m_alias(alias) {
	}

	// misc stuff

	bool isVariableLength() const {
		return m_alias.isVariableLength();
	}
	IDLType const &getAlias() const {
		return m_alias;
	}
	
	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const {
		m_alias.getCPPMemberDeclarator(id,typespec,dcl,(activeTypedef ? activeTypedef : this));
	}
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeTypedef(ostr,indent,state,dest,scope,(activeTypedef ? activeTypedef : this));
	}

	void getCPPConstantDeclarator(string const &id,string &typespec,string &dcl) {
		m_alias.getCPPConstantDeclarator(id,typespec,dcl);
	}

	// Container accessors
	void writeForwarder (ostream &header_ostr,
			     Indent  &header_indent,
			     ostream &impl_ostr,
			     Indent  &impl_indent) const;
    
	// struct / exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		m_alias.getCPPStructCtorDeclarator(id,typespec,dcl,(activeTypedef ? activeTypedef : this));
	}
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPStructCtor(ostr,indent,id,(activeTypedef ? activeTypedef : this));
	}
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPStructPacker(ostr,indent,id,(activeTypedef ? activeTypedef : this));
	}
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPStructUnpacker(ostr,indent,id,(activeTypedef ? activeTypedef : this));
	}

	//sequence stuff

	string getCTypeName() const {
		return getQualifiedCIdentifier();
	}


	// union stuff
	virtual void writeUnionAccessors(ostream &ostr,Indent &indent, string const &id,
									string const &discriminatorVal,
									 IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeUnionAccessors(ostr,indent,id,discriminatorVal,
									(activeTypedef ? activeTypedef : this));
	}
	virtual void writeUnionModifiers(ostream &ostr,Indent &indent, string const &id,
									string const &discriminatorVal,
									 IDLTypedef const *activeTypedef = NULL) const{
		m_alias.writeUnionModifiers(ostr,indent,id,discriminatorVal,
									(activeTypedef ? activeTypedef : this));

	}
	virtual void writeUnionReferents(ostream &ostr,Indent &indent, string const &id,
									string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const{
		m_alias.writeUnionReferents(ostr,indent,id,discriminatorVal,
									(activeTypedef ? activeTypedef : this));

	}



	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,
							  string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef = NULL) const {
		m_alias.getCPPStubDeclarator(attr,id,typespec,dcl, (activeTypedef ? activeTypedef : this));
	}
	void writeCPPStubMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								 IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPStubMarshalCode(attr,id,ostr,indent,(activeTypedef ? activeTypedef : this));
	}
	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		return m_alias.getCPPStubParameterTerm(attr,id,(activeTypedef ? activeTypedef : this));
	}
	void writeCPPStubDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPStubDemarshalCode(attr,id,ostr,indent,(activeTypedef ? activeTypedef : this));
	}

	// stub return stuff
	void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		m_alias.getCPPStubReturnDeclarator(id,typespec,dcl,(activeTypedef ? activeTypedef : this));
	}
	void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPStubReturnPrepCode(ostr,indent,(activeTypedef ? activeTypedef : this));
	}
	string getCPPStubReturnAssignment() const {
		return m_alias.getCPPStubReturnAssignment();
	}
	void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPStubReturnDemarshalCode(ostr,indent,(activeTypedef ? activeTypedef : this));
	}

	// skel stuff
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const {
		m_alias.getCSkelDeclarator(attr,id,typespec,dcl,(activeTypedef ? activeTypedef : this));
	}
	void writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPSkelDemarshalCode(attr,id,ostr,indent,(activeTypedef ? activeTypedef : this));
	}
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		return m_alias.getCPPSkelParameterTerm(attr,id,(activeTypedef ? activeTypedef : this));
	}
	void writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								 IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPSkelMarshalCode(attr,id,ostr,indent,(activeTypedef ? activeTypedef : this));
	}

	// skel return stuff
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
		m_alias.getCSkelReturnDeclarator(id,typespec,dcl,(activeTypedef ? activeTypedef : this));
	}
	void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPSkelReturnPrepCode(ostr,indent,passthru,(activeTypedef ? activeTypedef : this));
	}
	string getCPPSkelReturnAssignment(bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
		return m_alias.getCPPSkelReturnAssignment(passthru,(activeTypedef ? activeTypedef : this));
	}
	void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
									   IDLTypedef const *activeTypedef = NULL) const {
		m_alias.writeCPPSkelReturnMarshalCode(ostr,indent,passthru,(activeTypedef ? activeTypedef : this));
	}
	string getInvalidReturn() const {
		return m_alias.getInvalidReturn();
	}

  	// CORBA C++ 2.3 write code to initialise type for use in structs, sequences
	// and arrays. (e.g. in the above, strings must be init to "", not NULL)
	virtual void writeInitCode(ostream &ostr, Indent &indent, string const &ident) const {
		m_alias.writeInitCode(ostr,indent,ident);
    }

	virtual void writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident, string const &target) const {
		m_alias.writeCPPDeepCopyCode(ostr,indent,ident,target);
    }
	virtual void writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident, string const &target) const {
		m_alias.writeCDeepCopyCode(ostr,indent,ident,target);
    }
};

#endif //ORBITCPP_TYPES_IDLTYPEDEF

