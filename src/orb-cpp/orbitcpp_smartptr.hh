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
 *	Author: Andreas Kloeckner <ak@ixion.net>
 *          Phil Dawes <philipd@users.sourceforge.net>
 *			John K. Luebs <jkluebs@marikarpress.com>
 */




#ifndef __ORBITCPP_SMARTPTR_HH
#define __ORBITCPP_SMARTPTR_HH




#include "orbitcpp_types.hh"
#include "orbitcpp_tools.hh"

namespace CORBA {
	class Object;
}


#define ORBITCPP_MAKEREFTYPES(type) \
	typedef type										*type##_ptr; \
	typedef _orbitcpp::ObjectPtr_var<type,type##_ptr>	type##_var; \
	typedef _orbitcpp::ObjectPtr_out<type,type##_ptr>	type##_out; \
	typedef type##_ptr									type##Ref; 




// Object smart pointers ------------------------------------------------------
namespace _orbitcpp {
	// according to spec 20.3.7
	class Dummy_var { };
	
	template<class O,class O_ptr>
	class ObjectPtr_var : public Dummy_var {
	private:
		O_ptr	m_objectref;

	public:
		ObjectPtr_var()
			: m_objectref(O::_nil()) {
		}
		ObjectPtr_var(O_ptr const ptr)
			: m_objectref(ptr) {
		}

		ObjectPtr_var(ObjectPtr_var const &objectref)
			: m_objectref(O::_duplicate(objectref.m_objectref)) {
		}

		~ObjectPtr_var() {
			free();
		}

		ObjectPtr_var &operator=(O_ptr const ptr) {
			reset(ptr);
			return *this;
		}
		ObjectPtr_var &operator=(ObjectPtr_var const &objectref_var) {
			if (this == &objectref_var) return *this; 
			reset(O::_duplicate(const_cast<ObjectPtr_var &>(objectref_var)));
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
			if (m_objectref != O::_nil ())
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
  
  
  
  
	// according to spec 20.3.6
	template<class O,class O_ptr>
	class ObjectPtr_out {
	protected:
		O_ptr	&m_objectref;
  
	public:
		ObjectPtr_out(O_ptr &ptr)
			: m_objectref(ptr) {
			ptr = O::_nil();
		}
		ObjectPtr_out(ObjectPtr_var<O,O_ptr> &var)
			: m_objectref(var) {
			::CORBA::release(m_objectref);
			m_objectref = O::_nil();
		}
		ObjectPtr_out(ObjectPtr_out const & out) 
			: m_objectref(out.m_objectref) {
		}
  
		ObjectPtr_out &operator=(ObjectPtr_out &out) {
			m_objectref = out.m_objectref;
			return *this;
		}
		ObjectPtr_out &operator=(ObjectPtr_var<O,O_ptr> &var) {
			m_objectref = O::_duplicate(var.operator O_ptr &());
			return *this;
		}
		ObjectPtr_out &operator=(O_ptr ptr) {
			m_objectref = ptr;
			return *this;
		}
/*
  // GCC spews warnings if we include this		  
		operator O_ptr const &() const {
			return m_objectref;
		}
*/			
		operator O_ptr &() {
			return m_objectref;
		}
		operator CORBA_Object *() {
			return static_cast<CORBA_Object*>(&m_objectref);
		}
		operator CORBA_TypeCode *() {
			return static_cast<CORBA_TypeCode*>(&m_objectref);
		}
		O_ptr ptr() const {
			return m_objectref;
		}
		O_ptr operator->() const{
			return m_objectref;
		}

	};
}  
  
// Data smart pointers --------------------------------------------------------
namespace _orbitcpp {
	template<class T>
	class Data_out;
  
	template<class T>
	class Data_var {
		friend class Data_out<T>;
	protected:
		T		*m_data;
		
	public:
		// construction and destruction
		Data_var()
			: m_data(NULL) {
		}
		Data_var(T *data)
			: m_data(data) {
		}
		Data_var(Data_var const &src)
			: m_data(new T(*src.m_data)) {
		}
		~Data_var() {
			if (m_data) delete m_data;
		}
		
		// assignment
		Data_var &operator=(T *data) {
			if (m_data) delete m_data;
			m_data = data;
			return *this;
		}
		Data_var &operator=(Data_var const &src) {
			T *copy = new T(*src.m_data);
			if (m_data) delete m_data;
			m_data = copy;
			return *this;
		}
		
