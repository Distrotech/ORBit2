/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 1999 Phil Dawes
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
 *			Andreas Kloeckner <ak@ixion.net>
 */




#ifndef __ORBITCPP_SEQUENCE_HH
#define __ORBITCPP_SEQUENCE_HH

#include <cstddef>
#include <cassert>
#include "orbitcpp_smartptr.hh"
#include <orbit/orb-core/orb-core.h>




namespace _orbitcpp {
	template<class T, class CSEQTYPE>
	class UnboundedSequenceTmpl;
	template<class T, class CSEQTYPE, CORBA::ULong max>
	class BoundedSequenceTmpl;

	template<class SEQ,class T, class CSEQTYPE,CORBA::ULong CapBounds = 0>
	class SequenceBase {
	public:
		typedef T _orbitcpp_value_type;
	
	protected:
		struct {
			CORBA::ULong	_maximum,_length;
			T				*_buffer;
			CORBA::Boolean	_release;
		} m_target;
	
	public:
		static void freebuf(T *data) {
			CORBA_free(data);
		}

		// construction and destruction
		SequenceBase() { 
			m_target._length = 0;
		}
		SequenceBase(SequenceBase const &src) {
			m_target = src.m_target;
			
			m_target._buffer = SEQ::allocbuf(m_target._maximum);
			if (!m_target._buffer) throw CORBA::NO_MEMORY();
			
			try {
				for (CORBA::ULong i = 0;i<m_target._length;i++) 
					m_target._buffer[i] = src.m_target._buffer[i];
			}
			catch (...) {
				SEQ::freebuf(m_target._buffer);
				throw;
			}
			
			_orbitcpp_set_internal_release_flag(true);
			
		}
		SequenceBase(CORBA::ULong max,CORBA::ULong length,T *data,CORBA::Boolean rel) {
			assert (length <= max);
		
			m_target._maximum = max;
			m_target._length = length;
			m_target._buffer = data;
		
			_orbitcpp_set_internal_release_flag(rel);
		}
		~SequenceBase() {
			if (release()) SEQ::freebuf(m_target._buffer);
			// this must be done for the following delete to work ok.
			m_target._buffer = NULL;
			m_target._length = 0;
			m_target._maximum = 0;
		}
		
		// op=
		SequenceBase &operator=(SequenceBase const &src) {
			T *buffer = NULL;
			if(src.m_target._maximum != 0) {
				buffer = SEQ::allocbuf(src.m_target._maximum);
				if (!buffer) throw CORBA::NO_MEMORY();
			}
			try {
				for (CORBA::ULong i = 0;i < src.m_target._length;i++)
					buffer[i] = src.m_target._buffer[i];
			}
			catch (...) {
				SEQ::freebuf(buffer);
				throw;
			}
			
			m_target._length = src.m_target._length;
			m_target._maximum = src.m_target._maximum;
			
			if (release()) SEQ::freebuf(m_target._buffer);
			m_target._buffer = buffer;
			_orbitcpp_set_internal_release_flag(true);
			return *this;
		}
		
		// property/contents access
		CORBA::Boolean release() const {
			return CORBA_sequence_get_release(_orbitcpp_get_c_sequence_ptr());
		}
		CORBA::ULong maximum() const {
			return m_target._maximum;
		}
		CORBA::ULong length() const {
			return m_target._length;
		}
		T &operator[](CORBA::ULong idx) {
			return m_target._buffer[idx];
		}
		const T &operator[](CORBA::ULong idx) const {
			return m_target._buffer[idx];
		}

		// begin ORBIT-C++ extension
		void _orbitcpp_set_internal_release_flag (CORBA::Boolean rel) {
			CORBA_sequence_set_release(&m_target,rel);
		}
		CSEQTYPE* _orbitcpp_get_c_sequence_ptr() const {
			// this cast looks nasty, and rightly so!
			return const_cast<CSEQTYPE*>(reinterpret_cast<const CSEQTYPE*>(&m_target));
		}
	};

	// --------------------- Unbounded sequence type ---------------------------
	
