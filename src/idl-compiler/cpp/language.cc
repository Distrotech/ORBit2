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
 *  Purpose:	IDL compiler language representation
 *
 */




#include <cstdio>
#include <cstring>
#include "language.hh"
#include "error.hh"
#include "types.hh"
#include <algorithm>
#include <set>



// constant xlator ------------------------------------------------------------
static char *idlNumChars = "0123456789abcdef";




static string idlInt2String(IDL_longlong_t value,char radix = 10)
{
	bool neg = value < 0;
	if (neg) value = -value;
	string temp;
	do {
		temp = idlNumChars[value % radix]+temp;
		value /= radix;
	} while (value);
	if (neg) temp = '-'+temp;
	return temp;
}




string idlTranslateConstant(IDL_tree const constant,IDLScope const &scope) {
	char buffer[1<<8];
	IDLElement *cns;
	switch (IDL_NODE_TYPE(constant)) {
		case IDLN_INTEGER:
			return idlInt2String(IDL_INTEGER(constant).value);
		case IDLN_STRING:
			return string("\"")+IDL_STRING(constant).value+'"';
		case IDLN_WIDE_STRING:
			// *** FIXME implement this
			ORBITCPP_NYI("wide string constant");
			return "";
		case IDLN_CHAR:
			return '\''+string(IDL_CHAR(constant).value,1)+'\'';
		case IDLN_WIDE_CHAR:
			// *** FIXME implement this
			ORBITCPP_NYI("wide char constant")
			return "";
		case IDLN_FIXED:
			// *** FIXME implement this
			ORBITCPP_NYI("fixed constant")
			return "";
		case IDLN_FLOAT:
			sprintf(buffer,"%f",IDL_FLOAT(constant).value);
			return buffer;
		case IDLN_BOOLEAN:
			if (IDL_BOOLEAN(constant).value) return "1";
			else return "0";
		case IDLN_IDENT:
			cns = scope.lookup(idlGetQualIdentifier(constant));
			if (!cns) throw IDLExUnknownIdentifier(constant,idlGetQualIdentifier(constant));
			return cns->getQualifiedCPPIdentifier();
		case IDLN_UNARYOP: {
			char op = ' ';
			switch (IDL_UNARYOP(constant).op) {
				case IDL_UNARYOP_PLUS: op = '+'; break;
				case IDL_UNARYOP_MINUS: op = '-'; break;
				case IDL_UNARYOP_COMPLEMENT: op = '~'; break;
			}
			return string("(") + op + idlTranslateConstant(IDL_UNARYOP(constant).operand,scope) + ")";
		}
		case IDLN_BINOP: {
			string op;
			switch (IDL_BINOP(constant).op) {
				case IDL_BINOP_OR: op = "|"; break;
				case IDL_BINOP_XOR: op = "^"; break;
				case IDL_BINOP_AND: op = "&"; break;
				case IDL_BINOP_SHR: op = ">>"; break;
				case IDL_BINOP_SHL: op = "<<"; break;
				case IDL_BINOP_ADD: op = "+"; break;
				case IDL_BINOP_SUB: op = "-"; break;
				case IDL_BINOP_MULT: op = "*"; break;
				case IDL_BINOP_DIV: op = "/"; break;
				case IDL_BINOP_MOD: op = "%"; 
			}
			return '(' + idlTranslateConstant(IDL_BINOP(constant).left,scope) +
				op + idlTranslateConstant(IDL_BINOP(constant).right,scope) + ')';
		}
		default:
			ORBITCPP_NYI("parsing "+idlGetNodeTypeString(constant)+" as a constant")
	}
}




// IDLElement -----------------------------------------------------------------
IDLElement::IDLElement(string const &id,IDL_tree node,IDLScope *parentscope)
	: m_identifier(id),m_node(node),m_parentscope(parentscope) {

	m_cppkeyword = idlIsCPPKeyword(id);

	if (m_parentscope) {

		IDLElement *slot = parentscope->getItem(id);

		// I've removed the following check, since forward dcls mean that
		// there can be duplicate identifiers -PD  
		// okay, libIDL should catch all real evil cases for us anyway. -andy
		//if (slot) throw IDLExDuplicateIdentifier(node,*parentscope,m_identifier);

		if (slot && isInterface()) {
			// replace the old interface (forward dcl) with this new one
			// (which will have more info)
			std::replace(parentscope->begin(),parentscope->end(),slot,this);
		}
		else
			m_parentscope->m_items.push_back(this);
	}
}




string 
IDLElement::getQualifiedIDLIdentifier(IDLScope const *up_to,
        IDLScope const *assumed_base = NULL) const {
	if (up_to == this) return "";
	IDLScope const *run = assumed_base ? assumed_base : getParentScope();

	string result = getIDLIdentifier();

	while (run != up_to) {
		result.insert(0,run->getIDLIdentifier() + "::");
		run = run->getParentScope();
	}
	return result;
}




string 
IDLElement::getQualifiedCIdentifier(IDLScope const *up_to,
        IDLScope const *assumed_base = NULL) const {
	if (up_to == this) return "";
	IDLScope const *run = assumed_base ? assumed_base : getParentScope();

	string result = getCIdentifier();

	while (run != up_to) {
		result.insert(0,run->getCIdentifier() + "_");
		run = run->getParentScope();
	}
	if (result.size() && result.substr(0,strlen("_")) == "_")
		result.erase(0,1);
	return result;
}




string 
IDLElement::getQualifiedCPPIdentifier(IDLScope const *up_to,
        IDLScope const *assumed_base = NULL) const {
	if (up_to == this) return "";
	IDLScope const *run = assumed_base ? assumed_base : getParentScope();

	string result = getCPPIdentifier();

	while (run != up_to) {
		result.insert(0,run->getCPPIdentifier() + "::");
		run = run->getParentScope();
	}
	return result;
}