		// access
		T *operator->() {
			return m_data;
		}
		T const *operator->() const {
			return m_data;
		}
		operator T &() {
			return *m_data;
		}
/*
  // GCC spews warnings if we include this
		operator T const &() const {
			return *m_data;
		}
*/
		operator T *() {
			return _retn();
		}
		
		// parameter passing conversion
		T const &in() const {
			return *m_data;
		}
		T &inout() {
			return *m_data;
		}
		T &out() {
			if (m_data) {
				delete m_data;
				m_data = NULL;
			}
			return *m_data;
		}
		T *_retn() {
			T *temp = m_data;
			m_data = NULL;
			return temp;
		}
	};
  
	template<class T>
	class Data_out {
	protected:
		T *&m_data;
	  
	public:
		// constructors and destructors
		Data_out(T *&data)
			: m_data(data) {
		}
		Data_out(Data_var<T> &var)
			: m_data(var.m_data) {
			delete m_data;
			m_data = NULL;
		}
		Data_out(Data_out const & src)
			: m_data(src.m_data) {
		}
		~Data_out() {
		}
		
		// assignment
		Data_out &operator=(Data_out &src) {
			m_data = src.m_data;
			return src;
		}
		Data_out &operator=(T *src) {
			m_data = src;
			return *this;
		}
		
		// access
		operator T *&() {
			return m_data;
		}
		T *operator->() {
			return m_data;
		}
		T *&ptr() {
			return m_data;
		}
	private: // forbidden
		void operator=(Data_var<T> &);
	};

	template<class T>
	class DataVar_out;
  
	template<class T>
	class DataVar_var {
		friend class DataVar_out<T>;
	protected:
		T		*m_data;
		
	public:
		// construction and destruction
		DataVar_var()
			: m_data(NULL) {
		}
		DataVar_var(T *data)
			: m_data(data) {
		}
		DataVar_var(DataVar_var const &src)
			: m_data(new T(*src.m_data)) {
		}
		~DataVar_var() {
			if (m_data) delete m_data;
		}
		
		// assignment
		DataVar_var &operator=(T *data) {
			if (m_data) delete m_data;
			m_data = data;
			return *this;
		}
		DataVar_var &operator=(DataVar_var const &src) {
			T *copy = new T(*src.m_data);
			if (m_data) delete m_data;
			m_data = copy;
			return *this;
		}
		
		// access
		T *operator->() {
			return m_data;
		}
		T const *operator->() const {
			return m_data;
		}
		operator T &() {
			return *m_data;
		}
/*
  // GCC spews warnings if we include this
		operator T const &() const {
			return *m_data;
		}
*/
		operator T *() {
			return _retn();
		}
		
		// parameter passing conversion
		T const &in() const {
			return *m_data;
		}
		T &inout() {
			return *m_data;
		}
		T *&out() {
			if (m_data) {
				delete m_data;
				m_data = NULL;
			}
			return m_data;
		}
		T *_retn() {
			T *temp = m_data;
			m_data = NULL;
			return temp;
		}
	};
  
	template<class T>
	class DataVar_out {
	protected:
		T *&m_data;
	  
	public:
		// constructors and destructors
		DataVar_out(T *&data)
			: m_data(data) {
		}
		DataVar_out(DataVar_var<T> &var)
			: m_data(var.m_data) {
			delete m_data;
			m_data = NULL;
		}
		DataVar_out(DataVar_out const & src)
			: m_data(src.m_data) {
		}
		~DataVar_out() {
		}
		
		// assignment
		DataVar_out &operator=(DataVar_out &src) {
			m_data = src.m_data;
			return src;
		}
		DataVar_out &operator=(T *src) {
			m_data = src;
			return *this;
		}
		
		// access
		operator T *&() {
			return m_data;
		}
		T *operator->() {
			return m_data;
		}
		T *&ptr() {
			return m_data;
		}
	private: // forbidden
		void operator=(DataVar_var<T> &);
	};
}



// Sequence smart pointers ----------------------------------------------------
namespace _orbitcpp {
	template<class T>
	class Sequence_var : public Data_var<T> {
	protected:
		typedef typename T::value_t value_t;
		typedef typename T::index_t index_t;
		typedef Data_var<T>         Super;

	public:
		Sequence_var () {
			}
		Sequence_var (T *data)
			: Super(data) {
	  	}
		Sequence_var (Sequence_var const &src)
			: Super(src) {
		}

