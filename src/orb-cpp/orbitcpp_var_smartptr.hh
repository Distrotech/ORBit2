/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *	ORBit-C++: C++ bindings for ORBit.
 *
 *	Copyright (C) 2000 Andreas Kloeckner
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Library General Public
 *	License as published by the Free Software Foundation; either
 *	version 2 of the License, or (at your option) any later version.
 *
 *	This library is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	Library General Public License for more details.
 *
 *	You should have received a copy of the GNU Library General Public
 *	License along with this library; if not, write to the Free
 *	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *	Author: Phil Dawes 
 *
 *  This is a hack - there is redundancy between this and
 *  orbitcpp_smartptr.hh
 */

#ifndef __ORBITCPP_SMARTPTR_VAR_HH
#define __ORBITCPP_SMARTPTR_VAR_HH

namespace _orbitcpp {

	template<class O,class O_ptr>
	class ObjectSmartPtr_var : public Dummy_var {
	private:
		O_ptr	m_objectref;
  
	public:
		ObjectSmartPtr_var()
			: m_objectref(O::_nil()) {
		}
		ObjectSmartPtr_var(O_ptr const ptr)
			: m_objectref(ptr) {
		}

		ObjectSmartPtr_var(ObjectSmartPtr_var const &objectref)
			: m_objectref(O::_duplicate(objectref.m_objectref)) {
		}

		~ObjectSmartPtr_var() {
			free();
		}

		ObjectSmartPtr_var &operator=(O_ptr const ptr) {
			reset(ptr);
			return *this;
		}
		ObjectSmartPtr_var &operator=(ObjectSmartPtr_var const &objectref_var) {
			if (this == &objectref_var) return *this; 
			reset(O::_duplicate(const_cast<ObjectSmartPtr_var &>(objectref_var)));
			return *this;
		}
	  
		O_ptr in() const {
			return m_objectref;
		}
		O_ptr &inout() {
			return m_objectref;
		}
		O_ptr &out() {
			reset(O::_nil());
			return m_objectref;
		}
		O_ptr _retn() {
			O_ptr old = m_objectref;
			m_objectref = O::_nil();
			return old;
		}
  
		operator O_ptr &() {
			return m_objectref;
		}
/*
  // GCC spews warnings if we include this
		operator O_ptr const &() const {
			return m_objectref;
		}
*/
		O_ptr operator->() const {
			return m_objectref;
		}

		
	private:
		void free() {
			::CORBA::release(m_objectref);
			m_objectref = O::_nil();
		}
		
		void reset(O_ptr ptr) {
			free();
			m_objectref = ptr;
		}
		void operator=(Dummy_var const &oops);
		// not to be implemented
	};

}
  
#endif
