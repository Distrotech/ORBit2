#include "test-impl.hh"
#include <iostream>

void IMaster_impl::struct_in (const ::Test::Test_st &struct_in)
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_in: one = " << struct_in.one << std::endl;
    std::cerr << "IMaster_impl::struct_in: two = " << struct_in.two << std::endl;
    std::cerr << "IMaster_impl::struct_in: dir = " << struct_in.dir << std::endl;
}

void IMaster_impl::struct_inout (::Test::Test_st &struct_inout)
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_inout: one = " << struct_inout.one << std::endl;
    std::cerr << "IMaster_impl::struct_inout: two = " << struct_inout.two << std::endl;
    std::cerr << "IMaster_impl::struct_inout: dir = " << struct_inout.dir << std::endl;

    struct_inout.one *= 10;
    struct_inout.two *= 20;
    struct_inout.dir = Test::DIR_NORTH;
}

void IMaster_impl::struct_out (::Test::Test_st_out struct_out)
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_out" << std::endl;

    struct_out.one = 40;
    struct_out.two = 30;
    struct_out.dir = Test::DIR_SOUTH;
}

::Test::Test_st IMaster_impl::struct_ret ()
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_ret" << std::endl;

    ::Test::Test_st struct_ret;
    
    struct_ret.one = 50;
    struct_ret.two = 60;
    struct_ret.dir = Test::DIR_NORTH;

    return struct_ret;
}