		Sequence_var &operator= (T *data) {
			if (m_data) delete m_data;
			m_data = data;
			return *this;
		}

		Sequence_var &operator= (Sequence_var const &src) {
			T *copy = new T(*src.m_data);
			if (m_data) delete m_data;
			m_data = copy;
			return *this;
		}
		
		value_t& operator[] (index_t index) {
			return m_data->operator[](index);
	  	}
		const value_t& operator[] (index_t index) const {
			return m_data->operator[](index);
	  	}
	};
  
	template<class T>
	class Sequence_out : public Data_out<T>  {
	  protected:
		typedef typename T::value_t value_t;
		typedef typename T::index_t index_t;
		typedef Data_out<T>         Super;
  
	  public:
		Sequence_out(T *&data)	: Super(data) {}
		Sequence_out(Data_var<T> &var)	: Super(var) {}
		Sequence_out(Sequence_out const & src)	: Super(src) {}

		// assignment
		Sequence_out &operator= (Sequence_out &src) {
			m_data = src.m_data;
			return src;
		}
		Sequence_out &operator= (T *src) {
			m_data = src;
			return *this;
		}
		
		value_t& operator[] (index_t index) {
			return (*m_data)[index];
	  	}
		const value_t& operator[] (index_t index) const {
			return (*m_data)[index];
	  	}
	};
}


// Array smart pointers -------------------------------------------------------



namespace _orbitcpp {
	
	template<class T_slice, CORBA::ULong len>
	struct ArrayProperties { 		
		static T_slice *alloc();
		static void free(T_slice *array);
		static void copy(T_slice *dest, T_slice const *src);
	};

	template<class T_slice, CORBA::ULong len>
	class ArrayFixed_var {
	private:
		T_slice *m_data;

		typedef ArrayProperties<T_slice, len> Properties;
	public:

		ArrayFixed_var() {
			// This initialization is needed because an ArrayVar may be
			// used as an out-parameter without prior initialization
			// (see example in [16.12])
			m_data = Properties::alloc();  
		}
		ArrayFixed_var(T_slice *data) {
			m_data = data;
	  	}
		ArrayFixed_var(ArrayFixed_var<T_slice, len> const &src){
			m_data = Properties::alloc();
			Properties::copy(m_data,src.m_data);
		}

		ArrayFixed_var &operator=(T_slice *data) {
			if (m_data) Properties::free(m_data);
			m_data = data;
			return *this;
		}

		ArrayFixed_var &operator=(ArrayFixed_var<T_slice, len> const &src) {
			Properties::copy(m_data,src.m_data);
			return *this;
		}

		~ArrayFixed_var() {
			if (m_data) Properties::free(m_data);
		}

		T_slice &operator[](CORBA::ULong index) {
			g_assert (m_data);
			return m_data[index];
	  	}
		
		T_slice const &operator[](CORBA::ULong index) const {
			g_assert (m_data);
			return m_data[index];
	  	}

		operator T_slice *() {
			return m_data;
		}
		
/*
  // GCC spews warnings if we include this
		operator T_slice const *() const {
			return m_data;
		}
*/
		// parameter passing conversion
		T_slice const *in() const {
			return m_data;
		}
		T_slice *inout() {
			return m_data;
		}
		T_slice *out() {
			return m_data;
		}
		T_slice *_retn() {
			T_slice *temp = m_data;
			m_data = NULL;
			return temp;
		}		
	};

	template<class T_slice, CORBA::ULong len>
	class ArrayVariable_out;

	template<class T_slice, CORBA::ULong len>
	class ArrayVariable_var {
	private:
		T_slice *m_data;
		typedef ArrayProperties<T_slice, len> Properties;
		friend class ArrayVariable_out<T_slice, len>;
	public:

		ArrayVariable_var() {
			// This initialization is needed because an ArrayVar may be
			// used as an out-parameter without prior initialization
			// (see example in [16.12])
			m_data = Properties::alloc();  
		}
		ArrayVariable_var(T_slice *data) {
			m_data = data;
	  	}
		ArrayVariable_var(ArrayVariable_var<T_slice, len> const &src){
			m_data = Properties::alloc();
			Properties::copy(m_data,src.m_data);
		}

