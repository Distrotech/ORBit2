#include "test-impl.hh"
#include <iostream>

IMember_impl::IMember_impl (const std::string &name_)
	: name(name_)
{

}


void IMember_impl::print_name()
		throw (CORBA::SystemException)
{
	std::cout << name << "::print_name" << std::endl;
}


IMaster_impl::IMaster_impl()
	: array_member_1 ("IMember used for arrays of interfaces #1"),
    array_member_2 ("IMember used for arrays of interfaces #2"),
    array_member_3 ("IMember used for arrays of interfaces #3")
{
	array_member_1_ptr = array_member_1._this();
	array_member_2_ptr = array_member_2._this();
	array_member_3_ptr = array_member_3._this();
}


::Test::IMember_ptr IMaster_impl::simple_ret()
	throw (CORBA::SystemException)
{
	return ::Test::IMember::_duplicate(array_member_1_ptr);
}


::Test::ObjArray_slice *IMaster_impl::array_ret()
	throw (CORBA::SystemException)
{
	::Test::ObjArray_slice *retval = ::Test::ObjArray_alloc();

	retval[0] = ::Test::IMember::_duplicate(array_member_1_ptr);
	retval[1] = ::Test::IMember::_duplicate(array_member_2_ptr);
	retval[2] = ::Test::IMember::_duplicate(array_member_3_ptr);

	return retval;
}


void IMaster_impl::simple_in (::Test::IMember_ptr instance)
	throw (CORBA::SystemException)
{
	cout << "simple_in was passed:" << endl;
	instance->print_name();
}


void IMaster_impl::array_in (const ::Test::ObjArray instances)
	throw (CORBA::SystemException)
{
	int i = 0;

	cout << "array_in was passed:" << endl;
	for (i = 0; i < 3; i++) {
		instances[i]->print_name();
	}
}
