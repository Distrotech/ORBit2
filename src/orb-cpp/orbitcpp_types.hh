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

#define ORBITCPP_DECLARE_SIMPLE(CPPType, CType) \
    typedef CType    CPPType;			\
    typedef CPPType& CPPType##_out;

namespace CORBA {
    ORBITCPP_DECLARE_SIMPLE(Char,    CORBA_char);
    ORBITCPP_DECLARE_SIMPLE(WChar,   CORBA_wchar);
    ORBITCPP_DECLARE_SIMPLE(Boolean, CORBA_boolean);
    ORBITCPP_DECLARE_SIMPLE(Octet,   CORBA_octet);
    
    ORBITCPP_DECLARE_SIMPLE(Short,    CORBA_short);
    ORBITCPP_DECLARE_SIMPLE(Long,     CORBA_long);
    ORBITCPP_DECLARE_SIMPLE(LongLong, CORBA_long_long);
    
    ORBITCPP_DECLARE_SIMPLE(UShort,    CORBA_unsigned_short);
    ORBITCPP_DECLARE_SIMPLE(ULong,     CORBA_unsigned_long);
    ORBITCPP_DECLARE_SIMPLE(ULongLong, CORBA_unsigned_long_long);
    
    ORBITCPP_DECLARE_SIMPLE(Float,      CORBA_float);
    ORBITCPP_DECLARE_SIMPLE(Double,     CORBA_double);
    ORBITCPP_DECLARE_SIMPLE(LongDouble, CORBA_long_double);

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
