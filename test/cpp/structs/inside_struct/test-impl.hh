/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t;c-basic-offset: 4 -*- */
#ifndef TEST_IMPL_HH
#define TEST_IMPL_HH

#include "test-cpp-skels.hh"
#include <string>

class IMaster_impl: public POA_Test::IMaster
{
public:
	void struct_in (const ::Test::Outer_st &struct_in)
		throw (CORBA::SystemException);

	::Test::Outer_st * struct_ret ()
		throw (CORBA::SystemException);		
};

#endif // TEST_IMPL_HH
