/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 1998 Phil Dawes
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
 *  Author: Phil Dawes <philipd@users.sourceforge.net>
 *
 */




#include <orb/orbitcpp_types.hh>




CORBA::Char* 
CORBA::string_alloc(CORBA::ULong len) {
	// TODO: need to deal with exceptions here!
	return CORBA_string_alloc(len);
}




CORBA::Char* 
CORBA::string_dup(const Char* str) {
	// TODO: need to deal with exceptions here!
	return CORBA_string_dup(const_cast<Char*>(str));
}




void 
CORBA::string_free(Char* str) {
	// TODO: need to deal with exceptions here!
	CORBA_free(str);
}




CORBA::WChar* 
CORBA::wstring_alloc(CORBA::ULong len) {
	// TODO: need to deal with exceptions here!
	return CORBA_wstring_alloc(len);
}




CORBA::WChar* 
CORBA::wstring_dup(const WChar* str) {
	// TODO: need to deal with exceptions here!
	// return CORBA_wstring_dup(const_cast<WChar*>(str));
	// *** FIXME ORBit does not yet have this.
	return NULL;
}




void 
CORBA::wstring_free(WChar* str) {
	// TODO: need to deal with exceptions here!
	CORBA_free(str);
}

