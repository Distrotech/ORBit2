/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *	ORBit-C++: C++ bindings for ORBit.
 *
 *	Copyright (C) 1998-2000 Phil Dawes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Library General Public
 *	License as published by the Free Software Foundation; either
 *	version 2 of the License, or (at your option) any later version.
 *
 *	This library is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	Library General Public License for more details.
 *
 *	You should have received a copy of the GNU Library General Public
 *	License along with this library; if not, write to the Free
 *	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *	Author: Phil Dawes <philipd@users.sourceforge.net>
 *			Andreas Kloeckner <ak@ixion.net>
 *
 */




#include <orb/orbitcpp_exception.hh>




// CEnvironment ---------------------------------------------------------------
#define ORBITCPP_SYSEX_THROW_O_MATIC(name,quoted_repoid) \
	if (strcmp(repo_id,quoted_repoid)==0) \
		throw CORBA::name(ex_minor,completion);



_orbitcpp::CEnvironment::CEnvironment() {
	// *** FIXME this is ORBit-specific
	CORBA_exception_init(&m_env);
}


_orbitcpp::CEnvironment::~CEnvironment() {
	CORBA_exception_free(&m_env);
}



void 
_orbitcpp::CEnvironment::clear() {
	CORBA_exception_set(&m_env,CORBA_NO_EXCEPTION,NULL,NULL);
}
  
  
  
  
void 
_orbitcpp::CEnvironment::propagate_sysex_guts() {
	CORBA_SystemException *info = (CORBA_SystemException *)
		CORBA_exception_value(&m_env);
  
	CORBA::CompletionStatus completion = (CORBA::CompletionStatus) info->completed;
	CORBA::ULong ex_minor = info->minor;
	const char *repo_id = CORBA_exception_id(&m_env);	

    #define P(name,quotedrepoid) ORBITCPP_SYSEX_THROW_O_MATIC(name,quotedrepoid)
	#include <orb/orbitcpp_exceptionlist.hh>
    #undef P
	
	CORBA_exception_free(&m_env);
	throw ::CORBA::UNKNOWN(ex_minor,completion);
}
