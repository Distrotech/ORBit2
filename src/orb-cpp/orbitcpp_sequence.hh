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
#include "orbitcpp_exception.hh"
#include <orbit/orb-core/corba-defs.h>
#include <glib.h> // for g_assert

namespace _orbitcpp {

    template<typename CPPElem, typename CElem, typename CSeq>
    class SequenceBase
    {
    public:
	typedef SequenceBase<CPPElem, CElem, CSeq> self_t;
		
	typedef CPPElem       value_t;
	typedef CElem         c_value_t;
	typedef CSeq          c_seq_t;
	typedef value_t      *buffer_t;
	
	typedef CORBA::ULong  size_t;
	typedef CORBA::ULong  index_t;

    protected:
	size_t   _max;
	size_t   _length;
	buffer_t _buffer;
	bool     _release;
	
    public:	
	// Empty constructor
	explicit SequenceBase (size_t max = 0):
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
	static void     freebuf  (buffer_t buffer)  { delete[] buffer;               };
	
	// ORBit2/C++ extension: create C sequence
	c_seq_t* _orbitcpp_pack () const {
	    c_seq_t *retval = alloc_c ();
	    _orbitcpp_pack (*retval);
	    return retval;
	}
	
	// ORBit2/C++ extension: fill C sequence
	void _orbitcpp_pack (c_seq_t &c_seq) const {
	    c_seq._length = _length;
	    if (c_seq._release)
		CORBA_free (c_seq._buffer);
	    c_seq._buffer = alloc_c_buf (_length);
	    c_seq._release = 1;
	    
	    for (index_t i = 0; i < _length; i++)
		pack_elem (_buffer[i], c_seq._buffer[i]);
	}
	
	// ORBit2/C++ extension: fill C++ sequence from C sequence
	void _orbitcpp_unpack (const c_seq_t &c_seq) {
	    length (c_seq._length);
	    for (index_t i = 0; i < c_seq._length; i++)
		unpack_elem (_buffer[i], c_seq._buffer[i]);
	}
	
    protected:
	// ORBit2/C++ extension: memory managment of underlying C sequences
        virtual c_seq_t*   alloc_c () const = 0;
	virtual c_value_t* alloc_c_buf (size_t length) const = 0;

	virtual void pack_elem   (const value_t& cpp_value, c_value_t &c_value) const = 0;
	virtual void unpack_elem (value_t& cpp_value, const c_value_t &c_value) const = 0;
    };


    /////////////////////////////////////////////////////////////////////

    
    template<class Traits>
    class SimpleSeqBase: public SequenceBase <typename Traits::value_t,
	typename Traits::c_value_t, typename Traits::c_seq_t>
    {
	typedef Traits                       traits_t;
	typedef typename Traits::value_t     value_t;
	typedef typename traits_t::c_value_t c_value_t;
	typedef typename traits_t::c_seq_t   c_seq_t;
	
	typedef SequenceBase<value_t, c_value_t, c_seq_t> Super;
	typedef Super super_t;

	typedef SimpleSeqBase<traits_t> self_t;
	
	typedef typename Super::size_t   size_t;
	typedef typename Super::index_t  index_t;
	typedef typename Super::buffer_t buffer_t;
	
    public:
	// Empty constructor
	explicit SimpleSeqBase (size_t max = 0):
	    Super (max)
	    {
	    }
	
	// Create sequence from flat buffer
	SimpleSeqBase (size_t max,
		       size_t length, buffer_t buffer,
		       CORBA::Boolean release = false):
	    Super (max, length, buffer, release)
	    {
	    }
		
	// Copying
	SimpleSeqBase (const self_t &other):
	    Super (other)
	    {
	    }

    protected:
        c_seq_t* alloc_c () const {
	    return traits_t::alloc_c ();
	}
	
	c_value_t* alloc_c_buf (size_t length) const {
	    return traits_t::alloc_c_buf (length);
	}

	void pack_elem (const value_t& cpp_value, c_value_t &c_value) const {
	    c_value = (c_value_t) (cpp_value);
	}
	
	void unpack_elem (value_t& cpp_value, const c_value_t &c_value) const {
	    cpp_value = (value_t) (c_value);
	}
    };

    template<class Traits>
    class SimpleUnboundedSeq: public SimpleSeqBase<Traits>
    {
	typedef SimpleSeqBase<Traits>    Super;

	typedef typename Super::size_t   size_t;
	typedef typename Super::index_t  index_t;
	typedef typename Super::buffer_t buffer_t;
	typedef Super                    super_t;
	
