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
 *  Remarks:
 *    It is convention that the C struct is available by reference as "_cstruct"
 *    in the writeCPPStructPacker/Unpacker() contexts.
 *
 */




#ifndef ORBITCPP_TYPES
#define ORBITCPP_TYPES




#include "error.hh"
#include "language.hh"
#include "types.hh"




// forward --------------------------------------------------------------------
class IDLTypedef;
class IDLCompilerState;




// IDLType --------------------------------------------------------------------
class IDLType {
public:
	virtual ~IDLType() {}

	// misc stuff
	virtual bool isVariableLength() const = 0;
	virtual void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
										IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
							  IDLElement &dest,IDLScope const &scope,
							  IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPSpecCode(ostream &ostr,Indent &indent,IDLCompilerState &state) const {
	}
	virtual void getCPPConstantDeclarator(string const &id,string &typespec,string &dcl) {
		throw IDLExNoConstantOfThisType(getCTypeName()); // hack
	}

	// struct / exception stuff
	virtual void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
											IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
									IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
									  IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
										IDLTypedef const *activeTypedef = NULL) const = 0;

	// union stuff 
	virtual void writeUnionAccessors(ostream &ostr,Indent &indent, string const &id,
									string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const;
	virtual void writeUnionModifiers(ostream &ostr,Indent &indent, string const &id,
									string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const;
	virtual void writeUnionReferents(ostream &ostr,Indent &indent, string const &id,
									string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const;

  
	// sequence stuff
	virtual string getCTypeName() const = 0;         // e.g. blah_foo
	virtual string getNSScopedCTypeName() const = 0; // e.g. _orbitcpp::c::blah_foo
	virtual string getNSScopedCPPTypeName() const = 0; // e.g. blah::foo
  
	// stub stuff
	virtual void getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,
									  string &dcl,const IDLTypedef *activeTypedef = NULL) const = 0;
	virtual void writeCPPStubMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,
										 Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const {
	}
	virtual string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
										   IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPStubDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,
										   Indent &indent,
										   IDLTypedef const *activeTypedef = NULL) const {
	}

	// stub return stuff

	virtual void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
											IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
											IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual string getCPPStubReturnAssignment() const = 0;
	virtual void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
												 IDLTypedef const *activeTypedef = NULL) const = 0;

	// skel stuff
	virtual void getCSkelDeclarator(IDL_param_attr attr,string const &id,
									string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,
										   ostream &ostr, Indent &indent,
										   IDLTypedef const *activeTypedef = NULL) const {
	}
	virtual string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
										   IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,
										 ostream &ostr,Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const {
	}

	// skel return stuff
	// "passthru" needed for fake skeletons - see pass_skel.cc, find "passthru"
	virtual void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
										  IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru = false,
											IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual string getCPPSkelReturnAssignment(bool passthru = false,
											  IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru = false,
											   IDLTypedef const *activeTypedef = NULL) const = 0;
	virtual string getInvalidReturn() const = 0;

  	// CORBA C++ 2.3 write code to initialise type for use in structs, sequences
	// and arrays. (e.g. in the above, strings must be init to "", not NULL)
	virtual void writeInitCode(ostream &ostr, Indent &indent, string const &ident) const;
	virtual void writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
	virtual void writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
	
	// if this type is really an alias, this gets the aliased type
	IDLType const &getResolvedType() const;
};



// User defined types ---------------------------------------------------------
class IDLUserDefType : public IDLElement,public IDLType {
public:
	IDLUserDefType(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLElement(id,node,parentscope) {}

	virtual bool isType() {
		return true;
	}

	string getNSScopedCTypeName() const {
		return IDL_IMPL_C_NS "::" + getCTypeName();
	}
	string getNSScopedCPPTypeName() const { 
		return getQualifiedCPPIdentifier();
	}
};


class IDLUserDefScopeType : public IDLScope,public IDLType {
public:
	IDLUserDefScopeType(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLScope(id,node,parentscope) {}

	string getNSScopedCTypeName() const {
		return IDL_IMPL_C_NS "::" + getCTypeName();
	}

	string getNSScopedCPPTypeName() const { 
		return getQualifiedCPPIdentifier();
	}
	
	bool isType() {
		return true;
	}
};



class IDLTypedef : public IDLUserDefType {
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




// IDLSimpleType --------------------------------------------------------------
class IDLSimpleType : public IDLType {
public:
	virtual string getCTypeName() const = 0;
	virtual string getTypeName() const = 0;
	string getNSScopedCTypeName() const { 
		return getCTypeName(); // simple types are not scoped, hence same as CTypeName
	} 
	virtual string getNSScopedCPPTypeName() const {
		return getTypeName();
	}
		
