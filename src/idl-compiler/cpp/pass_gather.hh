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
 *  Purpose:	information gathering pass
 *
 */




#ifndef ORBITCPP_PASS_GATHER
#define ORBITCPP_PASS_GATHER




#include <libIDL/IDL.h>
#include "pass.hh"
#include "pass_idl_it.hh"




class IDLPassGather : public IDLIteratingPass,public IDLPass {
protected:
	typedef IDLIteratingPass	Super;

public:
	IDLPassGather(IDLCompilerState &state)
		: IDLPass(state) {
		state.m_pass_gather = this;
	}

	void runPass();

protected:
	void doTypedef(IDL_tree node,IDLScope &scope);
#if 0 //!!!
	void doStruct(IDL_tree node,IDLScope &scope);
	void doUnion(IDL_tree node,IDLScope &scope);
	void doNative(IDL_tree node,IDLScope &scope);
    
#endif //!!!
	void doEnum(IDL_tree node,IDLScope &scope);
	void doConstant(IDL_tree node,IDLScope &scope);
	void doOperation(IDL_tree node,IDLScope &scope);
	void doAttribute(IDL_tree node,IDLScope &scope);
	void doMember(IDL_tree node,IDLScope &scope);
	void doCaseStmt(IDL_tree node,IDLScope &scope);

	void doException(IDL_tree node,IDLScope &scope);
	void doInterface(IDL_tree node,IDLScope &scope);
	void doForwardDcl(IDL_tree node,IDLScope &scope);
	void doModule(IDL_tree node,IDLScope &scope);

	void enumerateBases(IDLInterface &iface,bool recurse,IDLInterface *omit_with_bases,
	                    vector<IDLInterface *> &dest);
};




#endif
