#include "test-impl.hh"
#include <iostream>

IMember_impl::IMember_impl (const std::string &name_):
    name (name_)
{
}

void IMember_impl::print_name ()
    throw (CORBA::SystemException)
{
    std::cout << name << "::print_name" << std::endl;
}



IMaster_impl::IMaster_impl ():
    array_member_1 ("IMember used for exceptions #1"),
    array_member_2 ("IMember used for exceptions #2"),
    array_member_3 ("IMember used for exceptions #3")
{
}


void IMaster_impl::provoke ()
    throw (CORBA::SystemException, Test::ExTest)
{
    static Test::IMember_ptr array_member_1_ptr = array_member_1._this ();
    static Test::IMember_ptr array_member_2_ptr = array_member_2._this ();
    static Test::IMember_ptr array_member_3_ptr = array_member_3._this ();

    Test::NumArray ex_num = { 11, 22, 33 };

    Test::ObjArray ex_obj;
    ex_obj[0] = Test::IMember::_duplicate (array_member_1_ptr);
    ex_obj[1] = Test::IMember::_duplicate (array_member_2_ptr);
    ex_obj[2] = Test::IMember::_duplicate (array_member_3_ptr);

    throw Test::ExTest (ex_obj, ex_num);
}
