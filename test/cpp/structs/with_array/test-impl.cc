#include "test-impl.hh"
#include <iostream>

void IMaster_impl::struct_in (const ::Test::Test_st &struct_in)
    throw (CORBA::SystemException)
{   
    std::cerr << "IMaster_impl::struct_in" << std::endl;

    std::cerr << "\tnum:\t" << struct_in.num << std::endl;
    std::cerr << "\t#1:\t" << struct_in.text[0] << std::endl;
    std::cerr << "\t#2:\t" << struct_in.text[1] << std::endl;
}

::Test::Test_st * IMaster_impl::struct_ret ()
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_ret" << std::endl;

    ::Test::Test_st *retval = new ::Test::Test_st;

    retval->num = 42;
    retval->text[0] = CORBA::string_dup ("Text #1");
    retval->text[1] = CORBA::string_dup ("Text #2");
    
    return retval;
}
