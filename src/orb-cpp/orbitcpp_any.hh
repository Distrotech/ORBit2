/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 2000 John Luebs
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
 *  Author: John Luebs <jkluebs@marikarpress.com>
 *
 */

#ifndef __ORBITCPP_ANY_HH
#define __ORBITCPP_ANY_HH

#include <cstdlib>
#include <orbit/orbit.h>
#include "orbitcpp_types.hh"
#include "orbitcpp_object.hh"
#include "orbitcpp_exception.hh"
#include "orbitcpp_typecode.hh"

namespace CORBA {
	class Any;
	
	typedef _orbitcpp::DataVar_var<Any> Any_var;
	typedef _orbitcpp::DataVar_out<Any> Any_out;
	
	class Any {
	private:
		CORBA_any m_target;
		
		void copy (const Any &in);
		void free ();

	public:		
		// begin ORBitcpp extension

		struct SeqTraits
        {
			typedef CORBA::Any               value_t;
			typedef CORBA_any                c_value_t;
			typedef CORBA_sequence_CORBA_any c_seq_t;

			static c_seq_t* alloc_c () {
				return CORBA_sequence_CORBA_any__alloc ();
			}
			static c_value_t* alloc_c_buf (CORBA::ULong l) {
				return CORBA_sequence_CORBA_any_allocbuf (l);
			}
				 
			static void pack_elem (const value_t &cpp_elem, c_value_t &c_elem) {
			}

			static void unpack_elem (value_t &cpp_elem, const c_value_t &c_elem) {
			}
        };

		
		CORBA_any* _orbitcpp_cobj () const {
			return const_cast<CORBA_any*> (&m_target);
		}
		
		static Any _orbitcpp_wrap (CORBA_any *c_any);
		
		void insert_simple(CORBA::TypeCode_ptr, void*, Boolean v_copy = CORBA_TRUE);
		void insert_simple(CORBA::TypeCode_ptr tc,const void* in,Boolean v_copy = CORBA_TRUE) {
				insert_simple(tc, const_cast<void*>(in), v_copy);
		}

		template <class T>
		Boolean extract(CORBA::TypeCode_ptr tc, T& value) const {
			if (m_target._type != tc->_orbitcpp_cobj ())
				return CORBA_FALSE;
			
			value = *reinterpret_cast<T*>(m_target._value);
			return CORBA_TRUE;
		}
		
		template <class T>
		Boolean extract_ptr(CORBA::TypeCode_ptr tc, const T*& value) const {
			if (m_target._type != tc->_orbitcpp_cobj ())
				return CORBA_FALSE;
			
			value = reinterpret_cast<T*>(m_target._value);
			return CORBA_TRUE;
		}
		// end ORBitcpp extension

		Any();
		~Any();
		
		Any(const Any& in);
		Any& operator =(const Any& in);
		
		const void *value() const {
			return m_target._value;
		}
				
		struct from_boolean {
			from_boolean(Boolean b) : val(b) {}
			Boolean val;
		};
		struct from_octet {
			from_octet(Octet o) : val(o) {}
			Octet val;
		};
		struct from_char {
			from_char(Char c) : val(c) {}
			Char val;
		};
		struct from_wchar {
			from_wchar(WChar wc) : val(wc) {}
			WChar val;
		};
		struct from_string {
			from_string(char *s, ULong b, Boolean n = CORBA_FALSE)
				: val(s), bound(b), nocopy(n) {}
			from_string(const char *s, ULong b)
				: val(const_cast<char*>(s)), bound(b), nocopy(0) {}
			char *val;
			ULong bound;
			Boolean nocopy;
		};
		struct from_wstring {
			from_wstring(WChar *s, ULong b, Boolean n = CORBA_FALSE)
				: val(s), bound(b), nocopy(n) {}
			from_wstring(const WChar *s, ULong b)
				: val(const_cast<WChar*>(s)), bound(b), nocopy(0) {}
			WChar *val;
			ULong bound;
			Boolean nocopy;
		};
		/*
		struct from_fixed {
			from_fixed(const Fixed& f, UShort d, UShort s)
				: val(f), digits(d), scale(s) {}
			const Fixed& val;
			UShort digits;
			UShort scale;
		};
		*/
		
		struct to_boolean {
			to_boolean(Boolean& b) : ref(b) {}
			Boolean &ref;
		};
		struct to_octet {
			to_octet(Octet &o) : ref(o) {}
			Octet &ref;
		};
		struct to_char {
			to_char(Char &c) : ref(c) {}
			Char &ref;
		};
		struct to_wchar {
			to_wchar(WChar &wc) : ref(wc) {}
			WChar &ref;
		};
		struct to_string {
			to_string(const char *&s, ULong b)
				: val(s), bound(b) {}
			const char *&val;
			ULong bound;
		};
		struct to_wstring {
			to_wstring(const WChar *&s, ULong b)
				: val(s), bound(b) {}
			const WChar *&val;
			ULong bound;
		};
		/*
		struct to_fixed {
			to_fixed(Fixed& f, UShort d, UShort s)
				: val(f), digits(d), scale(s) {}
			Fixed& val;
			UShort digits;
			UShort scale;
		};
		*/
		struct to_object {
			to_object(Object_ptr& obj) : ref(obj) {}
			Object_ptr &ref;
		};
		