	virtual bool isVariableLength() const {
		return false;
	}
	void getCPPConstantDeclarator(string const &id,string &typespec,string &dcl) {
		typespec = getTypeName();
		dcl = id;
	}

	// misc stuff
	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const {
		typespec = getTypeName();
		dcl = id;
	}
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const {
		ostr
		<< indent << "typedef " << getTypeName() << ' '
		<< dest.getCPPIdentifier() << ';' << endl
		<< indent << "typedef " << getTypeName() << "_out "
		<< dest.getCPPIdentifier() << "_out;" << endl;
	}

	// struct / exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = getTypeName();
		dcl = "_par_" + id;
	}
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << id << " = _par_" << id << ';' << endl;
	}
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << idlGetCast("_cstruct."+id,getTypeName()+"&") << " = "
			 << id << ';' << endl;
	}
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << id << " = "
			 << idlGetCast("_cstruct."+id,"const " +getTypeName()+"&") << ';' << endl;
	}

	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef=NULL) const {
		typespec = activeTypedef ?
			activeTypedef->getQualifiedCPPIdentifier() : getTypeName();
		if (attr == IDL_PARAM_OUT) typespec += "_out";
		dcl = attr == IDL_PARAM_INOUT ? "&"+id : id;
	}

	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		string typespec,dcl;
		getCSkelDeclarator(attr,"",typespec,dcl,activeTypedef);
		return idlGetCast((attr == IDL_PARAM_IN ? id : "&"+id),
						  typespec+dcl + (attr == IDL_PARAM_IN ? "&" : ""));
	}

	// stub return stuff
	void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = activeTypedef ?
			activeTypedef->getQualifiedCPPIdentifier() : getTypeName();
		dcl = id;
	}
	void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									IDLTypedef const *activeTypedef = NULL) const {
		string typespec,dcl;
		getCSkelReturnDeclarator("_retval",typespec,dcl,activeTypedef);
		ostr << indent << typespec << " " << dcl << ";" << endl;
	}

	string getCPPStubReturnAssignment() const {
		return "_retval = ";
	}

	void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const {
		string typespec,dcl;
		getCPPStubReturnDeclarator("",typespec,dcl,activeTypedef);	
		ostr << indent << "return " << idlGetCast("_retval",typespec+dcl+"&") << ";" << endl;
	}

	// skel stuff
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const {
		typespec = attr == IDL_PARAM_IN ? "const " : "";
		typespec += activeTypedef ? activeTypedef->getNSScopedCTypeName() : getNSScopedCTypeName();
		dcl = attr == IDL_PARAM_IN ? id : "*"+id;
	}
	
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		string typespec,dcl;
		getCPPStubDeclarator(attr,"",typespec,dcl,activeTypedef);
		string targetType = (attr == IDL_PARAM_IN ? "const " : "");
		targetType += typespec+dcl + (attr == IDL_PARAM_IN ? "&" : "");
		return idlGetCast((attr == IDL_PARAM_IN ? id : "*"+id),targetType);
	}

	// skel return stuff
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
		typespec = activeTypedef ? activeTypedef->getNSScopedCTypeName() : getNSScopedCTypeName();
		dcl = id;
	}

	void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									IDLTypedef const *activeTypedef = NULL) const {
		string typespec,dcl;
		getCPPStubReturnDeclarator("_retval",typespec,dcl,activeTypedef);
		ostr << indent << typespec << " " << dcl << ";" << endl;
	}

	string getCPPSkelReturnAssignment(bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
		return "_retval = ";
	}
	void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
									   IDLTypedef const *activeTypedef = NULL) const {

		string typespec,dcl;
		getCSkelReturnDeclarator("",typespec,dcl,activeTypedef);	
		ostr << indent << "return " << idlGetCast("_retval",typespec+dcl+"&") << ";" << endl;
	}

	string getInvalidReturn() const {
		return "return 0;";
	}
};


