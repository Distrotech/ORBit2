/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#include "types/IDLEnum.hh"
#include "types/IDLStruct.hh"
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
	void doTypedef(IDL_tree node,IDLScope &scope);
	
	void doStruct(IDL_tree node,IDLScope &scope);
#if 0 //!!!
	void doUnion(IDL_tree node,IDLScope &scope);
#endif
	void doEnum(IDL_tree node,IDLScope &scope);
	void doNative(IDL_tree node,IDLScope &scope);
	void doConstant(IDL_tree node,IDLScope &scope);

	void doInterface(IDL_tree node,IDLScope &scope);
	void doMember(IDLMember &member);
	void doAttribute(IDL_tree node,IDLScope &scope);
	void doOperation(IDL_tree node,IDLScope &scope);
	void doException(IDL_tree node,IDLScope &scope);

	void doModule(IDL_tree node,IDLScope &scope);

	void doInterfaceBase(IDLInterface &iface);
 
	void doInterfaceStaticMethodDeclarations(IDLInterface &iface);
	void doInterfaceStaticMethodDefinitions(IDLInterface &iface);
	void enumHook(IDL_tree list,IDLScope &scope);

	void struct_create_members    (const IDLStruct &strct);
	void struct_create_converters (const IDLStruct &strct);
	void struct_create_typedefs   (const IDLStruct &strct);
	void struct_create_any        (const IDLStruct &strct);

	void exception_create_members      (const IDLException &ex);
	void exception_create_constructors (const IDLException &ex);
	void exception_create_converters   (const IDLException &ex);
	void exception_create_any          (const IDLException &ex);
};

class IDLWriteCPPTraits: public IDLOutputPass::IDLOutputJob
{
	const IDLElement &m_element;
public:
	IDLWriteCPPTraits (const IDLElement &element,
			   IDLCompilerState &state,
			   IDLOutputPass    &pass);
	void run();
};

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
	IDLWriteAnyFuncs (IDLCompilerState &state,
			  IDLOutputPass    &pass);

	void writeAnyFuncs (bool          pass_value,
			    const string &cpptype,
			    const string &ctype);
public:
	enum FuncType {
		FUNC_VALUE,
		FUNC_COPY,
		FUNC_NOCOPY
	};
	
	static void writeInsertFunc (ostream      &ostr,
				     Indent       &indent,
				     FuncType      func,
				     string        ident,
				     const string &ctype);
	
	static void writeExtractFunc (ostream      &ostr,
				      Indent       &indent,
				      FuncType      func,
				      string        ident,
				      const string &ctype);
	
	void run() = 0;
};

class IDLWriteEnumAnyFuncs : public IDLWriteAnyFuncs
{
	IDLEnum const& m_enum;
public:
	IDLWriteEnumAnyFuncs (const IDLEnum    &_enum,
			      IDLCompilerState &state,
			      IDLOutputPass    &pass);
	void run();
};

class IDLWriteStructAnyFuncs : public IDLWriteAnyFuncs
{
	IDLElement const & m_element;
public:
	IDLWriteStructAnyFuncs (const IDLStruct  &_struct,
				IDLCompilerState &state,
				IDLOutputPass    &pass);
#if 0
	IDLWriteStructAnyFuncs (const IDLUnion   &_union,
				IDLCompilerState &state,
				IDLOutputPass    &pass);
#endif

	void run();
};


#if 0 //!!!
typedef IDLWriteUnionAnyFuncs IDLWriteStructAnyFuncs;
#endif

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
	IDLWriteIfaceAnyFuncs (const IDLInterface &iface,
			       IDLCompilerState   &state,
			       IDLOutputPass      &pass);
	void run();
};

class IDLWriteArrayAnyFuncs : public IDLWriteAnyFuncs
{
	IDLArray const &m_array;
	IDLElement const &m_dest;
public:
	IDLWriteArrayAnyFuncs (const IDLArray   &array,
			       const IDLElement &dest, 
			       IDLCompilerState &state,
			       IDLOutputPass    &pass);
	void run();
};

#endif
