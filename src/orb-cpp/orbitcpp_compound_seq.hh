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

#ifndef __ORBITCPP_COMPOUND_SEQUENCE_HH
#define __ORBITCPP_COMPOUND_SEQUENCE_HH

#include "orbitcpp_types.hh"
#include "orbitcpp_exception.hh"
#include <orbit/orb-core/corba-defs.h>
#include <glib.h> // for g_assert

namespace _orbitcpp {

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
} // namespace _orbitcpp

#endif // !__ORBITCPP_COMPOUND_SEQUENCE_HH