// Handy for things like enum (and maybe fixed?)
// since a good default implementation is available in SimpleType
class IDLUserDefSimpleType : public IDLElement, public IDLSimpleType {
public:
	IDLUserDefSimpleType(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLElement(id,node,parentscope) {}

	string getTypeName() const { 
		return getQualifiedCPPIdentifier();
	}
	string getCTypeName() const { 
		return  getQualifiedCIdentifier(getRootScope());
	} 
	string getNSScopedCTypeName() const { 
		return IDL_IMPL_C_NS "::" + getCTypeName(); 
	} 
	string getNSScopedCPPTypeName() const { 
		return getQualifiedCPPIdentifier();
	}

	
  // misc stuff
	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const {
		typespec = getQualifiedCPPIdentifier(getRootScope());
		dcl = id;
	}

  
	bool isType() {
		return true;
	}
};



// IDLEnum --------------------------------------------------------------------
class IDLEnum : public IDLUserDefSimpleType, public IDLUnionDescriminator
{
public:
	typedef std::vector<IDLEnumComponent *> 	ElementsVec;
	typedef ElementsVec::const_iterator 		const_iterator;

private:
	ElementsVec m_elements;
	
public:
	IDLEnum(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLUserDefSimpleType(id,node,parentscope) {		

		for(IDL_tree curitem = IDL_TYPE_ENUM(node).enumerator_list;
			curitem;
			curitem = IDL_LIST(curitem).next) {
			IDLEnumComponent *enc = new IDLEnumComponent(
				IDL_IDENT(IDL_LIST(curitem).data).str,curitem,parentscope);
			ORBITCPP_MEMCHECK(enc)
			m_elements.push_back(enc);
		}
	}

	const_iterator begin() const {
		return m_elements.begin();
	}
	const_iterator end() const {
		return m_elements.end();
	}

  	string getInvalidReturn() const {
		return "return "
			+ (*begin())->getNSScopedCTypeName() + ";";
	}

	virtual string getDefaultValue(set<string> const &labels)const;

};

// IDLArray --------------------------------------------------------------------
class IDLArrayList;

class IDLArray : public IDLUserDefSimpleType
{
friend class IDLArrayList;
	typedef IDLUserDefSimpleType Super;
	typedef std::vector<int> 	Dimensions;
	typedef Dimensions::const_iterator 		const_iterator;

private:
	Dimensions m_dims;
	IDLType const 	&m_elementType;
	
public:
	IDLArray(IDLType const &elementType, string const &id,IDL_tree node,
			 IDLScope *parentscope = NULL)
		: IDLUserDefSimpleType(id,node,parentscope),m_elementType(elementType) {		

		for(IDL_tree curdim = IDL_TYPE_ARRAY(node).size_list;
			curdim;
			curdim = IDL_LIST(curdim).next) {
			m_dims.push_back(IDL_INTEGER(IDL_LIST(curdim).data).value);
		}
	}

	const_iterator begin() const {
		return m_dims.begin();
	}
	const_iterator end() const {
		return m_dims.end();
	}

  	string getInvalidReturn() const {
		return "return NULL;\n";
	}

	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const;
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const;

	bool isVariableLength() const {
		return m_elementType.isVariableLength();
	}

	// struct and exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const;
	
	void writeUnionModifiers(ostream &ostr,Indent &indent, string const &id,
							 string const &discriminatorVal,
							 IDLTypedef const *activeTypedef = NULL) const;
	
	void writeUnionAccessors(ostream &ostr,Indent &indent, string const &id,
							 string const &discriminatorVal,
							 IDLTypedef const *activeTypedef = NULL) const;
	
	// union stuff
	virtual void writeUnionReferents(ostream &ostr,Indent &indent, string const &id,
									 string const &discriminatorVal,
									 IDLTypedef const *activeTypedef = NULL) const;  

	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef=NULL) const;
  
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const;
	
	void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const;
	
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const;

	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const;
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const;
	// see IDLType for doc
	void writeInitCode(ostream &ostr, Indent &indent, string const &ident) const;
	void writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
	void writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
};

class IDLArrayList {
	struct IDLArrayKey {
		string m_type;
		int m_length;
		
		IDLArrayKey(string const& type, int length)
			: m_type(type), m_length(length) {}
		bool operator <(IDLArrayKey const & key) const {
			if( m_length < key.m_length )
				return true;
			else if( m_length == key.m_length ) 
				return (m_type < key.m_type);
			else	
				return false;
		}
	};
	typedef std::vector<int>::const_iterator const_iterator;
	std::multiset<IDLArrayKey> m_arraySet;
public:
	IDLArrayList() {}
	bool doesArrayTypeExist(IDLArray const& array);
	void clear() { m_arraySet.clear(); }
};



// Interface type -------------------------------------------------------------
class IDLInterface : public IDLUserDefScopeType {
public:
	typedef vector<IDLInterface *> 		BaseList;
	BaseList 							m_bases;
	// all bases except the ones stemming from the first direct base
	BaseList 							m_all_mi_bases; 
	BaseList 							m_allbases;

