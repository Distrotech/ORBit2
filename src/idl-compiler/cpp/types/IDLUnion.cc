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

#include "IDLUnion.hh"

IDLUnion::IDLUnion(string const &id,IDL_tree node,
				   IDLType const &discriminatorType, IDLScope *parentscope = NULL)
	: IDLStruct(id,node,parentscope),m_discriminatorType(discriminatorType) {
}

bool
IDLUnion::isVariableLength() const {
	IDLUnion::const_iterator first = begin(), last = end();
	while (first != last) {
		IDLCaseStmt &stmt = (IDLCaseStmt &) **first++;
		const IDLMember &member = stmt.getMember();
		if(member.getType()->isVariableLength()){
			return true;
		}
	}
	return false;
}


string
IDLUnion::getDefaultDiscriminatorValue() const {
	IDLUnionDescriminator const &desc =
		dynamic_cast<IDLUnionDescriminator const &>(getDiscriminatorType());
	set<string> members;

	// collect all the union labels
	const_iterator it = begin();
	while (it != end()) {
		IDLCaseStmt &casestmt = (IDLCaseStmt &) **it;
		IDLCaseStmt::const_iterator csit = casestmt.labelsBegin(),
			csend = casestmt.labelsEnd();
		while (csit != csend) {
			members.insert(*csit);
			csit++;
		}
		it++;
	}
	return desc.getDefaultValue(members);
}

bool
IDLUnion::hasExplicitDefault() const {
	bool result = false;
	const_iterator it = begin();
	while (it != end()) {
		IDLCaseStmt &casestmt = (IDLCaseStmt &) **it;
		if(casestmt.isDefault()){
			result = true;
			break;
		}
		it++;
	}
	return result;
}


void IDLUnion::writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {

	ostr << indent <<  "switch(" << target << "._d()) {" << endl;
	const_iterator it = begin();
	while (it != end()) {
		// collect the case labels for this member
		IDLCaseStmt &casestmt = (IDLCaseStmt &) **it;
		IDLCaseStmt::const_iterator csit = casestmt.labelsBegin(),
			csend = casestmt.labelsEnd();
		if(casestmt.isDefault() == true) {
			ostr << indent << "default:" << endl;
		} else {
			while (csit != csend) {
				ostr << indent << "case " << *csit << ":" << endl;
				csit++;
			}
		}
		indent++;		
		IDLMember const &member = casestmt.getMember();
		ostr << indent << ident << "." << member.getCPPIdentifier()
			 << "(" << target << "." << member.getCPPIdentifier() << "());" << endl;
		ostr << indent << "break;" << endl;
		it++;
		indent--;
	}
	ostr << indent << "}" << endl << endl;
}

void IDLUnion::writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {

	ostr << indent <<  "switch(" << target << "._d) {" << endl;
	const_iterator it = begin();
	while (it != end()) {
		// collect the case labels for this member
		IDLCaseStmt &casestmt = (IDLCaseStmt &) **it;
		IDLCaseStmt::const_iterator csit = casestmt.labelsBegin(),
			csend = casestmt.labelsEnd();

		if(casestmt.isDefault() == true) {
			ostr << indent << "default:" << endl;
		} else {
			while (csit != csend) {
				ostr << indent << "case " << *csit << ":" << endl;
				csit++;
			}
		}
		
		indent++;		
		IDLMember const &member = casestmt.getMember();
		IDLType const *elemtype = member.getType();
		elemtype->writeCDeepCopyCode(ostr,indent,ident+"._u."+member.getCIdentifier(),target+"._u."+member.getCIdentifier());	
		ostr << indent << "break;" << endl;
		it++;
		indent--;
	}

	if(hasExplicitDefault() == false) {
		ostr << indent++ << "default:" << endl;
		ostr << indent-- << "break;" << endl;
	}
	
	ostr << indent << "}" << endl << endl;
	ostr << indent << ident << "._d = " << target << "._d;" << endl;
}