IDLScope const *
IDLElement::getRootScope() const {
	IDLScope const *run = getParentScope();
	IDLScope const *tug = getParentScope();

	while (run) {
		tug = run;
		run = run->getParentScope();
	}
	return tug;
}




IDLScope const *IDLElement::getTopLevelInterface() const {
	IDLScope const *run = getParentScope();
	IDLScope const *tli = NULL;
	
	while (run) {
		if (run->isInterface()) tli = run;
		run = run->getParentScope();
	}
	return tli;
}




// IDLScope -------------------------------------------------------------------
IDLScope::~IDLScope() {
	ItemList::iterator first = m_items.begin(), last = m_items.end();

	while (first != last)
		delete *first++;
}




IDLElement *
IDLScope::lookup(string const &id) const {
	const IDLScope *scope = this;
	string::size_type first = 0;

	if (id.substr(0,2) == "::") {
		scope = getRootScope();
		first += 2;
	}

	while (scope) {
		IDLElement *item = scope->lookupLocal(id.substr(first));
		if (item) return item;
		scope = scope->getParentScope();
	}
	return NULL;
}




IDLElement *
IDLScope::lookupLocal(string const &id) const {
	const IDLScope *scope = this;
	string::size_type first = 0;

	string::size_type nextscopeq = id.find("::",first);
	if (nextscopeq == string::npos) {
		return scope->getItem(id.substr(first));
	} else {
		int spos = 0;
		IDLScope *nextscope = NULL;
		while ((nextscope = scope->getScope(id.substr(first,nextscopeq-first),spos)) != NULL) {
			spos++;
			IDLElement* element = nextscope->lookupLocal(id.substr(nextscopeq+2,id.length()-nextscopeq-2));
			if (element != NULL) return element;
		}
	}

	return NULL;
}




IDLElement *
IDLScope::getItem(IDL_tree node) const {
	ItemList::const_iterator first = m_items.begin(),last = m_items.end();

	while (first != last) {
		if (node == (*first)->getNode()) return *first;
		first++;
	}
	throw IDLExInternal();
}




IDLElement *
IDLScope::getItem(string const &id) const {
	ItemList::const_iterator first = m_items.begin(),last = m_items.end();

	while (first != last) {
		if ((*first)->getIDLIdentifier() == id) return *first;
		first++;
	}
	return NULL;
}




IDLScope *
IDLScope::getScope (string const &id, int &spos) const {
	ScopeList::const_iterator first = m_scopes.begin(),last = m_scopes.end();

	int pos_counter = 0;
	while (first != last) {
		if ((*first)->getIDLIdentifier() == id && pos_counter >= spos) {
			spos = pos_counter;
			return *first;
		}

		first++;
		pos_counter++;
	}

	return NULL;
}




bool IDLScope::hasTypeChildren() const {
	const_iterator first = begin(), last = end();
	
	while (first != last) {
		if ((*first)->isType()) return true;
		first++;
	}
	return false;
}




IDLScope const *IDLScope::getTopLevelInterface() const {
	IDLScope const *run = this;
	IDLScope const *tli = NULL;
	
	while (run) {
		if (run->isInterface()) tli = run;
		run = run->getParentScope();
	}
	return tli;
}




void 
IDLScope::getCPPNamespaceDecl(string &ns_begin,string &ns_end,
                                    string const &prefix) {
	IDLScope const *scope = this;
	IDLScope const *rootscope = getRootScope();

	while (scope != rootscope) {
		IDLScope const *nextscope = scope->getParentScope();
		string id = scope->getCPPIdentifier();
		if (nextscope == rootscope)
			id.insert(0,prefix);
		ns_begin.insert(0,"namespace " + id + "\n{\n");
		ns_end += "} //namespace " + id +"\n\n";
		scope = nextscope;
	}
}



// IDLCaseStmt ---------------------------------------------------------------
IDLCaseStmt::IDLCaseStmt(IDLMember *member, string const &id,
						 IDL_tree node,IDLScope *parentscope = NULL)
	: IDLElement(id,node,parentscope), m_member(member),m_isDefault(false) {

	// labels
	g_assert(IDL_NODE_TYPE(node) == IDLN_CASE_STMT);
	IDL_tree list = IDL_CASE_STMT(node).labels;
	g_assert(IDL_NODE_TYPE(list) == IDLN_LIST);
	while (list) {
		IDL_tree label = IDL_LIST(list).data;
		if (label==NULL){
			m_isDefault=true;
			break;
		}
		m_labels.push_back(idlTranslateConstant(label,*parentscope));
		list = IDL_LIST(list).next;
	}	
}
 
// IDLOperation ---------------------------------------------------------------
string 
IDLOperation::getCPPOpParameterList() {
	string result;
	vector<ParameterInfo>::const_iterator
	first = m_parameterinfo.begin(),last = m_parameterinfo.end();

	while (first != last) {
		string typespec,dcl;
		first->Type->getCPPStubDeclarator(first->Direction,first->Identifier,typespec,dcl);

		result += typespec + ' ' + dcl;
		first++;
		if (first != last) result += ',';
	}
	return result;
}




string 
IDLOperation::getCOpParameterList() {
	string result;
	vector<ParameterInfo>::const_iterator
	first = m_parameterinfo.begin(),last = m_parameterinfo.end();

	while (first != last) {
		string typespec,dcl;
		first->Type->getCSkelDeclarator(first->Direction,first->Identifier,typespec,dcl);

		result += typespec + ' ' + dcl;
		first++;
		if (first != last) result += ',';
	}
	return result;
}




