#
# ORBit-C++: C++ bindings for ORBit.
#
# Copyright (C) 2000 Andreas Kloeckner
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# Author:	Andreas Kloeckner <ak@ixion.net>

INCLUDES =                                    \
	-I..			              \
	-I$(top_builddir)                     \
	-I$(top_builddir)/include             \
	 -I$(top_srcdir)                      \
	-I$(top_srcdir)/include               \
	-DVERSION=\"$(ORBIT_VERSION)\"        \
	-DORBIT2_INTERNAL_API                 \
	$(WARN_CFLAGS)                        \
	$(ORBIT_IDL_CFLAGS)

if ENABLE_CPP

noinst_LTLIBRARIES = libidl-cpp.la

headers = \
	base.hh \
	language.hh \
	types.hh \
	pass.hh \
	error.hh \
	pass.hh \
	pass_idl_it.hh \
	pass_gather.hh \
	pass_xlate.hh \
	pass_stubs.hh \
	pass_skels.hh

modules = \
	base.cc \
	language.cc \
	types.cc \
	pass.cc \
	pass_idl_it.cc \
	pass_gather.cc \
	pass_xlate.cc \
	pass_stubs.cc \
	pass_skels.cc \
	main.cc 

libidl_cpp_la_SOURCES = \
	$(headers) $(modules)

libidl_cpp_la_LIBADD = -lstdc++

endif