		~ArrayVariable_var() {
			if (m_data) Properties::free(m_data);
		}

		
		ArrayVariable_var &operator=(T_slice *data) {
			if (m_data) Properties::free(m_data);
			m_data = data;
			return *this;
		}

		ArrayVariable_var &operator=(ArrayVariable_var<T_slice, len> const &src) {
			Properties::copy(m_data,src.m_data);
			return *this;
		}

		T_slice& operator[] (CORBA::UShort index) {
			g_assert (m_data);
			return m_data[index];
	  	}
		const T_slice& operator[] (CORBA::UShort index) const {
			g_assert (m_data);
			return m_data[index];
	  	}

		T_slice& operator[] (CORBA::Short index) {
			g_assert (m_data);
			return m_data[index];
	  	}
		const T_slice& operator[] (CORBA::Short index) const {
			g_assert (m_data);
			return m_data[index];
	  	}


		T_slice& operator[] (CORBA::ULong index) {
			g_assert (m_data);
			return m_data[index];
	  	}
		
		const T_slice& operator[] (CORBA::ULong index) const {
			g_assert (m_data);
			return m_data[index];
	  	}

		T_slice& operator[] (CORBA::Long index) {
			g_assert (m_data);
			return m_data[index];
	  	}
		const T_slice& operator[] (CORBA::Long index) const {
			g_assert (m_data);
			return m_data[index];
	  	}

		operator T_slice*& () {
			return out();
		}
		
		operator const T_slice* () const {
			return m_data;
		}
		
		operator const T_slice* () {
			return m_data;
		}
		
		// parameter passing conversion
		T_slice const *in() const {
			return m_data;
		}
		T_slice *inout() {
			return m_data;
		}
		T_slice *&out() {
			if (m_data) {
				Properties::free(m_data);
				m_data = NULL;
			}
			return m_data;
		}
		T_slice *_retn() {
			T_slice *temp = m_data;
			m_data = NULL;
			return temp;
		}		
	};
	
	template<class T_slice, CORBA::ULong len>
	class ArrayVariable_out {
	private:
		T_slice *&m_data;
		typedef ArrayProperties<T_slice, len> Properties;
	public:
		ArrayVariable_out(T_slice *&data)
			: m_data(data){
		}
		ArrayVariable_out(ArrayVariable_var<T_slice, len> &var)
			: m_data(var.m_data) {
			Properties::free(m_data);
			m_data = NULL;
		}

		ArrayVariable_out(ArrayVariable_out const &src)
			: m_data(src.m_data) {
	  	}

		// assignment
		ArrayVariable_out &operator=(ArrayVariable_out &src) {
			m_data = src.m_data;
			return src;
		}
		ArrayVariable_out &operator=(T_slice *src) {
			m_data = src;
			return *this;
		}
		
		// index
		T_slice &operator[](CORBA::ULong index) {
			g_assert (m_data);
			return m_data[index];
	  	}
		T_slice const &operator[](CORBA::ULong index) const {
			g_assert (m_data);
			return m_data[index];
	  	}

		// access
		operator T_slice *&() {
			return m_data;
		}
		T_slice *&ptr() {
			return m_data;
		}
	private: // forbidden
		void operator=(ArrayVariable_var<T_slice, len> &);

	};

	template<class T_slice, CORBA::ULong len>
	class ArrayFixed_forany {
	private:
		T_slice *m_data;
		CORBA::Boolean m_nocopy;

	public:
		ArrayFixed_forany() {
			m_data = NULL;
			m_nocopy = FALSE;
		}
		ArrayFixed_forany(T_slice *data, CORBA::Boolean nocopy = FALSE) {
			m_data = data;
			m_nocopy = nocopy;
	  	}
		ArrayFixed_forany(ArrayFixed_forany<T_slice, len> const &src, CORBA::Boolean nocopy){
			m_data = src.m_data;
			m_nocopy = nocopy;
		}

		ArrayFixed_forany<T_slice, len> &operator=(T_slice *data) {
			m_data = data;
			return *this;
		}

		ArrayFixed_forany<T_slice, len> &operator=(ArrayFixed_forany<T_slice, len> const &src) {
			m_data = src.m_data;
			return *this;
		}

		T_slice &operator[](CORBA::ULong index) {
			g_assert (m_data);
			return m_data[index];
	  	}
		
		T_slice const &operator[](CORBA::ULong index) const {
			g_assert (m_data);
			return m_data[index];
	  	}

