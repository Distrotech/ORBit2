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
 *
 *  Purpose:	IDL compiler type representation
 *
 *
 */


#ifndef ORBITCPP_TYPES_IDLSIMPLETYPE
#define ORBITCPP_TYPES_IDLSIMPLETYPE

#include "IDLType.hh"

class IDLSimpleType: public IDLType
{
protected:
	virtual string get_fixed_cpp_typename () const = 0;
	virtual string get_fixed_c_typename () const = 0;
public:
	////////////////////////////////////////////
	// Constants

	void const_decl_write (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_id,
			       const string     &value,
			       const IDLTypedef *active_typedef = 0) const;
	
	////////////////////////////////////////////
	// Stubs

	string stub_decl_arg_get (const string     &cpp_id,
				  IDL_param_attr    direction,
				  const IDLTypedef *active_typedef = 0) const;

	string stub_decl_ret_get (const IDLTypedef *active_typedef = 0) const;
	
	void stub_impl_arg_pre (ostream        &ostr,
				Indent         &indent,
				const string   &cpp_id,
				IDL_param_attr  direction) const;
	
	string stub_impl_arg_call (const string   &cpp_id,
				   IDL_param_attr  direction) const;
	
	void stub_impl_arg_post (ostream        &ostr,
				 Indent         &indent,
				 const string   &cpp_id,
				 IDL_param_attr  direction) const;

	void stub_impl_ret_pre (ostream &ostr,
				Indent  &indent) const;

	void stub_impl_ret_call (ostream      &ostr,
				 Indent       &indent,
				 const string &c_call_expression) const;

	void stub_impl_ret_post (ostream &ostr,
				 Indent  &indent) const;
	
	////////////////////////////////////////////
	// Skels

	string skel_decl_arg_get (const string     &c_id,
				  IDL_param_attr    direction,
				  const IDLTypedef *active_typedef = 0) const;

	string skel_decl_ret_get (const IDLTypedef *active_typedef = 0) const;
	
	void skel_impl_arg_pre (ostream        &ostr,
				Indent         &indent,
				const string   &c_id,
				IDL_param_attr  direction) const;
	
	string skel_impl_arg_call (const string   &c_id,
				   IDL_param_attr  direction) const;
	
	void skel_impl_arg_post (ostream        &ostr,
				 Indent         &indent,
				 const string   &c_id,
				 IDL_param_attr  direction) const;

	void skel_impl_ret_pre (ostream &ostr,
				Indent  &indent) const ;

	void skel_impl_ret_call (ostream      &ostr,
				 Indent       &indent,
				 const string &cpp_call_expression) const;

	void skel_impl_ret_post (ostream &ostr,
				 Indent  &indent) const;

	////////////////////////////////////////////
	// Members of compund types

	// Compund declaration
	string get_cpp_member_typename () const;

	string member_decl_arg_get () const;

	void member_impl_arg_copy (ostream      &ostr,
				   Indent       &indent,
				   const string &cpp_id) const;

	// Compound conversion: C++ -> C
	void member_pack_to_c_pre  (ostream      &ostr,
				    Indent       &indent,
				    const string &member_id,
				    const string &c_struct_id) const;

	void member_pack_to_c_pack (ostream      &ostr,
				    Indent       &indent,
				    const string &member_id,
				    const string &c_struct_id) const;

	void member_pack_to_c_post (ostream      &ostr,
				    Indent       &indent,
				    const string &member_id,
				    const string &c_struct_id) const;

	
	// Compound conversion: C -> C++
	void member_unpack_from_c_pre  (ostream      &ostr,
					Indent       &indent,
					const string &member_id,
					const string &c_struct_id) const;

	void member_unpack_from_c_pack (ostream      &ostr,
					Indent       &indent,
					const string &member_id,
					const string &c_struct_id) const;

	void member_unpack_from_c_post  (ostream      &ostr,
					 Indent       &indent,
					 const string &member_id,
					 const string &c_struct_id) const;
};

#endif //ORBITCPP_TYPES_IDLSIMPLETYPE


