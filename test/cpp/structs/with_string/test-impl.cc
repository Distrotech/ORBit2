#include "test-impl.hh"
#include <iostream>

void IMaster_impl::struct_in (const ::Test::Test_st &struct_in)
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_in: num = " << struct_in.num << std::endl;
    std::cerr << "IMaster_impl::struct_in: string_one = " << struct_in.string_one << std::endl;
    std::cerr << "IMaster_impl::struct_in: string_two = " << struct_in.string_two << std::endl;
}

void IMaster_impl::struct_inout (::Test::Test_st &struct_inout)
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_in: num = " << struct_inout.num << std::endl;
    std::cerr << "IMaster_impl::struct_in: string_one = " << struct_inout.string_one << std::endl;
    std::cerr << "IMaster_impl::struct_in: string_two = " << struct_inout.string_two << std::endl;

    struct_inout.num = 30;
    struct_inout.string_one = CORBA::string_dup ("Modified #1");
    struct_inout.string_two = CORBA::string_dup ("Modified #2");
}

void IMaster_impl::struct_out (::Test::Test_st_out struct_out)
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_out" << std::endl;

    struct_out = new ::Test::Test_st;
    
    struct_out->num = 30;
    struct_out->string_one = CORBA::string_dup ("Ret #1");
    struct_out->string_two = CORBA::string_dup ("Ret #2");
}

::Test::Test_st * IMaster_impl::struct_ret ()
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_ret" << std::endl;
    
    ::Test::Test_st *retval = new ::Test::Test_st;
    
    retval->num = 50;
    retval->string_one = CORBA::string_dup ("Ret #1");
    retval->string_two = CORBA::string_dup ("Ret #2");

    return retval;
}
