/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t;c-basic-offset: 4 -*- */
#ifndef TEST_IMPL_HH
#define TEST_IMPL_HH

#include "test-cpp-skels.hh"
#include <string>

class IMember_impl: public POA_Test::IMember
{
	std::string name;

public:
	IMember_impl (const std::string &name);
	
	void print_name ()
		throw (CORBA::SystemException);	
};

class IMaster_impl: public POA_Test::IMaster
{
	IMember_impl obj_member_1, obj_member_2;
	
public:
	IMaster_impl ();

	// Simple interfaces
	void provoke ()
		throw (CORBA::SystemException, ::Test::ExTest);
};

#endif // TEST_IMPL_HH