	template <class T, class CSEQTYPE>
	class UnboundedSequenceTmpl :
		public SequenceBase<UnboundedSequenceTmpl<T,CSEQTYPE>,T,CSEQTYPE> {
		typedef SequenceBase <UnboundedSequenceTmpl<T,CSEQTYPE>,T,CSEQTYPE> Super;
	public:

		UnboundedSequenceTmpl() {
			m_target._maximum = 0;
			m_target._length = 0;
			m_target._buffer = NULL;
			CORBA_sequence_set_release(&m_target,TRUE);
		}
		UnboundedSequenceTmpl(CORBA::ULong max) {
			m_target._maximum = max;
			m_target._length = 0;
			m_target._buffer = allocbuf(max);
			if (!m_target._buffer) throw CORBA::NO_MEMORY();
			
			CORBA_sequence_set_release(&m_target,TRUE);
		}
		UnboundedSequenceTmpl(CORBA::ULong max, CORBA::ULong length, T *value,
					 CORBA::Boolean rel = FALSE)
			: Super(max,length,value,rel) {
		}
		
		// need this because of dumb overriding rule
		CORBA::ULong length() const {
			return Super::length();
		}
		void length(CORBA::ULong len) {
			if (len > m_target._maximum) {
				T *buffer = allocbuf(len);
				if (!buffer) throw CORBA::NO_MEMORY();
				
				for (CORBA::ULong i = 0;i<m_target._length;i++) 
					buffer[i] = m_target._buffer[i];

				if (release()) 
					freebuf(m_target._buffer);
				m_target._buffer = buffer;
				m_target._maximum = len;
				_orbitcpp_set_internal_release_flag(true);
			}
			m_target._length = len;
		}
		
		// new and allocBuf defined through partial specialization
		void* operator new(size_t size);
		void operator delete(void* c_sequence) {
			CORBA_free(c_sequence);
		}
		static T *allocbuf(CORBA::ULong size);

		// copy ctor and op= as generated
	};

	// --------------------- Bounded sequence type ---------------------------
	
	template<class T, class CSEQTYPE,CORBA::ULong max>
	class BoundedSequenceTmpl :
		public SequenceBase<BoundedSequenceTmpl<T,CSEQTYPE,max>,T,CSEQTYPE,max> {
		typedef SequenceBase<BoundedSequenceTmpl<T,CSEQTYPE,max>,T,CSEQTYPE,max> Super;
	public:	
		BoundedSequenceTmpl() {
			m_target._maximum = max;
			m_target._buffer = allocbuf(max);
			if (!m_target._buffer) throw CORBA::NO_MEMORY();
			
			CORBA_sequence_set_release(&m_target,TRUE);
		}
		BoundedSequenceTmpl(CORBA::ULong length, T *value,CORBA::Boolean rel = FALSE)
			: Super(max,length,value,rel) {
		}

		// need this because of dumb overriding rule
		CORBA::ULong length() const {
			return Super::length();
		}
		void length(CORBA::ULong l) {
			m_target._length = l;
		}

		// new and allocBuf defined through partial specialization
		void* operator new(size_t size);
		void operator delete(void* c_sequence) {
			CORBA_free(c_sequence);
		}
		static T *allocbuf(CORBA::ULong size);

		// copy ctor and op= as generated
	};	
}




#define ORBITCPP_DECLARE_SIMPLE_SEQUENCE(COMPONENT,ALLOC_FUNC,ALLOCBUF_FUNC,C_NAME,NAME) \
	typedef _orbitcpp::UnboundedSequenceTmpl< COMPONENT,C_NAME> NAME; \
	inline void *NAME::operator new(size_t size) { \
		/* Keep the compiler quiet. */ \
		(void) size; \
		return ALLOC_FUNC(); \
	} \
	inline void NAME::operator delete(void *seq) { \
		CORBA_free(seq); \
	} \
	inline COMPONENT *NAME::allocbuf(CORBA::ULong len) { \
		return reinterpret_cast< COMPONENT *>( \
		  ALLOCBUF_FUNC(len)); \
	} \
	typedef ::_orbitcpp::Sequence_var< NAME> NAME##_var; \
	typedef ::_orbitcpp::Sequence_out< NAME> NAME##_out; \




#endif
