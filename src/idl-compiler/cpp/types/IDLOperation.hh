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

#ifndef ORBITCPP_TYPES_IDLOPERATION
#define ORBITCPP_TYPES_IDLOPERATION

#include "IDLElement.hh"
#include "IDLType.hh"

class IDLException;

class IDLOperation : public IDLElement {
public:
	struct ParameterInfo {
		IDL_param_attr  direction;
		IDLType        *type;
		string          id;
	};

	typedef vector<ParameterInfo>  ParameterList;
	typedef vector<IDLException *> ExceptionList;

	ParameterList m_parameterinfo;
	ExceptionList m_raises;

	IDLType *m_returntype;

	IDLOperation(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLElement(id,node,parentscope) {
	}
	
	string stub_ret_get     () const;
	string stub_arglist_get () const;
	string stub_decl_proto  () const;
	string stub_decl_impl   () const;

	void stub_do_pre  (ostream &ostr, Indent &indent) const;
	void stub_do_call (ostream &ostr, Indent &indent) const;
	void stub_do_post (ostream &ostr, Indent &indent) const;

	string skel_ret_get     () const;
	string skel_arglist_get () const;
	string skel_decl_proto  () const;

	void skel_do_pre  (ostream &ostr, Indent &indent) const;
	void skel_do_call (ostream &ostr, Indent &indent) const;
	void skel_do_post (ostream &ostr, Indent &indent) const;
};

#endif //ORBITCPP_TYPES_IDLOPERATION
