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
    obj_member_1 ("IMember used for exceptions #1"),
    obj_member_2 ("IMember used for exceptions #2")
{
}


void IMaster_impl::provoke ()
    throw (CORBA::SystemException, Test::ExTest)
{
    static Test::IMember_ptr obj_member_1_ptr = obj_member_1._this ();
    static Test::IMember_ptr obj_member_2_ptr = obj_member_2._this ();

    Test::var_st var_st;
    var_st.member_1 = Test::IMember::_duplicate (obj_member_1_ptr);
    var_st.member_2 = Test::IMember::_duplicate (obj_member_2_ptr);

    Test::fix_st fix_st;
    fix_st.num_1 = 10;
    fix_st.num_2 = 11;
    
    throw Test::ExTest (var_st, fix_st);
}