    public:
	// Empty constructor
	explicit SimpleUnboundedSeq (size_t max = 0):
	    Super (max)
	    {
	    }
	
	// Create sequence from flat buffer
	SimpleUnboundedSeq (size_t max,
			    size_t length, buffer_t buffer,
			    CORBA::Boolean release = false):
	    Super (max, length, buffer, release)
	    {
	    }
		
	// Copying
	SimpleUnboundedSeq (const super_t &other):
	    Super (other)
	    {
	    }
	
	// Size information
	using Super::length;
	using Super::maximum;
#if 0
	size_t maximum () const { return _max;    };
	size_t length ()  const { return _length; };
#endif
	
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
		
		_release = true;
		_buffer = buffer_tmp;
		_max = new_length;
	    }
	    
	    _length = new_length;
	}
    };

    template<class Traits, CORBA::ULong max>
    class SimpleBoundedSeq: public SimpleSeqBase<Traits>
    {
	typedef SimpleSeqBase<Traits>    Super;

	typedef typename Super::size_t   size_t;
	typedef typename Super::index_t  index_t;
	typedef typename Super::buffer_t buffer_t;
	typedef Super                    super_t;
	
    public:
	// Empty constructor
	SimpleBoundedSeq ():
	    Super (max)
	    {
	    }
		
	// Create sequence from flat buffer
	SimpleBoundedSeq (size_t length, buffer_t buffer,
			  CORBA::Boolean release = false):
	    Super (max, length, buffer, release)
	    {
	    }
		
	// Copying
	SimpleBoundedSeq (const super_t &other):
	    Super (other)
	    {
	    }
	
	// Size information
	using Super::length;
	using Super::maximum;
	
	// Size requisition
	void length (size_t new_length) {
	    g_assert (new_length <= _max);
	    
	    _length = new_length;
	}
    };
    
    /////////////////////////////////////////////////////////////////////

    
    template<typename CPPElem>
    class CompoundSeqBase: public SequenceBase<typename CPPElem::SeqTraits::value_t,
	typename CPPElem::SeqTraits::c_value_t,
	typename CPPElem::SeqTraits::c_seq_t>
    {
	typedef typename CPPElem::SeqTraits  traits_t;
	typedef typename traits_t::value_t   value_t;
	typedef typename traits_t::c_value_t c_value_t;
	typedef typename traits_t::c_seq_t   c_seq_t;
	
	typedef SequenceBase<value_t, c_value_t, c_seq_t> Super;
	typedef Super super_t;

	typedef CompoundSeqBase<CPPElem> self_t;

	typedef typename Super::size_t   size_t;
	typedef typename Super::index_t  index_t;
	typedef typename Super::buffer_t buffer_t;
	
    public:
	// Empty constructor
	explicit CompoundSeqBase (size_t max = 0):
	    Super (max)
	    {
	    }
	
	// Create sequence from flat buffer
	CompoundSeqBase (size_t max,
			 size_t length, buffer_t buffer,
			 CORBA::Boolean release = false):
	    Super (max, length, buffer, release)
	    {
	    }
		
	// Copying
	CompoundSeqBase (const self_t &other):
	    Super (other)
	    {
	    }

    protected:
        c_seq_t* alloc_c () const {
	    return traits_t::alloc_c ();
	}
	
	c_value_t* alloc_c_buf (size_t length) const {
	    return traits_t::alloc_c_buf (length);
	}

	void pack_elem (const value_t &cpp_value, c_value_t &c_value) const {
	    traits_t::pack_elem (cpp_value, c_value);
	}
	
	void unpack_elem (value_t &cpp_value, const c_value_t &c_value) const {
	    traits_t::unpack_elem (cpp_value, c_value);
	}
    };
    
    template<typename CPPElem>
    class CompoundUnboundedSeq: public CompoundSeqBase<CPPElem>
    {
	typedef typename CPPElem::SeqTraits  traits_t;
	//typedef typename traits_t::value_t   value_t;
	typedef typename traits_t::c_value_t c_value_t;
	typedef typename traits_t::c_seq_t   c_seq_t;
	
	typedef CompoundSeqBase<CPPElem> Super;
	typedef typename Super::value_t value_t;
	typedef SequenceBase<value_t, c_value_t, c_seq_t> super_t;

	
	typedef typename Super::size_t   size_t;
	typedef typename Super::index_t  index_t;
	typedef typename Super::buffer_t buffer_t;
	
    public:
	// Empty constructor
	explicit CompoundUnboundedSeq (size_t max = 0):
	    Super (max)
	    {
	    }
	
	// Create sequence from flat buffer
	CompoundUnboundedSeq (size_t max,
			 size_t length, buffer_t buffer,
			 CORBA::Boolean release = false):
	    Super (max, length, buffer, release)
	    {
	    }
		
	// Copying
	CompoundUnboundedSeq (const super_t &other):
	    Super (other)
	    {
	    }
	
	// Size information
	using Super::length;
	using Super::maximum;
#if 0
	size_t maximum () const { return _max;    };
	size_t length ()  const { return _length; };
#endif
	
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
		
		_release = true;
		_buffer = buffer_tmp;
		_max = new_length;
	    }
	    
	    _length = new_length;
	}
    };
    
    template<typename CPPElem, CORBA::ULong max>
    class CompoundBoundedSeq: public CompoundSeqBase<CPPElem>
    {
	typedef typename CPPElem::SeqTraits traits_t;
	typedef typename traits_t::value_t  value_t;
	typedef typename traits_t::c_elem_t c_value_t;
	typedef typename traits_t::c_seq_t  c_seq_t;
	
	typedef CompoundSeqBase<value_t> Super;
	typedef Super super_t;
	
	typedef typename Super::size_t   size_t;
	typedef typename Super::index_t  index_t;
	typedef typename Super::buffer_t buffer_t;
	
    public:
	// Empty constructor
	CompoundBoundedSeq ():
	    Super (max)
	    {
	    }
		
	// Create sequence from flat buffer
	CompoundBoundedSeq (size_t length, buffer_t buffer,
			    CORBA::Boolean release = false):
	    Super (max, length, buffer, release)
	    {
	    }
		
	// Copying
	CompoundBoundedSeq (const super_t &other):
	    Super (other)
	    {
	    }
	
	// Size information
	using Super::length;
	using Super::maximum;
	
	// Size requisition
	void length (size_t new_length) {
	    g_assert (new_length <= _max);
	    
	    _length = new_length;
	}
    };

