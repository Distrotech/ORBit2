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

#include "IDLException.hh"

void
IDLException::stub_check_and_propagate (ostream &ostr,
					Indent  &indent) const
{
	ostr << indent << "if (!strcmp (repo_id, ex_" << get_c_typename () << "))" << endl
	     << indent++ << "{" << endl;

	ostr << indent << get_cpp_typename () << " ex;" << endl;
	ostr << indent << "ex._orbitcpp_unpack ("
	     << "*((" << get_c_typename () << "*) value));" << endl;
	ostr << indent << "throw ex;" << endl;
	
	ostr << --indent << "}" << endl;
	
}