	IDLInterface(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLUserDefScopeType(id,node,parentscope) {
	}

	bool isVariableLength() const {
		return true;
	}
	bool isBaseClass(IDLInterface *iface);

  	virtual string getCPPStub() const {
		//return getCPPIdentifier()+"_stub";
		return getCPPIdentifier();
	}

	string getCPP_ptr() const {
		return getCPPIdentifier()+"_ptr";
	}
	string getCPP_var() const {
		return getCPPIdentifier()+"_var";
	}
	string getCPP_mgr() const {
		return getCPPIdentifier()+"_mgr";
	}
	string getCPP_out() const {
		return getCPPIdentifier()+"_out";
	}
	string getCPP_POA() {
		if (getParentScope() == getRootScope()) return "POA_"+getCPPIdentifier();
		else return getCPPIdentifier();
	}
	string getCPP_tie() {
		return getCPP_POA() + "_tie";
	}
	virtual string getQualifiedCPPStub(IDLScope const *up_to = NULL) const {
		// This is a dodgy hack - fixme!
		string retval;
		if(up_to == NULL)
			retval = IDL_IMPL_STUB_NS + getQualifiedCPPIdentifier();
		else if (up_to == getRootScope())
			retval = IDL_IMPL_NS_ID "::" IDL_IMPL_STUB_NS_ID  + getQualifiedCPPIdentifier();
		else
			g_error("getQualifiedCPP_stub doesnt support an up_to unless it's rootScope");
		return retval;
	}

	string getQualifiedCPP_ptr(IDLScope const *up_to = NULL) const {
		return getQualifiedCPPIdentifier(up_to) + "_ptr";
	}
	string getQualifiedCPP_var(IDLScope const *up_to = NULL) const {
		return getQualifiedCPPIdentifier(up_to) + "_var";
	}
	string getQualifiedCPP_mgr(IDLScope const *up_to = NULL) const {
		return getQualifiedCPPIdentifier(up_to) + "_mgr";
	}
	string getQualifiedCPP_out(IDLScope const *up_to = NULL) const {
		return getQualifiedCPPIdentifier(up_to) + "_out";
	}
	string getQualifiedCPPCast(string const &expr) const {
		return "reinterpret_cast< "+getQualifiedCPP_ptr()+">("+expr+")";
	}

	// cast with ref for smart pointer to normal ptr
	string getQualifiedCPPSmartptrCast(string const &expr) const {
		return "reinterpret_cast< "+getQualifiedCPP_ptr()+"&>("+expr+")";
	}

	bool requiresSmartPtr() const;
  
	string getQualifiedCPP_POA() const {
		return "POA_" + getQualifiedCPPIdentifier(getRootScope());
	}
	string getQualifiedC_POA() const {
		return "POA_" + getQualifiedCIdentifier();
	}
	string getQualifiedC_epv() const {
		return "POA_" + getQualifiedCIdentifier()+"__epv";
	}
	string getQualifiedC_vepv() const {
		return "POA_" + getQualifiedCIdentifier()+"__vepv";
	}
	string getRepositoryId() const {
		return IDL_IDENT_REPO_ID(IDL_INTERFACE(getNode()).ident);
	};

	void getCPPpoaNamespaceDecl(string &ns_begin,string &ns_end) const {
		getParentScope()->getCPPNamespaceDecl(ns_begin,ns_end,"POA_");
	}
	string getCPPpoaIdentifier() const {
		if (getParentScope() == getRootScope()) return "POA_"+getCPPIdentifier();
		else return getCPPIdentifier();
	}

	// misc stuff
	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const {
		typespec = getQualifiedCPP_mgr(getRootScope());
		dcl = id;
	}
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const;

	// struct / exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = getQualifiedCPP_ptr();
		dcl = "_par_"+id;
	}
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const {
		ostr 
			<< indent << id << " = " << getQualifiedCPPCast("_par_"+id)
			<< ';' << endl;
	}
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const;

	// sequence stuff
	string getCTypeName() const {
		return  getQualifiedCIdentifier();
	}
	
	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,
							  string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef = NULL) const;
	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const;

	// stub return stuff
	void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = getQualifiedCPP_ptr();
		dcl = id;
	}
	void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									IDLTypedef const *activeTypedef = NULL) const {
		ostr
			<< indent << getNSScopedCTypeName()
			<< " _retval = NULL;" << endl;
	}
	string getCPPStubReturnAssignment() const {
		return "_retval = ";
	}
	void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const;
	// skel stuff
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const;
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		return "_" + id + "_ptr";
	}
	void writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								 IDLTypedef const *activeTypedef = NULL) const;

	// skel return stuff
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
		typespec = this->getNSScopedCTypeName();
		dcl = id;
	}
	void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									
									IDLTypedef const *activeTypedef = NULL) const {
		if (passthru)
			ostr << indent << getQualifiedCIdentifier() << " _retval;" << endl;
		else
			ostr << indent << getQualifiedCPP_ptr()
				<< " _retval = NULL;" << endl;
	}
	string getCPPSkelReturnAssignment(bool passthru, 
									  IDLTypedef const *activeTypedef = NULL) const {
		return "_retval = ";
	}
	void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
									   IDLTypedef const *activeTypedef = NULL) const;

	string getInvalidReturn() const {
		return "return NULL;";
	}
};



