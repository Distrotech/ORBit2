#include "test-impl.hh"
#include <iostream>

void IMaster_impl::array_in (const ::Test::TestArray array_in)
    throw (CORBA::SystemException)
{   
    std::cerr << "IMaster_impl::array_in" << std::endl;

    std::cerr << "\t#1" << std::endl;
    std::cerr << "\t\tNum:\t\t" << array_in[0].num << std::endl;
    std::cerr << "\t\tString #1:\t" << array_in[0].string_one << std::endl;
    std::cerr << "\t\tString #2:\t" << array_in[0].string_two << std::endl;

    std::cerr << "\t#2" << std::endl;
    std::cerr << "\t\tNum:\t\t" << array_in[1].num << std::endl;
    std::cerr << "\t\tString #1:\t" << array_in[1].string_one << std::endl;
    std::cerr << "\t\tString #2:\t" << array_in[1].string_two << std::endl;
}

::Test::TestArray_slice * IMaster_impl::array_ret ()
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_ret" << std::endl;

    ::Test::TestArray_slice *retval = ::Test::TestArray_alloc ();

#if 0
    inner->num = 33;
    inner->string_one = CORBA::string_dup ("Ret inner #1");
    inner->string_two = CORBA::string_dup ("Ret inner #2");
    
    ::Test::Outer_st *retval = new ::Test::Outer_st;
    retval->num = 50;
    retval->id = CORBA::string_dup ("Ret outer");
    retval->struct_member = inner;
#endif
    
    return retval;
}
