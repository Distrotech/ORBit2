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


#ifndef __ORBITCPP_BASICTYPES_HH
#define __ORBITCPP_BASICTYPES_HH

#include <orbit/orbit.h>




namespace CORBA {
	typedef CORBA_char Char;
	typedef CORBA_wchar WChar;
	typedef CORBA_boolean Boolean;
	typedef CORBA_octet Octet;
	typedef Char& Char_out;
	typedef WChar& WChar_out;
	typedef Boolean& Boolean_out;
	typedef Octet& Octet_out;
	
	typedef CORBA_short Short;
	typedef CORBA_long Long;
	typedef CORBA_long_long LongLong;
	typedef Short& Short_out;
	typedef Long& Long_out;
	typedef LongLong& LongLong_out;
	
	typedef CORBA_unsigned_short UShort;
	typedef CORBA_unsigned_long ULong;
	typedef CORBA_unsigned_long_long ULongLong;
	typedef UShort& UShort_out;
	typedef ULong& ULong_out;
	typedef ULongLong& ULongLong_out;
	
	typedef CORBA_float Float;
	typedef CORBA_double Double;
	typedef CORBA_long_double LongDouble;
	typedef Float& Float_out;
	typedef Double& Double_out;
	typedef LongDouble& LongDouble_out;
	
	typedef Char *ORBid;
	typedef Char *RepositoryId;

	Char* string_alloc(ULong len);
	Char* string_dup(const Char* str);
	void string_free(Char* str);

	WChar* wstring_alloc(ULong len);
	WChar* wstring_dup(const WChar* str);
	void wstring_free(WChar* str);
};




#endif //__ORBITCPP_BASICTYPES_HH
