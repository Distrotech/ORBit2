/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t;c-basic-offset: 4 -*- */
#ifndef TEST_IMPL_HH
#define TEST_IMPL_HH

#include "test-cpp-skels.hh"
#include <string>

class IFace_impl: public POA_Test::IFace
{
public:
	void array_method (::Test::LongArray_out   long_results,
					   ::Test::StringArray_out string_results)
		throw (CORBA::SystemException);	


	void strings_method (const ::Test::StringArray array_in,
						 ::Test::StringArray       array_inout)
		throw (CORBA::SystemException);	

	Test::StringArray_slice * strings_ret ()
		throw (CORBA::SystemException);	
};

#endif // TEST_IMPL_HH
