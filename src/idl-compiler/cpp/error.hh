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
 *  Purpose:	IDL compiler error declarations
 *
 */




#ifndef ORBITCPP_ERROR
#define ORBITCPP_ERROR


#include "base.hh"
#include "language.hh"
#include <libIDL/IDL.h>
#include <stdexcept>
#include <string>




// Useful macros --------------------------------------------------------------
#define ORBITCPP_EXPECT_TYPE(node,type) \
	if (IDL_NODE_TYPE(node) != type) \
	throw IDLExNodeType(node,type);
#define ORBITCPP_MEMCHECK(item) \
	if (!item) throw IDLExMemory();
#define ORBITCPP_NYI(what) \
	throw IDLExNotYetImplemented(what);
#define ORBITCPP_DEFAULT_CASE(node) \
	default: \
	throw IDLExUnexpectedNodeType(node);




// Exception types ------------------------------------------------------------
struct IDLBaseException : public runtime_error {
	IDLBaseException(const string &what_arg)
		: runtime_error(what_arg) {
	}
};




struct IDLExNode : public IDLBaseException {
	IDLExNode(IDL_tree node,const string &what_arg)
		: IDLBaseException(idlGetNodeLocation(node) + ": " + what_arg) {
	}

};




struct IDLExNodeType : public IDLExNode {
	IDLExNodeType(IDL_tree node,IDL_tree_type expected)
		: IDLExNode(node,idlGetTypeString(expected)+" expected, "+
		  idlGetNodeTypeString(node)+" encountered") {
	}
};




struct IDLExUnexpectedNodeType : public IDLExNode {
	IDLExUnexpectedNodeType(IDL_tree node)
		: IDLExNode(node,idlGetNodeTypeString(node)+" not expected") {
	}
};




struct IDLExDuplicateIdentifier : public IDLExNode {
	IDLExDuplicateIdentifier(IDL_tree node,IDLScope const &scope,string const &id)
		: IDLExNode(node,"duplicate identifier in "+
	      scope.getQualifiedIDLIdentifier()+": "+id) {
	}
};




struct IDLExUnknownIdentifier : public IDLExNode {
	IDLExUnknownIdentifier(IDL_tree node,string const &id)
		: IDLExNode(node,"unknown identifier "+id) {
	}
};




struct IDLExTypeIdentifierExpected : public IDLExNode {
	IDLExTypeIdentifierExpected(IDL_tree node,string const &id)
		: IDLExNode(node,"not a type identifier: "+id) {
	}
};




struct IDLExNoConstantOfThisType : public IDLBaseException {
	IDLExNoConstantOfThisType(string type)
		: IDLBaseException(type + " cannot be used as a constant type") {
	}
};




struct IDLExVoid : public IDLBaseException {
	IDLExVoid()
		: IDLBaseException("void can only be used as a function return type") {
	}
};




struct IDLExMemory : public IDLBaseException {
	IDLExMemory()
		: IDLBaseException("insufficient memory") {
	}
};




struct IDLExInternal : public IDLBaseException {
	IDLExInternal()
		: IDLBaseException("internal error") {
	}
};




struct IDLExNotYetImplemented : public IDLBaseException {
	IDLExNotYetImplemented(string const &what)
		: IDLBaseException("not yet implemented: "+what) {
	}
};




#endif
