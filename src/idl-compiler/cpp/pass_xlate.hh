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
 *          Phil Dawes <philipd@users.sourceforge.net>
 *
 *  Purpose:	idl -> c++ translation pass
 *
 */




#ifndef ORBITCPP_PASS_XLATE
#define ORBITCPP_PASS_XLATE



#include "pass.hh"
#include "pass_idl_it.hh"
//#include "types/IDLEnum.hh"
//#include "types/IDLStruct.hh"
//#include "types/IDLUnion.hh"
#include "types/IDLException.hh"
#include <libIDL/IDL.h>
#include <iostream>
#include <vector>
#include <set>
#include <string>





class IDLPassXlate : public IDLIteratingPass,public IDLOutputPass {
protected:
	typedef IDLIteratingPass		Super;

public:
	IDLPassXlate(IDLCompilerState &state,ostream &header,ostream &module)
		: IDLOutputPass(state,header,module) {
		state.m_pass_xlate = this;
	}

	void runPass();

protected:
#if 0 //!!!
	void doTypedef(IDL_tree node,IDLScope &scope);
	void doStruct(IDL_tree node,IDLScope &scope);
	void doUnion(IDL_tree node,IDLScope &scope);
#endif
	void doEnum(IDL_tree node,IDLScope &scope);
	void doNative(IDL_tree node,IDLScope &scope);
	void doConstant(IDL_tree node,IDLScope &scope);
#if 0
	void doAttribute(IDL_tree node,IDLScope &scope);
#endif
	void doOperation(IDL_tree node,IDLScope &scope);
	void doMember(IDLMember &member);

	void doException(IDL_tree node,IDLScope &scope);
	void doInterface(IDL_tree node,IDLScope &scope);
	void doModule(IDL_tree node,IDLScope &scope);

	void doInterfaceBase(IDLInterface &iface);
 
	void doInterfaceStaticMethodDeclarations(IDLInterface &iface);
	void doInterfaceStaticMethodDefinitions(IDLInterface &iface);
#if 0 //!!!
	void enumHook(IDL_tree list,IDLScope &scope);
#endif
};

#if 0 //!!!
class IDLWriteCPPSpecCode : public IDLOutputPass::IDLOutputJob {
	IDLType const	&m_type;
public:
	IDLWriteCPPSpecCode(IDLType const &type,IDLCompilerState &state,IDLOutputPass &pass) 
		: IDLOutputJob(IDL_EV_TOPLEVEL,state,pass),m_type(type) {
	}
	void run() {
		m_type.writeCPPSpecCode(m_header, indent, m_state);
	}
};
#endif

#if 0 //!!!
class IDLWriteArrayProps : public IDLOutputPass::IDLOutputJob {
	IDLArray const	&m_array;
	IDLElement const &m_dest;
public:
	IDLWriteArrayProps(IDLArray const &array,IDLElement const &dest,IDLCompilerState &state,
						IDLOutputPass &pass) 
		: IDLOutputJob(IDL_EV_TOPLEVEL,state,pass),m_array(array),m_dest(dest) {
	}
	void run();
};

class IDLWriteAnyFuncs : public IDLOutputPass::IDLOutputJob {
protected:
	void writeAnyFuncs(bool pass_value, string const& cpptype, string const& ctype);
	IDLWriteAnyFuncs(IDLCompilerState& state, IDLOutputPass &pass)
		: IDLOutputJob("",state,pass) {}
public:
	enum FuncType {
		FUNC_VALUE,
		FUNC_COPY,
		FUNC_NOCOPY
	};
	static void writeInsertFunc(ostream&, Indent&, FuncType, string, string const&);
	static void writeExtractFunc(ostream&, Indent&, FuncType, string, string const&);
	void run() = 0;
};

class IDLWriteEnumAnyFuncs : public IDLWriteAnyFuncs
{
	IDLEnum const& m_enum;
public:
	IDLWriteEnumAnyFuncs(IDLEnum const& _enum, IDLCompilerState &state, IDLOutputPass &pass)
		: IDLWriteAnyFuncs(state, pass), m_enum(_enum) {}
	void run() {
		writeAnyFuncs(true, m_enum.getQualifiedCPPIdentifier(),
			m_enum.getQualifiedCIdentifier() );
	}
};

class IDLWriteStructAnyFuncs : public IDLWriteAnyFuncs
{
	IDLElement const & m_element;
public:
	IDLWriteStructAnyFuncs(IDLStruct const& _struct, IDLCompilerState &state, IDLOutputPass &pass)
		: IDLWriteAnyFuncs(state, pass), m_element(_struct) {}
	IDLWriteStructAnyFuncs(IDLUnion const& _union, IDLCompilerState &state, IDLOutputPass &pass)
		: IDLWriteAnyFuncs(state, pass), m_element(_union) {}

	void run() {
		writeAnyFuncs(false, m_element.getQualifiedCPPIdentifier(),
			m_element.getQualifiedCIdentifier() );
	}

};


typedef IDLWriteUnionAnyFuncs IDLWriteStructAnyFuncs;

class IDLWriteExceptionAnyFuncs : public IDLWriteAnyFuncs
{
	IDLElement const & m_element;
public:
	IDLWriteExceptionAnyFuncs(IDLException const& _exception, IDLCompilerState &state, IDLOutputPass &pass)
		: IDLWriteAnyFuncs(state, pass), m_element(_exception) {}

	void run();
};


class IDLWriteIfaceAnyFuncs : public IDLWriteAnyFuncs
{
	IDLInterface const &m_iface;
public:
	IDLWriteIfaceAnyFuncs(IDLInterface const& _iface, IDLCompilerState &state, IDLOutputPass &pass)
		: IDLWriteAnyFuncs(state, pass), m_iface(_iface) {}
	void run() {
		string cpptype = m_iface.getQualifiedCPPIdentifier()+"_ptr";
		string ctype = m_iface.getQualifiedCIdentifier();
		writeInsertFunc(m_header, indent, FUNC_NOCOPY, cpptype, ctype);
		writeAnyFuncs(true, cpptype, ctype );
	}
};

class IDLWriteArrayAnyFuncs : public IDLWriteAnyFuncs
{
	IDLArray const &m_array;
	IDLElement const &m_dest;
public:
	IDLWriteArrayAnyFuncs(IDLArray const &_array, IDLElement const &_dest, 
						IDLCompilerState &state, IDLOutputPass &pass)
		: IDLWriteAnyFuncs(state, pass), m_array(_array), m_dest(_dest) {}
	void run();
};
#endif

#endif
