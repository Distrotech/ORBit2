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
	IMember_impl obj_one, obj_two, obj_one_mod;
	IMember_impl out_obj_one, out_obj_two;
	
public:
	IMaster_impl ();
	
	void struct_in (const ::Test::Test_st &struct_in)
		throw (CORBA::SystemException);

	void struct_inout (::Test::Test_st &struct_inout)
		throw (CORBA::SystemException);

	void struct_out (::Test::Test_st_out struct_out)
		throw (CORBA::SystemException);

	::Test::Test_st * struct_ret ()
		throw (CORBA::SystemException);		
};

#endif // TEST_IMPL_HH