#if 0
    
    template<typename CPPElem, typename CElem, typename ElemTraits,
		     typename CSeq, CORBA::ULong max>
    class BoundedSequence: public SequenceBase<CPPElem, CElem, ElemTraits, CSeq>
    {
	public:
		typedef SequenceBase<CPPElem, CElem, ElemTraits, CSeq> Super;
		typedef typename Super::self_t   super_t;

		typedef typename Super::buffer_t buffer_t;
		
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
		BoundedSequence (const super_t &other):
			Super (other)
			{
			}
		
		// Size information
		size_t maximum () const { return _max;    };
		size_t length () const  { return _length; };
		
		// Size requisition
		void length (size_t new_length) {
			g_assert (new_length <= _max);
			
			_length = new_length;
		}
    };
#endif
    
} // namespace _orbitcpp



#define ORBITCPP_DECLARE_SIMPLE_SEQUENCE(CPPType, CElem)	\
    typedef ::_orbitcpp::SimpleUnboundedSeq< ::_orbitcpp::seq_traits::__##CElem > CPPType; \
    typedef ::_orbitcpp::Sequence_var< CPPType > CPPType##_var;	\
    typedef ::_orbitcpp::Sequence_out< CPPType > CPPType##_out;

#define ORBITCPP_CREATE_SIMPLE_TRAITS(CPPElem, CElem)			\
    namespace _orbitcpp {						\
	namespace seq_traits {						\
	    struct __##CElem {						\
		typedef CPPElem                value_t;			\
		typedef CElem                  c_value_t;		\
		typedef CORBA_sequence_##CElem c_seq_t;			\
									\
		static c_seq_t* alloc_c () {				\
		    return CORBA_sequence_##CElem##__alloc ();		\
		}							\
									\
		static c_value_t* alloc_c_buf (CORBA::ULong l) {	\
		    return CORBA_sequence_##CElem##_allocbuf (l);	\
		}							\
	    };								\
	}								\
    }

    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::Char,    CORBA_char);
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::WChar,   CORBA_wchar);
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::Boolean, CORBA_boolean);
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::Octet,   CORBA_octet);
    
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::Short,    CORBA_short);
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::Long,     CORBA_long);
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::LongLong, CORBA_long_long);
    
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::UShort,    CORBA_unsigned_short);
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::ULong,     CORBA_unsigned_long);
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::ULongLong, CORBA_unsigned_long_long);
    
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::Float,      CORBA_float);
    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::Double,     CORBA_double);

// This one is not generated in corba-defs.h
//    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::LongDouble, CORBA_long_double);

#undef ORBITCPP_CREATE_SIMPLE_TRAITS

#endif // !__ORBITCPP_SEQUENCE_HH

