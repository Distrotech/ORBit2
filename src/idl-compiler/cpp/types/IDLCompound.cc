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

#include "IDLCompound.hh"
#include "IDLType.hh"

#include <strstream>

void
IDLCompound::write_packing_decl (ostream &ostr,
				 Indent  &indent) const
{
	string c_id = IDL_IMPL_C_NS_NOTUSED + get_c_typename ();

	ostr << indent << c_id << " *" << "_orbitcpp_pack () const;" << endl;
	ostr << indent << "void _orbitcpp_pack (" << c_id << " &_c_struct) const;" << endl;
	ostr << indent << "void _orbitcpp_unpack (const " << c_id << " &_c_struct);" << endl;
}

void
IDLCompound::write_packing_impl (ostream &ostr,
				 Indent  &indent) const
{
	string c_id = IDL_IMPL_C_NS_NOTUSED + get_c_typename ();

	// Implementation of _orbitcpp_pack that returns a newly
	// allocated C structure on heap	
	ostr << indent << c_id << " * "
	     << get_cpp_method_prefix () << "::_orbitcpp_pack () const" << endl
	     << indent++ << '{' << endl;
	ostr << indent << c_id << " *_c_struct = " << c_id << "__alloc ()"
	     << ';' << endl << endl;
	ostr << indent << "if (!_c_struct)" << endl;
	indent++;
	ostr << indent << "throw CORBA::NO_MEMORY ();" << endl << endl;
	indent--;
	ostr << indent << "_orbitcpp_pack (*_c_struct);" << endl;
	ostr << indent << "return _c_struct;" << endl
	     << --indent << '}' << endl << endl;


	
	// Implementation of _orbitcpp_pack that fills a
	// caller-supplied C structure
	ostr << indent << "void " << get_cpp_method_prefix () << "::_orbitcpp_pack "
	     << "(" << c_id << " &_c_struct) const" << endl
	     << indent++ << '{' << endl;

	// We create the output in three cycles to allow _pre and _post operations
	for (const_iterator i = begin (); i != end (); i++)
	{
		IDLMember &member = (IDLMember &) **i;

		member.getType ()->member_pack_to_c_pre (ostr, indent,
							 member.get_cpp_identifier (),
							 "_c_struct");
	}
	ostr << endl;

	for (const_iterator i = begin (); i != end (); i++)
	{
		IDLMember &member = (IDLMember &) **i;

		member.getType ()->member_pack_to_c_pack (ostr, indent,
							  member.get_cpp_identifier (),
							  "_c_struct");
	}
	ostr << endl;

	for (const_iterator i = begin (); i != end (); i++)
	{
		IDLMember &member = (IDLMember &) **i;

		member.getType ()->member_pack_to_c_post (ostr, indent,
							  member.get_cpp_identifier (),
							  "_c_struct");
	}
	ostr << endl;
	
	ostr << --indent << '}' << endl << endl;


	// Implementation of _orbitcpp_unpack
	ostr << indent << "void " << get_cpp_method_prefix () << "::_orbitcpp_unpack "
	     << "(const " << c_id << " &_c_struct)" << endl
	     << indent++ << '{' << endl;

	// We create the output in three cycles to allow _pre and _post operations
	for (const_iterator i = begin (); i != end (); i++)
	{
		IDLMember &member = (IDLMember &) **i;

		member.getType ()->member_unpack_from_c_pre (ostr, indent,
							     member.get_cpp_identifier (),
							     "_c_struct");
	}
	ostr << endl;
	
	for (const_iterator i = begin (); i != end (); i++)
	{
		IDLMember &member = (IDLMember &) **i;

		member.getType ()->member_unpack_from_c_pack (ostr, indent,
							      member.get_cpp_identifier (),
							      "_c_struct");
	}
	ostr << endl;
	
	for (const_iterator i = begin (); i != end (); i++)
	{
		IDLMember &member = (IDLMember &) **i;

		member.getType ()->member_unpack_from_c_post (ostr, indent,
							      member.get_cpp_identifier (),
							      "_c_struct");
	}
	ostr << endl;

	ostr << --indent << '}' << endl << endl;
}