		operator T_slice *() {
			return m_data;
		}
		
/*
  // GCC spews warnings if we include this
		operator T_slice const *() const {
			return m_data;
		}
*/
		// parameter passing conversion
		T_slice const *in() const {
			return m_data;
		}
		T_slice *inout() {
			return m_data;
		}
		T_slice *out() {
			return m_data;
		}
		T_slice *_retn() {
			T_slice *temp = m_data;
			m_data = NULL;
			return temp;
		}
		
		CORBA::Boolean _nocopy() { return m_nocopy; }
	};

	template<class T_slice, CORBA::ULong len>
	class ArrayVariable_forany {
	private:
		T_slice *m_data;
		CORBA::Boolean m_nocopy;
	public:

		ArrayVariable_forany() {
			m_data = NULL;
			m_nocopy = FALSE;
		}
		ArrayVariable_forany(T_slice *data, CORBA::Boolean nocopy = FALSE) {
			m_data = data;
			m_nocopy = nocopy;
	  	}
		ArrayVariable_forany(ArrayVariable_forany<T_slice, len> const &src, CORBA::Boolean nocopy){
			m_data = src.m_data;
			m_nocopy = nocopy;
		}

		ArrayVariable_forany<T_slice, len> &operator=(T_slice *data) {
			m_data = data;
			return *this;
		}

		ArrayVariable_forany<T_slice, len> &
		operator=(ArrayVariable_forany<T_slice, len> const &src) {
			m_data = src.m_data;
			return *this;
		}

		T_slice &operator[](CORBA::ULong index) {
			g_assert (m_data);
			return m_data[index];
	  	}
		
		T_slice const &operator[](CORBA::ULong index) const {
			g_assert (m_data);
			return m_data[index];
	  	}
	  	
		operator T_slice *&() const {
			return const_cast<T_slice *&>(m_data);
		}

		// parameter passing conversion
		T_slice const *in() const {
			return m_data;
		}
		T_slice *inout() {
			return m_data;
		}
		T_slice *&out() {
			return m_data;
		}
		T_slice *_retn() {
			T_slice *temp = m_data;
			m_data = NULL;
			return temp;
		}
	
		CORBA::Boolean _nocopy() { return m_nocopy; }
	};

}


// String smart pointers ------------------------------------------------------
namespace _orbitcpp {
	template<class CharT>
	struct StringProperties { 
	};
	
	template<>
	struct StringProperties< ::CORBA::Char> { 
		typedef CORBA::Char CharT;
		
		static CharT *alloc(::CORBA::ULong len) {
			return CORBA::string_alloc(len);
		}
		static CharT *dup(CharT const *str) {
			return CORBA::string_dup(str);
		}
		static void free(CharT *str) {
			CORBA::string_free(str);
		}

	};
  
	template<>
	struct StringProperties< ::CORBA::WChar> { 
		typedef CORBA::WChar CharT;
		
		static CharT *alloc(::CORBA::ULong len) {
			return CORBA::wstring_alloc(len);
		}
		static CharT *dup(CharT const *str) {
			return CORBA::wstring_dup(str);
		}
		static void free(CharT *str) {
			return CORBA::wstring_free(str);
		}
	};
  
	template<class CharT>
	class String_out;
	
	template<class CharT,bool p_manager = false>
	class String_var {
		friend class String_out<CharT>;
	protected:
		CharT								*m_data;
		typedef StringProperties<CharT>		Properties;
	
	public: 
		String_var()
			: m_data(NULL) {
			if(p_manager) m_data = Properties::dup("");
		}
		String_var(CharT *p)
			: m_data(p) {
		}
		String_var(const CharT *p)
			: m_data(Properties::dup(p)) {
		}
		String_var(const String_var &s)
			: m_data(Properties::dup(s.m_data)) {
		}
		~String_var() {
			Properties::free(m_data);
			m_data = NULL;
			//_orbitcpp::point_to_memhow_none((gpointer*)&m_data); 
		}
  
		String_var &operator=(CharT *p) {
			Properties::free(m_data);
			m_data = p;
	  		return *this;
		}
		String_var &operator=(const CharT *p) {
			Properties::free(m_data);
	  		m_data = Properties::dup(p);
	  		return *this;
	  	}
		String_var &operator=(const String_var &s) {
			Properties::free(m_data);
	  		m_data = Properties::dup(s.m_data);
	  		return *this;
		}
		/*  
		operator CharT*() {
			return m_data;
	  	}
		*/
		operator const CharT*() const {
			return m_data;
	  	}
		
