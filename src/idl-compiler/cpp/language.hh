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




#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <libIDL/IDL.h>
#include "base.hh"




// forward --------------------------------------------------------------------
class IDLType;
class IDLScope;




// tool functions -------------------------------------------------------------
string idlTranslateConstant(IDL_tree const constant,IDLScope const &scope);




// items and scoping ----------------------------------------------------------
class IDLElement {
protected:
	string			m_identifier;
	IDL_tree		m_node;
	IDLScope		*m_parentscope;
	bool			m_cppkeyword;

public:
	IDLElement(string const &id,IDL_tree node,IDLScope *parentscope = NULL);
	virtual ~IDLElement() {
	}

	string const &getIDLIdentifier() const {
		return m_identifier;
	}
	string const &getCIdentifier() const {
		return m_identifier;
	}
	string getCPPIdentifier() const {
		if (m_cppkeyword) return IDL_CPP_KEY_PREFIX+m_identifier;
		else return m_identifier;
	}
	virtual string getQualifiedIDLIdentifier(IDLScope const *up_to = NULL,
											 IDLScope const *assumed_base = NULL) const;
	virtual string getQualifiedCIdentifier(IDLScope const *up_to = NULL,
										   IDLScope const *assumed_base = NULL) const;
	virtual string getQualifiedCPPIdentifier(IDLScope const *up_to = NULL,
											 IDLScope const *assumed_base = NULL) const;
	IDL_tree getNode() const {
		return m_node;
	}
	IDLScope *getParentScope() const {
		return m_parentscope;
	}
	virtual IDLScope const *getRootScope() const;
	virtual bool isType() {
		return false;
	}
	bool isInterface() const {
		return IDL_NODE_TYPE(getNode()) == IDLN_INTERFACE;
	}
	virtual IDLScope const *getTopLevelInterface() const;
};




//struct IDLScope : public IDLElement {
class IDLScope : public IDLElement {
protected:
	typedef IDLElement					Super;
	typedef vector<IDLElement *> 		ItemList;
	typedef vector<IDLScope *>			ScopeList;
	ItemList							m_items;
	ScopeList							m_scopes;

public:
	typedef ItemList::iterator			iterator;
	typedef ItemList::const_iterator	const_iterator;
	

	IDLScope(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLElement(id,node,parentscope) {
		if (parentscope) parentscope->m_scopes.push_back(this);
	}
	~IDLScope();
	IDLElement *lookup(string const &id) const;
	IDLElement *lookupLocal(string const &id) const;

	IDLElement *getItem(IDL_tree node) const;
	IDLElement *getItem(string const &id) const;
	IDLScope *getScope (string const &id, int &spos) const;
	IDLScope const *getRootScope() const {
		if (getParentScope()) return Super::getRootScope();
		else return this;
	}
	bool hasTypeChildren() const;
	IDLScope const *getTopLevelInterface() const;

	iterator begin() {
		return m_items.begin();
	}
	const_iterator begin() const {
		return m_items.begin();
	}
	iterator end() {
		return m_items.end();
	}
	const_iterator end() const {
		return m_items.end();
	}
	unsigned int size() const {
		return m_items.size();
	}

	void getCPPNamespaceDecl(string &ns_begin,string &ns_end,string const &prefix = "");

	friend class IDLElement;
};




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
		return IDL_IMPL_C_NS_NOTUSED + getQualifiedCIdentifier();
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
	IDLCaseStmt(IDLMember *member, string const &id,
				IDL_tree node,IDLScope *parentscope = NULL);

	~IDLCaseStmt() {
		delete m_member;
	}
  
	IDLMember const &getMember() {
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




class IDLOperation : public IDLElement {
public:
	struct ParameterInfo {
		IDL_param_attr	Direction;
		IDLType 		*Type;
		string			Identifier;
	};

	typedef vector<ParameterInfo>	ParameterList;
	typedef vector<IDLException *>	ExceptionList;

	ParameterList			m_parameterinfo;
	ExceptionList			m_raises;

	IDLType 				*m_returntype;

	IDLOperation(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLElement(id,node,parentscope) {
	}

	string getCPPOpParameterList();
	string getCOpParameterList();
};


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
// union descriminators
class IDLUnionDescriminator {
public:
	// retns a default value, given a set of values used to
	// descriminate members of the union
	virtual string getDefaultValue(set<string> const &labels)const =0;
};

#endif
