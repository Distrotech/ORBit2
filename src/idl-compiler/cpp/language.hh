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
 *  Purpose: IDL compiler language representation
 *
 */




#ifndef ORBITCPP_LANGUAGE
#define ORBITCPP_LANGUAGE



#include <libIDL/IDL.h>
#include "base.hh"
#include <vector>
#include <list>
#include <map>
#include <set>




// forward --------------------------------------------------------------------
class IDLType;
class IDLScope;




// tool functions -------------------------------------------------------------
string idlTranslateConstant(IDL_tree const constant,IDLScope const &scope);




#include "types/IDLElement.hh"
#include "types/IDLScope.hh"

class IDLConstant : public IDLElement {
protected:
	IDLType	*m_type;
public:
	IDLConstant(IDLType *type,string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLElement(id,node,parentscope),m_type(type) { }
	string getValue() const {
		return idlTranslateConstant(IDL_CONST_DCL(getNode()).const_exp,*getParentScope());
	}
	IDLType *getType() {
		return m_type;
	}
};




class IDLMember : public IDLElement {
protected:
	IDLType	*m_type;
public:
	IDLMember(IDLType *type,string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLElement(id,node,parentscope),m_type(type) {
	}
	IDLType const *getType() const {
		return m_type;
	}
};




class IDLEnumComponent : public IDLElement {
public:
	IDLEnumComponent(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLElement(id,node,parentscope) {
	}
	string getNSScopedCTypeName() const {
		return IDL_IMPL_C_NS_NOTUSED + get_c_typename ();
	}
};


class IDLCaseStmt : public IDLElement {
public:
	typedef std::list<std::string>  LabelList;
	typedef LabelList::const_iterator  const_iterator;
private:
	IDLMember *m_member;  
	LabelList m_labels;
	bool m_isDefault;
public:
	// takes ownership of member
	IDLCaseStmt (IDLMember    *member,
		     const string &id,
		     IDL_tree      node,
		     IDLScope     *parentscope = 0);

	~IDLCaseStmt() {
		delete m_member;
	}
  
	const IDLMember& get_member () const {
		return *m_member;
	}
	const_iterator labelsBegin() const {
		return m_labels.begin();
	}
	const_iterator labelsEnd() const {
		return m_labels.end();
	}
	bool isDefault() const {
		return m_isDefault;
	}
};



class IDLException;




class IDLAttribute : public IDLElement {

	IDLType 		*m_type;
	bool            m_readOnly;
  
public:

	IDLAttribute(string const &id,IDL_tree node,IDLType *type,IDLScope *parentscope = NULL)
	  : IDLElement(id,node,parentscope) {
		m_type = type;
		m_readOnly = IDL_ATTR_DCL(node).f_readonly;
	}

	IDLType *getType() {return m_type;}
	bool isReadOnly() {return m_readOnly;}
	
};


// An interface implemented by types that can be used as
// union discriminators
class IDLUnionDiscriminator {
public:
#if 0
	// retns a default value, given a set of values used to
	// descriminate members of the union
	virtual string getDefaultValue(set<string> const &labels)const = 0;
#endif
	virtual string discr_get_c_typename () const = 0;
	virtual string discr_get_cpp_typename () const = 0;
};

#endif
