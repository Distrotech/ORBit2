#ifndef TEST_IMPL_HH
#define TEST_IMPL_HH

#include "test-cpp-skels.hh"
#include <string>

class IMember_impl: public POA_Test::IMember
{
	std::string name;

public:
	IMember_impl (const std::string &name);

	void print_name()
		throw (CORBA::SystemException);
};


class IMaster_impl: public POA_Test::IMaster
{
	IMember_impl array_member_1;
	IMember_impl array_member_2;
	IMember_impl array_member_3;

	::Test::IMember_ptr array_member_1_ptr;
	::Test::IMember_ptr array_member_2_ptr;
	::Test::IMember_ptr array_member_3_ptr; 

public:
	IMaster_impl();

	::Test::IMember_ptr simple_ret()
		throw (CORBA::SystemException);

	::Test::ObjArray_slice *array_ret()
		throw (CORBA::SystemException);

	void simple_in (::Test::IMember_ptr instance)
		throw (CORBA::SystemException);

	void array_in (const ::Test::ObjArray instances)
		throw (CORBA::SystemException);
};

#endif
