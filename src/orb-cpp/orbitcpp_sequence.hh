// -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
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
 *  Authors: Phil Dawes <philipd@users.sourceforge.net>
 *           Andreas Kloeckner <ak@ixion.net>
 *           Gergõ Érdi <cactus@cactus.rulez.org>
 */

#ifndef __ORBITCPP_SEQUENCE_HH
#define __ORBITCPP_SEQUENCE_HH

#include "orbitcpp_types.hh"
#include <glib.h> // for g_assert

namespace _orbitcpp {

    template<typename CPPElem, typename CElem, typename CType, typename Traits>
    class SequenceBase
    {
    public:
		typedef CPPElem                                      value_t;
		typedef CElem                                        c_value_t;
		typedef SequenceBase<CPPElem, CElem, CType, Traits>  self_t;
		typedef value_t                                     *buffer_t;
		typedef CType                                        c_seq_t;
		typedef Traits                                       traits_t;
		
		typedef CORBA::ULong size_t;
		typedef CORBA::ULong index_t;
		
    protected:
		size_t   _max;
		size_t   _length;
		buffer_t _buffer;
		bool     _release;
		
    public:
	
		// Empty constructor
		SequenceBase (size_t max = 0):
	    _max (max),
	    _length (0),
	    _buffer (max ? new value_t[max] : 0),
	    _release (true)
			{
			}
		
		// Create sequence from flat buffer
		SequenceBase (size_t max,
					  size_t length, buffer_t buffer,
					  CORBA::Boolean release = false):
			_max (max),
			_length (length),
			_buffer (buffer),
			_release (release)
			{
				g_assert (length <= max);
			}
		
		// Copy constructor
		SequenceBase (const self_t &src):
			_max (src._max),
			_length (src._length),
			_release (true)
			{
				_buffer = allocbuf (_max);
				if (!_buffer)
					throw CORBA::NO_MEMORY ();
				
				for (index_t i = 0; i < _length; i++)
					_buffer[i] = src._buffer[i];
			}
		
		// Copy operator
		self_t & operator= (const self_t &src) {
			buffer_t buffer_tmp = 0;
			
			if (src._max != 0) {
				buffer_tmp = allocbuf (src._max);
				if (!buffer_tmp)
					throw CORBA::NO_MEMORY ();
			}
			
			for (index_t i = 0; i < src._length; i++)
				buffer_tmp[i] = src._buffer[i];
			
			_length = src._length;
			_max = src._max;
			
			if (_release)
				freebuf (_buffer);
			_buffer = buffer_tmp;
	    
			return *this;
		}
		
		// Destructor
		virtual ~SequenceBase () {
			if (_release)
				freebuf (_buffer);
		}
		
		// Size information
		size_t maximum () const { return _max;    };
		size_t length () const  { return _length; };
		virtual void length (size_t new_length) = 0;
		
		// Element access
		value_t& operator[] (index_t index)             { return _buffer[index]; };
		const value_t& operator[] (index_t index) const { return _buffer[index]; };
		
		// Memory managment
		static buffer_t allocbuf (size_t num_elems) { return new value_t[num_elems]; };
		static void freebuf (buffer_t buffer)       { delete buffer;                 };    
		
		// ORBit2/C++ extension: create C sequence
		c_seq_t* _orbitcpp_pack () const {
			c_seq_t *retval = traits_t().alloc_c ();
			_orbitcpp_pack (*retval);
			return retval;
		}
		
		// ORBit2/C++ extension: fill C sequence
		void _orbitcpp_pack (c_seq_t &c_seq) const {
			c_seq._length = _length;
			c_seq._buffer = traits_t().alloc_c_buf (_length);
			for (index_t i = 0; i < _length; i++)
				traits_t().pack_elem (_buffer[i], c_seq._buffer[i]);
		}
		
		// ORBit2/C++ extension: fill C++ sequence from C sequence
		void _orbitcpp_unpack (const c_seq_t &c_seq) {
			length (c_seq._length);
			for (index_t i = 0; i < c_seq._length; i++)
				traits_t().unpack_elem (_buffer[i], c_seq._buffer[i]);
		}
    };
    
    template<typename CPPElem, typename CElem, typename CType, typename Traits>
    class UnboundedSequence: public SequenceBase<CPPElem, CElem, CType, Traits>
    {
		typedef SequenceBase<CPPElem, CElem, CType, Traits> Super;
		
    public:
		// Empty constructor
		UnboundedSequence (size_t max = 0):
			Super (max)
			{
			}
		
		// Create sequence from flat buffer
	UnboundedSequence (size_t max,
					   size_t length, buffer_t buffer,
					   CORBA::Boolean release = false):
	    Super (max, length, buffer, release)
			{
			}
	
		// Copying
		UnboundedSequence (const self_t &other):
			Super (other)
			{
			}
		
		// Size information
		size_t maximum () const { return _max;    };
		size_t length () const  { return _length; };
		
		// Size requisition
		void length (size_t new_length) {
			if (new_length > _max)
			{
				buffer_t buffer_tmp = allocbuf (new_length);
				if (!buffer_tmp)
					throw CORBA::NO_MEMORY ();
		
				for (index_t i = 0; i < _length; i++)
					buffer_tmp[i] = _buffer[i];
				
				if (_release)
					freebuf (_buffer);
				
				_buffer = buffer_tmp;
				_max = new_length;
			}
			
			_length = new_length;
		}
    };
    
    template<typename CPPElem, typename CElem, typename CType, typename Traits,
	     SequenceBase<CPPElem, CElem, CType, Traits>::size_t max>
    class BoundedSequence: public SequenceBase<CPPElem, CElem, CType, Traits>
    {
		typedef SequenceBase<CPPElem, CElem, CType, Traits> Super;
		
    public:
		// Empty constructor
		BoundedSequence ():
			Super (max)
			{
			}
		
		// Create sequence from flat buffer
		BoundedSequence (size_t length, buffer_t buffer,
						 CORBA::Boolean release = false):
			Super (max, length, buffer, release)
			{
			}
		
		// Copying
		BoundedSequence (const self_t &other):
			Super (other)
			{
			}
		
		// Size requisition
		void length (size_t new_length) {
			g_assert (new_length <= _max);
			
			_length = new_length;
		}
    };
} // namespace _orbitcpp

#define ORBITCPP_DECLARE_SIMPLE_SEQUENCE(CPPType, CPPElem, CElem, CType)	\
    struct CPPType##_traits							\
    {										\
	CType* alloc_c () const {						\
	    return CType##__alloc ();						\
	}									\
										\
        CElem* alloc_c_buf (CORBA::ULong length) const {			\
	    return (CElem*)CType##_allocbuf (length);				\
	}									\
										\
	void pack_elem (CPPElem &cpp_value, CElem &c_value) const {		\
	    c_value = (CElem) cpp_value;					\
	}									\
										\
	void unpack_elem (CPPElem &cpp_value, CElem &c_value) const {		\
	    cpp_value = (CPPElem) c_value;					\
	}									\
    };										\
										\
    typedef ::_orbitcpp::UnboundedSequence<CPPElem, CElem, CType, CPPType##_traits> CPPType; \
    typedef ::_orbitcpp::Sequence_var< CPPType > CPPType##_var;			\
    typedef ::_orbitcpp::Sequence_out< CPPType > CPPType##_out;

#endif // !__ORBITCPP_SEQUENCE_HH