// Sequence type -------------------------------------------------------------
class IDLSequenceList;

class IDLSequence : public IDLType
{
friend class IDLSequenceComp;
private:
	IDLType const 	&m_elementType;
	int 		m_length;
  
public:
	
	IDLSequence(IDLType const &elementType,int length)
		: m_elementType(elementType),m_length(length) {
	}
	string getCPPType() const;
	IDLType const &getElementType() const {
		return m_elementType;
	}

	// misc stuff
	string getCTypeName() const {
		string id = "";
		id = id + "CORBA_sequence_" + m_elementType.getCTypeName();
		return  id;
	}

	string getNSScopedCTypeName() const;
	
	string getNSScopedCPPTypeName() const { 
		return getCPPType();
	}
	
	bool isVariableLength() const;	
	
	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const;
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPSpecCode(ostream &ostr, Indent &indent, IDLCompilerState &state) const;

	// struct / exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const;

	// union stuff
	virtual void writeUnionReferents(ostream &ostr,Indent &indent, string const &id,
									string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const;

	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef=NULL) const;
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
	void writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
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

struct IDLSequenceComp {
	bool operator()(IDLSequence const *s1, IDLSequence const *s2) const { 
		if( s1->m_length < s2->m_length )
			return true;
		else if( s1->m_length == s2->m_length) {
			string ts1, ts2, dcl1 = "", dcl2 = "";
			s1->m_elementType.getCPPMemberDeclarator(dcl1, ts1, dcl1);
			s2->m_elementType.getCPPMemberDeclarator(dcl2, ts2, dcl2);
			return( (ts1 + dcl1) < (ts2 + dcl2) );
		}
		else
			return false;
	}
};

class IDLSequenceList {
	std::multiset<IDLSequence const*, IDLSequenceComp> m_seqSet;

public:
	IDLSequenceList() {}
	bool doesSeqTypeExist(IDLSequence const& seq) {
		if( m_seqSet.find(&seq) == m_seqSet.end() ) {
			m_seqSet.insert(&seq);
			return false;
		}
		else
			return true;
	}
	void clear() { m_seqSet.clear(); }
};

// Structured types -----------------------------------------------------------
class IDLCompound : public IDLScope {

public:
	IDLCompound(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLScope(id,node,parentscope) {}

	void writeCPackingCode(ostream &header,Indent &indent,ostream &module,Indent &mod_indent);
};




class IDLException : public IDLCompound {
public:
	IDLException(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLCompound(id,node,parentscope) {
	}
	bool isType() {
		// this is required for type container generation
		return true;
	}
	string getRepositoryId() {
		return IDL_IDENT_REPO_ID(IDL_EXCEPT_DCL(getNode()).ident);
	};
};




class IDLStruct : public IDLCompound,public IDLType {
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
		return IDL_IMPL_C_NS "::" + getCTypeName();
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

// This class derives from IDLStruct because all the parameter passing stuff
// is the same. Yes I know this is a hack.
class IDLUnion : public IDLStruct //public IDLCompound //,public IDLType
{
private:
	IDLType const 	&m_discriminatorType;
public:
	IDLUnion(string const &id,IDL_tree node,
			 IDLType const &discriminatorType, IDLScope *parentscope = NULL);
	IDLType const &getDiscriminatorType() const { return m_discriminatorType;}
	virtual bool isVariableLength() const;

	// returns true if the idlunion has an explicit default clause
	bool hasExplicitDefault() const;
	string getDefaultDiscriminatorValue() const;

	void writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
	void writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
};


// Type parser ----------------------------------------------------------------
class IDLTypeParser {
protected:
	IDLCompilerState &m_state;
	vector<IDLType *> 	m_anonymous_types;

public:
	IDLTypeParser(IDLCompilerState & state)
		: m_state(state) {}
	~IDLTypeParser();
	IDLType *parseTypeSpec(IDLScope &scope,IDL_tree typespec);
	IDLType *parseDcl(IDL_tree dcl,IDLType *typespec,string &id);
};




#endif
