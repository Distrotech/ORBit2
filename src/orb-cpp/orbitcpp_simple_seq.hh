// -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*-
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

#ifndef __ORBITCPP_SIMPLE_SEQUENCE_HH
#define __ORBITCPP_SIMPLE_SEQUENCE_HH

#include "orbitcpp_sequence.hh"
#include <orbit/orb-core/corba-defs.h>

namespace _orbitcpp {

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

// This one is not generated in corba-defs.h -- cactus
//    ORBITCPP_CREATE_SIMPLE_TRAITS(CORBA::LongDouble, CORBA_long_double);

#undef ORBITCPP_CREATE_SIMPLE_TRAITS

#endif // !__ORBITCPP_SIMPLE_SEQUENCE_HH

