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

#ifndef ORBITCPP_TYPES_IDLARRAY
#define ORBITCPP_TYPES_IDLARRAY

#include "IDLType.hh"
#include "IDLElement.hh"

class IDLArray: public IDLElement,
		public IDLType
{
        typedef IDLElement                 Super;
public:
        typedef std::vector<int>           Dimensions;
        typedef Dimensions::const_iterator const_iterator;

private:
        Dimensions     m_dims;
        const IDLType &m_element_type;

	friend class IDLArrayList;
	
public:
	IDLArray (IDLType const &element_type,
		  string const  &id,
		  IDL_tree       node,
		  IDLScope      *parentscope = 0);
	
	const_iterator begin () const { return m_dims.begin (); };
        const_iterator end ()   const { return m_dims.end ();   };
	
public:

	bool conversion_required () const;
	bool is_fixed () const;

	////////////////////////////////////////////
	// Creating typedefs

	void typedef_decl_write (ostream          &ostr,
				 Indent           &indent,
				 IDLCompilerState &state,
				 const IDLTypedef &target,
				 const IDLTypedef *active_typedef = 0) const;
	
	////////////////////////////////////////////
	// Stubs

	// Stub declaration
	string stub_decl_arg_get (const string     &cpp_id,
				  IDL_param_attr    direction,
				  const IDLTypedef *active_typedef = 0) const;
	
	string stub_decl_ret_get (const IDLTypedef *active_typedef = 0) const;
	
	// Stub implementation -- argument
	void stub_impl_arg_pre (ostream          &ostr,
				Indent           &indent,
				const string     &cpp_id,
				IDL_param_attr    direction,
				const IDLTypedef *active_typedef = 0) const;
	
	string stub_impl_arg_call (const string     &cpp_id,
				   IDL_param_attr    direction,
				   const IDLTypedef *active_typedef = 0) const;
	
	void stub_impl_arg_post (ostream          &ostr,
				 Indent           &indent,
				 const string     &cpp_id,
				 IDL_param_attr    direction,
				 const IDLTypedef *active_typedef = 0) const;

	// Stub implementation -- return value
	void stub_impl_ret_pre (ostream          &ostr,
				Indent           &indent,
				const IDLTypedef *active_typedef = 0) const;
	
	void stub_impl_ret_call (ostream          &ostr,
				 Indent           &indent,
				 const string     &c_call_expression,
				 const IDLTypedef *active_typedef = 0) const;

	void stub_impl_ret_post (ostream          &ostr,
				 Indent           &indent,
				 const IDLTypedef *active_typedef = 0) const;
	
	////////////////////////////////////////////
	// Skels

	// Skel declaration
	string skel_decl_arg_get (const string     &c_id,
				  IDL_param_attr    direction,
				  const IDLTypedef *active_typedef = 0) const;
	
	string skel_decl_ret_get (const IDLTypedef *active_typedef = 0) const;
	
	// Skel implementation -- argument
	void skel_impl_arg_pre (ostream          &ostr,
				Indent           &indent,
				const string     &c_id,
				IDL_param_attr    direction,
				const IDLTypedef *active_typedef = 0) const;
	
	string skel_impl_arg_call (const string     &c_id,
				   IDL_param_attr    direction,
				   const IDLTypedef *active_typedef = 0) const;
	
	void skel_impl_arg_post (ostream          &ostr,
				 Indent           &indent,
				 const string     &c_id,
				 IDL_param_attr    direction,
				 const IDLTypedef *active_typedef = 0) const;

	// Skel implementation -- return value
	void skel_impl_ret_pre (ostream          &ostr,
				Indent           &indent,
				const IDLTypedef *active_typedef = 0) const;

	void skel_impl_ret_call (ostream          &ostr,
				 Indent           &indent,
				 const string     &cpp_call_expression,
				 const IDLTypedef *active_typedef = 0) const;
    
	void skel_impl_ret_post (ostream          &ostr,
				 Indent           &indent,
				 const IDLTypedef *active_typedef = 0) const;

	////////////////////////////////////////////
	// Members of compund types

	// Compund declaration
	string get_cpp_member_typename (const IDLTypedef *active_typedef = 0) const;
	string get_c_member_typename   (const IDLTypedef *active_typedef = 0) const;

	string member_decl_arg_get (const IDLTypedef *active_typedef = 0) const;
	
	void member_impl_arg_copy (ostream          &ostr,
				   Indent           &indent,
				   const string     &cpp_id,
				   const IDLTypedef *active_typedef = 0) const;
	
	// Compound conversion: C++ -> C
	void member_pack_to_c (ostream          &ostr,
			       Indent           &indent,
			       const string     &cpp_id,
			       const string     &c_id,
			       const IDLTypedef *active_typedef = 0) const;

	// Initialization
	void member_init_cpp (ostream          &ostr,
			      Indent           &indent,
			      const string     &cpp_id,
			      const IDLTypedef *active_typedef = 0) const;
	
	void member_init_c   (ostream          &ostr,
			      Indent           &indent,
			      const string     &c_id,
			      const IDLTypedef *active_typedef = 0) const;

	// Compound conversion: C -> C++
	void member_unpack_from_c  (ostream          &ostr,
				    Indent           &indent,
				    const string     &cpp_id,
				    const string     &c_id,
				    const IDLTypedef *active_typedef = 0) const;
private:
	void init_c_array (ostream      &ostr,
			   Indent       &indent,
			   const string &c_id) const;

	void init_cpp_array (ostream      &ostr,
			     Indent       &indent,
			     const string &cpp_id) const;
	
	void fill_c_array (ostream      &ostr,
			   Indent       &indent,
			   const string &cpp_id,
			   const string &c_id) const;

	void fill_cpp_array (ostream      &ostr,
			     Indent       &indent,
			     const string &cpp_id,
			     const string &c_id) const;

	void copy_cpp_array (ostream      &ostr,
			     Indent       &indent,
			     const string &src_id,
			     const string &dest_id) const;
};

#endif //ORBITCPP_TYPES_IDLARRAY


