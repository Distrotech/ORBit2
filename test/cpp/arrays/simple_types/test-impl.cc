#include "test-impl.hh"
#include <iostream>

void IFace_impl::array_method (::Test::LongArray_out   long_results,
			       ::Test::StringArray_out string_results)
    throw (CORBA::SystemException)
{
    gchar *tmp;

    string_results = ::Test::StringArray_alloc ();
    
    for (CORBA::ULong i = 0; i < 10; i++)
    {
	long_results[i] = i * 10;

	tmp = g_strdup_printf ("String #%d", i);
	
	string_results[i] = CORBA::string_dup (tmp);
	g_free (tmp);
    }
}

void IFace_impl::strings_method (const ::Test::StringArray array_in,
				 ::Test::StringArray       array_inout)
    throw (CORBA::SystemException)
{
    std::cout << "IFace_impl::strings_method:" << std::endl;

    for (CORBA::ULong i = 0; i < 10; i++)
    {
	std::cout << "\tIN:\t" << array_in[i] << std::endl;
    }

    gchar *tmp;
    for (CORBA::ULong i = 0; i < 10; i++)
    {
	std::cout << "\tINOUT:\t" << array_inout[i] << std::endl;

	tmp = g_strdup_printf ("Modified by skel #%d", i + 1);
	array_inout[i] = CORBA::string_dup (tmp);
	g_free (tmp);
    }
}

Test::StringArray_slice * IFace_impl::strings_ret ()
    throw (CORBA::SystemException)
{
    std::cout << "IFace_impl::strings_get" << std::endl;
    
    Test::StringArray_slice * retval = Test::StringArray_alloc ();

    gchar *tmp;
    for (CORBA::ULong i = 0; i < 10; i++)
    {
	tmp = g_strdup_printf ("RET #%d", i + 1);
	retval[i] = CORBA::string_dup (tmp);
	g_free (tmp);
    }

    return retval;
}
