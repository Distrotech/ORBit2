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
 *          Phil Dawes        <philipd@users.sourceforge.net>
 *
 *  Purpose:	IDL compiler generic IDL iterating pass
 *
 */




#ifndef ORBITCPP_PASS_IDL_IT
#define ORBITCPP_PASS_IDL_IT




#include <libIDL/IDL.h>
#include "language.hh"




class IDLIteratingPass {
protected:
	virtual void doTypedef(IDL_tree node,IDLScope &scope) {
	}
	virtual void doEnum(IDL_tree node,IDLScope &scope) {
	}
	virtual void doConstant(IDL_tree node,IDLScope &scope) {
	}
#if 0 //!!!
	virtual void doNative(IDL_tree node,IDLScope &scope) {
	}
#endif
	virtual void doAttribute(IDL_tree node,IDLScope &scope) {
	}
	virtual void doOperation(IDL_tree node,IDLScope &scope) {
	}
	virtual void doMember(IDL_tree node,IDLScope &scope) {
	}
	virtual void doCaseStmt(IDL_tree node,IDLScope &scope) {
	}
#if 0 //!!!
	virtual void doSequence(IDL_tree node, IDLScope &scope){
	}
#endif
	virtual void doForwardDcl(IDL_tree node, IDLScope &scope){
	}

#if 0 //!!!
	virtual void doStruct(IDL_tree node,IDLScope &scope);
#endif
	virtual void doException(IDL_tree node,IDLScope &scope);
	virtual void doInterface(IDL_tree node,IDLScope &scope);
	virtual void doModule(IDL_tree node,IDLScope &scope);
#if 0 //!!!
	virtual void doUnion(IDL_tree node,IDLScope &scope);
#endif

	virtual void doSwitchBody(IDL_tree list,IDLScope &scope);
	virtual void doMemberList(IDL_tree list,IDLScope &scope);
	virtual void doExportList(IDL_tree list,IDLScope &scope);
	virtual void doDefinitionList(IDL_tree list,IDLScope &scope);
	
	virtual void enumHook(IDL_tree next,IDLScope &scope) {
	}
};




#endif
