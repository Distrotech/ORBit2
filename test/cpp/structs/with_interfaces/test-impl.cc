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
    obj_one ("Obj #1 created by IMaster"),
    obj_two ("Obj #2 created by IMaster"),
    obj_one_mod ("Obj #1 modified by IMaster"),
    out_obj_one ("OutObj #1"),
    out_obj_two ("OutObj #2")
{
}

void IMaster_impl::struct_in (const ::Test::Test_st &struct_in)
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_in: num = " << struct_in.num << std::endl;
    struct_in.obj_one->print_name ();
    struct_in.obj_two->print_name ();
}

void IMaster_impl::struct_inout (::Test::Test_st &struct_inout)
    throw (CORBA::SystemException)
{
    static ::Test::IMember_ptr obj_one_mod_ptr = obj_one_mod._this ();

    std::cerr << "IMaster_impl::struct_inout: num = " << struct_inout.num << std::endl;
    struct_inout.obj_one->print_name ();
    struct_inout.obj_two->print_name ();

    struct_inout.obj_one = ::Test::IMember::_duplicate (obj_one_mod_ptr);
}

void IMaster_impl::struct_out (::Test::Test_st_out struct_out)
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_out" << std::endl;

    static ::Test::IMember_ptr out_obj_one_ptr = out_obj_one._this ();
    static ::Test::IMember_ptr out_obj_two_ptr = out_obj_two._this ();
    
    struct_out = new ::Test::Test_st;
    
    struct_out->num = 100;
    struct_out->obj_one = ::Test::IMember::_duplicate (out_obj_one_ptr);
    struct_out->obj_two = ::Test::IMember::_duplicate (out_obj_two_ptr);
}

::Test::Test_st * IMaster_impl::struct_ret ()
    throw (CORBA::SystemException)
{
    std::cerr << "IMaster_impl::struct_ret" << std::endl;
    
    static ::Test::IMember_ptr obj_one_ptr = obj_one._this ();
    static ::Test::IMember_ptr obj_two_ptr = obj_two._this ();

    ::Test::Test_st *retval = new ::Test::Test_st;
    
    retval->num = 50;
    retval->obj_one = ::Test::IMember::_duplicate (obj_one_ptr);
    retval->obj_two = ::Test::IMember::_duplicate (obj_two_ptr);

    return retval;
}