		const CharT* in() const {
			return m_data;
		}
		CharT *&inout() {
			return m_data;
	  	}
		CharT *&out() {
			Properties::free(m_data);
	  		m_data = NULL;
		  	return m_data;
		}
		CharT *_retn() {
			CharT *temp = m_data;
			Properties::free(m_data);
	  		m_data = NULL;
	  		return temp;
		}
  
		CharT &operator[](CORBA::ULong index) {
			return m_data[index];
	  	}
		CharT operator[](CORBA::ULong index) const {
			return m_data[index];
	  	}
	};
  
	template<class CharT>
	class String_out {
	protected:
		CharT								*&m_data;
		typedef StringProperties<CharT>		Properties;
  
	public: 
		String_out(CharT *&p)
			: m_data(p) {
	  		m_data = NULL;
	  	}
		String_out(String_var<CharT> &p)
			: m_data(p.m_data) {
	  		Properties::free(m_data);
	  		m_data = NULL;
	  	}
		String_out(String_out const & s)
			: m_data(s.m_data) {
	  	}
  
		String_out &operator=(String_out const &s) {
			m_data = s.m_data;
	  		return *this;
	  	}
		String_out &operator=(CharT *p) {
			m_data = p;
	  		return *this;
		}
		String_out &operator=(const CharT *p) {
			m_data = Properties::dup(p);
	  		return *this;
		}
  
		operator CharT *&() {
			return m_data;
	  	}
		operator CharT **() { // nonstandard
			return &m_data;
		}
		CharT *&ptr() {
			return m_data;
	  	}
  
	  private: 
		// assignment from String_var disallowed 
		void operator=(const String_var<CharT> &s);
	};
}
  
  
  
  
  
namespace CORBA {
	typedef _orbitcpp::String_var<CORBA::Char>			String_var;
	typedef _orbitcpp::String_var<CORBA::Char,true>		String_mgr;
	typedef _orbitcpp::String_out<CORBA::Char>			String_out;

	struct String_seq_elem_traits
	{
        void pack_elem (String_mgr &cpp_value, CORBA_char *&c_value) const {
			c_value = CORBA::string_dup (cpp_value);
		}

        void unpack_elem (String_mgr &cpp_value, CORBA_char *&c_value) const {
			cpp_value = CORBA::string_dup (c_value);
        }
	};

	typedef _orbitcpp::String_var<CORBA::WChar>			WString_var;
	typedef _orbitcpp::String_var<CORBA::WChar,true>	WString_mgr;
	typedef _orbitcpp::String_out<CORBA::WChar>			WString_out;
}




namespace CORBA { 
	class Environment;
	class Policy;
	class DomainManager;
	class NamedValue;
	class NVList;
	class ExceptionList;
	class ContextList;
	class Request;
	class Context;
	class TypeCode;
	class ORB;
	class Object;
	
	ORBITCPP_MAKEREFTYPES(Environment)
	ORBITCPP_MAKEREFTYPES(Policy)
	ORBITCPP_MAKEREFTYPES(DomainManager)
	ORBITCPP_MAKEREFTYPES(NamedValue)
	ORBITCPP_MAKEREFTYPES(NVList)
	ORBITCPP_MAKEREFTYPES(ExceptionList)
	ORBITCPP_MAKEREFTYPES(ContextList)
	ORBITCPP_MAKEREFTYPES(Request)
	ORBITCPP_MAKEREFTYPES(Context)
	ORBITCPP_MAKEREFTYPES(TypeCode)
	ORBITCPP_MAKEREFTYPES(ORB)
	ORBITCPP_MAKEREFTYPES(Object)
}




namespace PortableServer {
	class POAManager;
	class AdapterActivator;
	class ServantManager;
	class ServantActivator;
	class ServantLocator;
	class POA;
	class Current;
	
	ORBITCPP_MAKEREFTYPES(POAManager)
	ORBITCPP_MAKEREFTYPES(AdapterActivator)
	ORBITCPP_MAKEREFTYPES(ServantManager)
	ORBITCPP_MAKEREFTYPES(ServantActivator)
	ORBITCPP_MAKEREFTYPES(ServantLocator)
	ORBITCPP_MAKEREFTYPES(POA)
	ORBITCPP_MAKEREFTYPES(Current)
}




#endif