		void operator<<=(Short in) {
			insert_simple(_tc_short, &in);
		}
		void operator<<=(UShort in) {
			insert_simple(_tc_ushort, &in);
		}
		void operator<<=(Long in) {
			insert_simple(_tc_long, &in);
		}
		void operator<<=(ULong in) {
			insert_simple(_tc_ulong, &in);
		}
		void operator<<=(LongLong in) {
			insert_simple(_tc_longlong, &in);
		}
		void operator<<=(ULongLong in) {
			insert_simple(_tc_ulonglong, &in);
		}
		void operator<<=(Float in) {
			insert_simple(_tc_float, &in);
		}
		void operator<<=(Double in) {
			insert_simple(_tc_double, &in);
		}
		// ORBit does not implement this for good reasons
		/*void operator<<=(LongDouble in) {
			insert_simple(_tc_longdouble, &in);
		}*/
		void operator<<=(const Any & in) {
			insert_simple(_tc_any, &in);
		}
		void operator<<=(Any * in) {
			insert_simple(_tc_any, in, CORBA_FALSE);
		}
		//void operator<<=(const Exception &);
		//void operator<<=(Exception *);
		void operator<<=(const char * in) {
			insert_simple(_tc_string, &in);
		}
		void operator<<=(const WChar * in) {
			insert_simple(_tc_wstring, &in);
		}
		void operator<<=(from_boolean in) {
			insert_simple(_tc_boolean, &in.val);
		}
		void operator<<=(from_octet in) {
			insert_simple(_tc_octet, &in.val);
		}
		void operator<<=(from_char in) {
			insert_simple(_tc_char, &in.val);
		}
		void operator<<=(from_wchar in) {
			insert_simple(_tc_wchar, &in.val);
		}
		void operator<<=(from_string in);
		void operator<<=(from_wstring in);
		void operator<<=(Object_ptr in) {
			insert_simple(_tc_Object, &in);
		}
		void operator<<=(Object_ptr * in) {
			insert_simple(_tc_Object, &in);
			release(*in);
		}
		//void operator<<=(from_fixed);
		
		Boolean operator>>=(Short& out) const {
			return extract(_tc_short, out);
		}
		Boolean operator>>=(UShort& out) const {
			return extract(_tc_ushort, out);
		}			
		Boolean operator>>=(Long& out) const {
			return extract(_tc_long, out);
		}
		Boolean operator>>=(ULong& out) const {
			return extract(_tc_ulong, out);
		}
		Boolean operator>>=(LongLong& out) const {
			return extract(_tc_longlong, out);
		}
		Boolean operator>>=(ULongLong& out) const {
			return extract(_tc_ulonglong, out);
		}
		Boolean operator>>=(Float& out) const {
			return extract(_tc_float, out);
		}
		Boolean operator>>=(Double& out) const {
			return extract(_tc_double, out);
		}
		//void operator>>=(LongDouble&) const;
		Boolean operator>>=(const char *& out) const {
			return extract(_tc_string, const_cast<char*&>(out));
		}
		Boolean operator>>=(const WChar *& out) const {
			return extract(_tc_wstring, const_cast<WChar*&>(out));
		}
		
		Boolean operator>>=(to_boolean out) const {
			return extract(_tc_boolean, out.ref);
		}
		Boolean operator>>=(to_octet out) const {
			return extract(_tc_octet, out.ref);
		}
		Boolean operator>>=(to_char out) const {
			return extract(_tc_char, out.ref);
		}
		Boolean operator>>=(to_wchar out) const {
			return extract(_tc_wchar, out.ref);
		}
		
		Boolean operator>>=(to_string out) const;
		Boolean operator>>=(to_wstring out) const;

		Boolean operator>>=(to_object out) const;
		Boolean operator>>=(Object_ptr& out) const {
			return extract(_tc_Object, out);
		}
		
		// Begin Orbit-CPP Extension
		Boolean operator>>=(Any & out) const {
			return extract(_tc_any, out);
		}
		// End Orbit-CPP Extension
		Boolean operator>>=(const Any *& out) const {
			return extract_ptr(_tc_any, out);
		}
		
	private:
		void operator<<=(unsigned char);
		Boolean operator>>=(unsigned char&) const;
	};
};

#endif
