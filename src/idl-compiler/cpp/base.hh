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
 *  Purpose:	IDL compiler base and tools
 *
 */




#ifndef ORBITCPP_BASE
#define ORBITCPP_BASE




#include <libIDL/IDL.h>
#include <string>
#include <iostream>


using namespace std;



#define IDL_CPP_KEY_PREFIX			"_cxx_"
#define IDL_CORBA_NS				"CORBA"
#define IDL_POA_NS					"PortableServer"
#define IDL_IMPL_NS_ID				"_orbitcpp"
//#define IDL_IMPL_C_NS_ID			"c"
#define IDL_IMPL_TYPE_CONT_NS_ID	"type_container"
#define IDL_IMPL_STUB_NS_ID			"stub"
#define IDL_IMPL_NS					"::" IDL_IMPL_NS_ID
//#define IDL_IMPL_C_NS				"::" IDL_IMPL_NS_ID "::" IDL_IMPL_C_NS_ID

//Uncomment the second line to see the error. murrayc.
#define IDL_IMPL_C_NS_NOTUSED ""
//#define IDL_IMPL_C_NS_NOTUSED IDL_IMPL_C_NS "::"

#define IDL_IMPL_TYPE_CONT_NS		"::" IDL_IMPL_NS_ID "::" IDL_IMPL_TYPE_CONT_NS_ID
#define IDL_IMPL_STUB_NS			"::" IDL_IMPL_NS_ID "::" IDL_IMPL_STUB_NS_ID
#define IDL_INDENT_AMOUNT			0 // i.e. one tab (>0 count spaces)
#define IDL_CPP_HEADER_EXT			"-cpp-common.hh"
#define IDL_CPP_MODULE_EXT			"-cpp-common.cc"
#define IDL_CPP_STUB_HEADER_EXT		"-cpp-stubs.hh"
#define IDL_CPP_STUB_MODULE_EXT		"-cpp-stubs.cc"
#define IDL_CPP_SKEL_HEADER_EXT		"-cpp-skels.hh"
#define IDL_CPP_SKEL_MODULE_EXT		"-cpp-skels.cc"
#define IDL_CPP_MAIN_HEADER_EXT		"-cpp.hh"
#define IDL_CPP_MAIN_MODULE_EXT		"-cpp.cc"




class Indent {
private:
	string::size_type		Position;

public:
	Indent()
		: Position(0) {
	}

	void indent() {
		Position++;
	}
	void outdent() {
		Position--;
	}
	Indent &operator++() {
		indent();
		return *this;
	}
	Indent &operator--() {
		outdent();
		return *this;
	}
	Indent operator++(int) {
		Indent copy(*this);
		indent();
		return copy;
	}
	Indent operator--(int) {
		Indent copy(*this);
		outdent();
		return copy;
	}

	friend ostream &operator<<(ostream &ostr,Indent const &indent);
};




// Tool functions -------------------------------------------------------------
string idlUpper(string const &orig);
string idlLower(string const &orig);
string idlGetNodeLocation(IDL_tree node);
string idlGetNodeTypeString(IDL_tree node);
string idlGetTypeString(IDL_tree_type type);
string idlGetQualIdentifier(IDL_tree ident);
bool idlIsCPPKeyword(string const &id);
string idlGetCast(string const &what, string const &type);
	



#endif
