#include "test-impl.hh"
#include <iostream>

void IMaster_impl::struct_in (const ::Test::Outer_st &outer_in)
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_in: num = "
	      << outer_in.num << std::endl;
    std::cerr << "IMaster_impl::struct_in: id = "
	      << outer_in.id << std::endl;

    std::cerr << "IMaster_impl::struct_in: struct_member.num = "
	      << outer_in.struct_member.num << std::endl;
    std::cerr << "IMaster_impl::struct_in: struct_member.string_one = "
	      << outer_in.struct_member.string_one << std::endl;
    std::cerr << "IMaster_impl::struct_in: struct_member.string_two = "
	      << outer_in.struct_member.string_two << std::endl;
}

::Test::Outer_st * IMaster_impl::struct_ret ()
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_ret" << std::endl;

#if 0
    ::Test::Inner_st_var inner = new ::Test::Inner_st;

    inner->num = 33;
    inner->string_one = CORBA::string_dup ("Ret inner #1");
    inner->string_two = CORBA::string_dup ("Ret inner #2");
#endif
    
    ::Test::Outer_st *retval = new ::Test::Outer_st;
    retval->num = 50;
    retval->id = CORBA::string_dup ("Ret outer");
    retval->struct_member.num = 33;
    retval->struct_member.string_one = CORBA::string_dup ("Ret inner #1");
    retval->struct_member.string_two = CORBA::string_dup ("Ret inner #2");
    
    return retval;
}
