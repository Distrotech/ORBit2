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


#ifndef ORBITCPP_TYPES_IDLARRAY
#define ORBITCPP_TYPES_IDLARRAY

#include "IDLUserDefSimpleType.hh"

class IDLArray : public IDLUserDefSimpleType
{
friend class IDLArrayList;
	typedef IDLUserDefSimpleType Super;
	typedef std::vector<int> 	Dimensions;
	typedef Dimensions::const_iterator 		const_iterator;

private:
	Dimensions m_dims;
	IDLType const 	&m_elementType;
	
public:
	IDLArray(IDLType const &elementType, string const &id,IDL_tree node,
			 IDLScope *parentscope = NULL)
		: IDLUserDefSimpleType(id,node,parentscope),m_elementType(elementType) {		

		for(IDL_tree curdim = IDL_TYPE_ARRAY(node).size_list;
			curdim;
			curdim = IDL_LIST(curdim).next) {
			m_dims.push_back(IDL_INTEGER(IDL_LIST(curdim).data).value);
		}
	}

	const_iterator begin() const {
		return m_dims.begin();
	}
	const_iterator end() const {
		return m_dims.end();
	}

  	string getInvalidReturn() const {
		return "return NULL;\n";
	}

	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const;
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const;

	bool isVariableLength() const {
		return m_elementType.isVariableLength();
	}

	// struct and exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const;
	
	void writeUnionModifiers(ostream &ostr,Indent &indent, string const &id,
							 string const &discriminatorVal,
							 IDLTypedef const *activeTypedef = NULL) const;
	
	void writeUnionAccessors(ostream &ostr,Indent &indent, string const &id,
							 string const &discriminatorVal,
							 IDLTypedef const *activeTypedef = NULL) const;
	
	// union stuff
	virtual void writeUnionReferents(ostream &ostr,Indent &indent, string const &id,
									 string const &discriminatorVal,
									 IDLTypedef const *activeTypedef = NULL) const;

	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef=NULL) const;

	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const;
	
	void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const;
	
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const;

	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const;
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const;
	// see IDLType for doc
	void writeInitCode(ostream &ostr, Indent &indent, string const &ident) const;
	void writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
	void writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
};

#endif //ORBITCPP_TYPES_IDLARRAY
