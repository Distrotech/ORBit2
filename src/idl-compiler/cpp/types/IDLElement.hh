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

#ifndef ORBITCPP_TYPES_IDLELEMENT
#define ORBITCPP_TYPES_IDLELEMENT

#include <libIDL/IDL.h>
#include "base.hh"
#include <string>

class IDLScope;

class IDLElement {
protected:
	string			m_identifier;
	IDL_tree		m_node;
	IDLScope		*m_parentscope;
	bool			m_cppkeyword;

public:
	IDLElement(string const &id,IDL_tree node,IDLScope *parentscope = NULL);
	virtual ~IDLElement();

	string get_idl_identifier () const;
	string get_c_identifier   () const;
	string get_cpp_identifier () const;
	
	virtual string get_idl_typename () const;
	virtual string get_c_typename   () const;
	virtual string get_cpp_typename () const;
	
	virtual string get_c_method_prefix   () const;
	virtual string get_cpp_method_prefix () const;
	

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

#endif //ORBITCPP_TYPES_IDLELEMENT
