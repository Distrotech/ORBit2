/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t;c-basic-offset: 4 -*- */
#ifndef _ORBIT_CPP_IDL_helloworld_IMPL_HH
#define _ORBIT_CPP_IDL_helloworld_IMPL_HH

#include "struct_simple-cpp-skels.hh"
#include <string>

namespace Test
{
	class TestIface_impl: public virtual POA_Test::TestIface
	{
	public:
		virtual SimpleStructArg* test_ret ()
			throw (CORBA::SystemException);
			
		virtual void test_in (const SimpleStructArg &in_struct,
							  const char            *message)
			throw (CORBA::SystemException);
	};
	
	class OutputStream_impl: public virtual POA_Test::OutputStream
	{
		std::string member_data;

	public:
		OutputStream_impl (const std::string &member_data);
		
		virtual void print (const char *message)
			throw (CORBA::SystemException);
	};
	
}; // namespace Test


#endif //_ORBIT_CPP_IDL_helloworld_IMPL_HH
