/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 2000 Sam Couter
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
 *  Author: Sam Couter <sam@topic.com.au>
 */

#ifndef __ORBITCPP_TYPECODE_HH
#define __ORBITCPP_TYPECODE_HH

#include <orbit/orb-core/corba-typecode.h>
#include "orbitcpp_object.hh"
#include "orbitcpp_exception.hh"
#include "orbitcpp_sequence.hh"

namespace CORBA {

	/*
	 * Need declarations and definitions for:
	 *
	 * StructMemberSeq
	 * UnionMemberSeq
	 * EnumMemberSeq
	 */
	
	/* try this for now */
	class StructMember : public CORBA::Object
	{
	private:
		CORBA_StructMember m_target;

	public:
		operator CORBA_StructMember *() {
			return(&m_target);
		}
	};

	ORBITCPP_DECLARE_SIMPLE_SEQUENCE(CORBA::StructMember,
		CORBA_sequence_CORBA_StructMember__alloc,
		CORBA_sequence_CORBA_StructMember_allocbuf,
		CORBA_sequence_CORBA_StructMember,
		StructMemberSeq)

	class UnionMember : public CORBA::Object
	{
	private:
		CORBA_UnionMember m_target;

	public:
		operator CORBA_UnionMember *() {
			return(&m_target);
		}
	};

	ORBITCPP_DECLARE_SIMPLE_SEQUENCE(CORBA::UnionMember,
		CORBA_sequence_CORBA_UnionMember__alloc,
		CORBA_sequence_CORBA_UnionMember_allocbuf,
		CORBA_UnionMember,
		UnionMemberSeq)

	class EnumMember : public CORBA::Object
	{
	private:
		CORBA_Identifier m_target;

	public:
		operator CORBA_Identifier *() {
			return(&m_target);
		}
	};

	ORBITCPP_DECLARE_SIMPLE_SEQUENCE(CORBA::EnumMember,
		CORBA_sequence_CORBA_Identifier__alloc,
		CORBA_sequence_CORBA_Identifier_allocbuf,
		CORBA_Identifier,
		EnumMemberSeq)

	typedef short ValueModifier;
	const ValueModifier VM_NONE = 0;
	const ValueModifier VM_CUSTOM = 1;
	const ValueModifier VM_ABSTRACT = 2;
	const ValueModifier VM_TRUNCATABLE = 3;

	enum TCKind {
		tk_null = CORBA_tk_null,
		tk_void = CORBA_tk_void,
		tk_short = CORBA_tk_short,
		tk_long = CORBA_tk_long,
		tk_ushort = CORBA_tk_ushort,
		tk_ulong = CORBA_tk_ulong,
		tk_float = CORBA_tk_float,
		tk_double = CORBA_tk_double,
		tk_boolean = CORBA_tk_boolean,
		tk_char = CORBA_tk_char,
		tk_octet = CORBA_tk_octet,
		tk_any = CORBA_tk_any,
		tk_TypeCode = CORBA_tk_TypeCode,
		tk_Principal = CORBA_tk_Principal,
		tk_objref = CORBA_tk_objref,
		tk_struct = CORBA_tk_struct,
		tk_union = CORBA_tk_union,
		tk_enum = CORBA_tk_enum,
		tk_string = CORBA_tk_string,
		tk_sequence = CORBA_tk_sequence,
		tk_array = CORBA_tk_array,
		tk_alias = CORBA_tk_alias,
		tk_except = CORBA_tk_except,
		tk_longlong = CORBA_tk_longlong,
		tk_ulonglong = CORBA_tk_ulonglong,
		tk_longdouble = CORBA_tk_longdouble,
		tk_wchar = CORBA_tk_wchar,
		tk_wstring = CORBA_tk_wstring,
		tk_fixed = CORBA_tk_fixed,
		tk_recursive = CORBA_tk_recursive,
		tk_last = CORBA_tk_last		/* ORBit hack - is it needed here? */
	};

	class TypeCode;
	typedef TypeCode *TypeCode_ptr;
	typedef ::_orbitcpp::ObjectPtr_var<TypeCode,TypeCode_ptr>
		TypeCode_var;

	class TypeCode
	{
		friend void release (TypeCode_ptr o);
	public:
		class Bounds : public UserException { ; };
		class BadKind : public UserException { ; };

		// for all TypeCode kinds
		Boolean equal(TypeCode_ptr) const;
		Boolean equivalent(TypeCode_ptr) const;
		TCKind kind() const;
		TypeCode_ptr get_compact_typecode() const;

		// for tk_objref, tk_struct, tk_union, tk_enum, tk_alias and tk_except
		const char* id() const;

		// for tk_objref, tk_struct, tk_union, tk_enum, tk_alias and tk_except
		const char* name() const;

		// for tk_struct, tk_union, tk_enum and tk_except
		ULong member_count() const;
		const char* member_name(ULong index) const;

		// for tk_struct, tk_union and tk_except
		TypeCode_ptr member_type(ULong index) const;

		// for tk_union
		// Any *member_label(ULong index) const;
		TypeCode_ptr discriminator_type() const;
		Long default_index() const;

		// for tk_string, tk_sequence and tk_array
		ULong length() const;

		// for tk_sequence, tk_array and tk_alias
		TypeCode_ptr content_type() const;

		// for tk_fixed
		UShort fixed_digits() const;
		Short fixed_scale() const;

#if 0
		Visibility member_visibility(ULong index) const;
#endif
		ValueModifier type_modifier() const;
		TypeCode_ptr concrete_base_type() const;

		// so that this pseudo object can be used by the object_var templates...
		static TypeCode_ptr _duplicate(TypeCode_ptr o) {
			return reinterpret_cast<CORBA::TypeCode_ptr>(
				_orbitcpp::duplicate_guarded(*o)
			);
		}
		static TypeCode_ptr _narrow(TypeCode_ptr o) {
			return _duplicate(o);
		}
		static TypeCode_ptr _nil() {
			return CORBA_OBJECT_NIL;
		}
		
	private:
		CORBA_TypeCode_struct m_target;

	public:
		operator CORBA_TypeCode_struct *() {
			return(&m_target);
		}
	};

	inline void release(TypeCode_ptr o) {
		_orbitcpp::release_guarded((CORBA_TypeCode)o);
	}
	
	Boolean is_nil(Object_ptr o);

	
	extern TypeCode_ptr _tc_null;
	extern TypeCode_ptr _tc_void;
	extern TypeCode_ptr _tc_short;
	extern TypeCode_ptr _tc_long;
	extern TypeCode_ptr _tc_longlong;
	extern TypeCode_ptr _tc_ushort;
	extern TypeCode_ptr _tc_ulong;
	extern TypeCode_ptr _tc_ulonglong;
	extern TypeCode_ptr _tc_float;
	extern TypeCode_ptr _tc_double;
	extern TypeCode_ptr _tc_longdouble;
	extern TypeCode_ptr _tc_boolean;
	extern TypeCode_ptr _tc_char;
	extern TypeCode_ptr _tc_wchar;
	extern TypeCode_ptr _tc_octet;
	extern TypeCode_ptr _tc_any;
	extern TypeCode_ptr _tc_TypeCode;
	extern TypeCode_ptr _tc_Principal;
	extern TypeCode_ptr _tc_Object;
	extern TypeCode_ptr _tc_string;
	extern TypeCode_ptr _tc_wstring;
}

#endif		/* __ORBITCPP_TYPECODE_HH */
