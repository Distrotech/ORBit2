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
 *  Purpose: IDL compiler base and tools
 *
 */




#include <cstring>
#include <cstdio>
#include <cctype>
#include "base.hh"




// Indent --------------------------------------------------------------------
ostream &
operator<<(ostream &ostr,Indent const &indent) {
	if (indent.Position > 1000 || indent.Position < 0) {
		cerr << "Indentation screwup. This is a bug." << endl;
		abort();
	}
	if (IDL_INDENT_AMOUNT == 0)
		ostr << string(indent.Position,'\t');
	else
		ostr << string(indent.Position * IDL_INDENT_AMOUNT,' ');
	return ostr;
}




// Tool functions -------------------------------------------------------------
string 
idlUpper(string const &orig) {
	string result(orig.size(),' ');

	string::const_iterator first = orig.begin(),last = orig.end();
	string::iterator dfirst = result.begin();
	while (first != last)
		*dfirst++ = toupper(*first++);
	return result;
}




string 
idlLower(string const &orig) {
	string result(orig.size(),' ');

	string::const_iterator first = orig.begin(),last = orig.end();
	string::iterator dfirst = result.begin();
	while (first != last)
		*dfirst++ = tolower(*first++);
	return result;
}




string 
idlGetNodeLocation(IDL_tree node) {
	char buf[1024];
	sprintf(buf,"%s:%i",node->_file,node->_line);
	return buf;
}




string idlGetNodeTypeString(IDL_tree node) {
	char const *original = IDL_NODE_TYPE_NAME(node);

	return idlLower(original);
}




string idlGetTypeString(IDL_tree_type type) {
	char const *original = IDL_tree_type_names[type];
  
	return idlLower(original);
}




string 
idlGetQualIdentifier(IDL_tree ident) {
	char *qname = IDL_ns_ident_to_qstring(ident,"::",0);
	string result = string("::")+qname;
	g_free(qname);
	return result;
}




bool 
idlIsCPPKeyword(string const &id) {
	// according to 1996 c++ draft standard, rev 2
	if (id == "and") return true;
	if (id == "and_eq") return true;
	if (id == "asm") return true;
	if (id == "auto") return true;
	if (id == "bitand") return true;
	if (id == "bitor") return true;
	if (id == "bool") return true;
	if (id == "break") return true;
	if (id == "case") return true;
	if (id == "catch") return true;
	if (id == "char") return true;
	if (id == "class") return true;
	if (id == "compl") return true;
	if (id == "const") return true;
	if (id == "const_cast") return true;
	if (id == "continue") return true;
	if (id == "default") return true;
	if (id == "delete") return true;
	if (id == "do") return true;
	if (id == "double") return true;
	if (id == "dynamic_cast") return true;
	if (id == "else") return true;
	if (id == "enum") return true;
	if (id == "explicit") return true;
	if (id == "extern") return true;
	if (id == "false") return true;
	if (id == "float") return true;
	if (id == "for") return true;
	if (id == "friend") return true;
	if (id == "goto") return true;
	if (id == "if") return true;
	if (id == "inline") return true;
	if (id == "int") return true;
	if (id == "long") return true;
	if (id == "mutable") return true;
	if (id == "namespace") return true;
	if (id == "new") return true;
	if (id == "not") return true;
	if (id == "not_eq") return true;
	if (id == "operator") return true;
	if (id == "or") return true;
	if (id == "or_eq") return true;
	if (id == "private") return true;
	if (id == "protected") return true;
	if (id == "public") return true;
	if (id == "register") return true;
	if (id == "reinterpret_cast") return true;
	if (id == "return") return true;
	if (id == "short") return true;
	if (id == "signed") return true;
	if (id == "sizeof") return true;
	if (id == "static") return true;
	if (id == "static_cast") return true;
	if (id == "struct") return true;
	if (id == "switch") return true;
	if (id == "template") return true;
	if (id == "this") return true;
	if (id == "throw") return true;
	if (id == "true") return true;
	if (id == "try") return true;
	if (id == "typedef") return true;
	if (id == "typeid") return true;
	if (id == "typename") return true;
	if (id == "union") return true;
	if (id == "unsigned") return true;
	if (id == "using") return true;
	if (id == "virtual") return true;
	if (id == "void") return true;
	if (id == "volatile") return true;
	if (id == "wchar_t") return true;
	if (id == "while") return true;
	if (id == "xor") return true;
	if (id == "xor_eq ") return true;
	return false;
}




string idlGetCast(string const &what, string const &type) {
	return "reinterpret_cast< "+type+">("+what+")";
}
