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
    
} // namespace _orbitcpp
    
#endif // !__ORBITCPP_SEQUENCE_HH

