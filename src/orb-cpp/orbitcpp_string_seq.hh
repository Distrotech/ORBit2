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

#ifndef __ORBITCPP_STRING_SEQUENCE_HH
#define __ORBITCPP_STRING_SEQUENCE_HH

#include "orbitcpp_sequence.hh"
#include "orbitcpp_smartptr.hh"
#include <orbit/orb-core/corba-defs.h>

namespace _orbitcpp {

    class StringSeqBase: public SequenceBase <CORBA::String_mgr,
			 CORBA_char*, CORBA_sequence_CORBA_string>
    {
    protected:
	typedef CORBA::String_mgr           value_t;
	typedef CORBA_char*                 c_value_t;
	typedef CORBA_sequence_CORBA_string c_seq_t;
	
	typedef SequenceBase<value_t, c_value_t, c_seq_t> Super;
	typedef Super super_t;
	
	typedef StringSeqBase self_t;
	
	typedef Super::size_t   size_t;
	typedef Super::index_t  index_t;
	typedef Super::buffer_t buffer_t;
	
    public:
	// Empty constructor
	explicit StringSeqBase (size_t max = 0):
	    Super (max)
	    {
	    }
	
	// Create sequence from flat buffer
	StringSeqBase (size_t max,
		       size_t length, buffer_t buffer,
		       CORBA::Boolean release = false):
	    Super (max, length, buffer, release)
	    {
	    }
	
	// Copying
	StringSeqBase (const self_t &other):
	    Super (other)
	    {
	    }
	
    protected:
        c_seq_t* alloc_c () const {
	    return CORBA_sequence_CORBA_string__alloc ();
	}
	
	c_value_t* alloc_c_buf (size_t length) const {
	    return CORBA_sequence_CORBA_string_allocbuf (length);
	}
	
	void pack_elem (const value_t& cpp_value, c_value_t &c_value) const {
	    c_value = CORBA::string_dup (cpp_value);
	}
	
	void unpack_elem (value_t& cpp_value, const c_value_t &c_value) const {
	    cpp_value = CORBA::string_dup (c_value);
	}
    };

    class StringUnboundedSeq: public StringSeqBase
    {
	typedef StringSeqBase            Super;
	
	typedef Super::size_t   size_t;
	typedef Super::index_t  index_t;
	typedef Super::buffer_t buffer_t;
	typedef Super           super_t;
	
    public:
	// Empty constructor
	explicit StringUnboundedSeq (size_t max = 0):
	    Super (max)
	    {
	    }
	
	// Create sequence from flat buffer
	StringUnboundedSeq (size_t max,
			    size_t length, buffer_t buffer,
			    CORBA::Boolean release = false):
	    Super (max, length, buffer, release)
	    {
	    }
		
	// Copying
	StringUnboundedSeq (const super_t &other):
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

    template<CORBA::ULong max>
    class StringBoundedSeq: public StringSeqBase
    {
	typedef StringSeqBase   Super;

	typedef Super::size_t   size_t;
	typedef Super::index_t  index_t;
	typedef Super::buffer_t buffer_t;
	typedef Super           super_t;
	
    public:
	// Empty constructor
	StringBoundedSeq ():
	    Super (max)
	    {
	    }
		
	// Create sequence from flat buffer
	StringBoundedSeq (size_t length, buffer_t buffer,
			  CORBA::Boolean release = false):
	    Super (max, length, buffer, release)
	    {
	    }
		
	// Copying
	StringBoundedSeq (const super_t &other):
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

#endif // !__ORBITCPP_STRING_SEQUENCE_HH

