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

#include "IDLElement.hh"

#include "IDLScope.hh"

#include <algorithm>

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

IDLElement::~IDLElement()
{
}

string
IDLElement::get_idl_identifier () const
{
    return m_identifier;
}

string
IDLElement::get_c_identifier () const
{
    return m_identifier;
}

string
IDLElement::get_cpp_identifier () const
{
	if (m_cppkeyword)
		return IDL_CPP_KEY_PREFIX+m_identifier;
	return
		m_identifier;
}

string
IDLElement::get_idl_typename () const
{
	string retval = get_idl_identifier ();
	
	for (const IDLScope *curr_scope = getParentScope ();
	     curr_scope; curr_scope = curr_scope->getParentScope ())
	{
		retval = curr_scope->get_idl_identifier () + "::" + retval;
	}
	
	return retval;
}

string
IDLElement::get_c_typename () const
{
	string retval = get_c_identifier ();

	for (const IDLScope *curr_scope = getParentScope ();
	     curr_scope; curr_scope = curr_scope->getParentScope ())
	{
		retval = curr_scope->get_c_identifier () + "_" + retval;
	}

	if (retval[0] == '_')
		retval.erase (0, 1);

	return retval;
}

string
IDLElement::get_cpp_typename () const
{
	string retval = get_cpp_identifier ();
	
	for (const IDLScope *curr_scope = getParentScope ();
	     curr_scope; curr_scope = curr_scope->getParentScope ())
	{
		retval = curr_scope->get_cpp_identifier () + "::" + retval;
	}
	
	return retval;
}

string
IDLElement::get_cpp_method_prefix () const
{
	string retval = get_cpp_typename ();
	
	// Remove :: from head
	string::iterator i = retval.begin ();
	while (i != retval.end () && *i == ':')
		i = retval.erase (i);

	return retval;
}

string
IDLElement::get_c_method_prefix () const
{
    return get_c_typename ();
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




IDLScope const *
IDLElement::getTopLevelInterface() const {
	IDLScope const *run = getParentScope();
	IDLScope const *tli = NULL;
	
	while (run) {
		if (run->isInterface()) tli = run;
		run = run->getParentScope();
	}
	return tli;
}
