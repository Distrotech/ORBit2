IDL = $(top_builddir)/src/idl-compiler/orbit-idl
INCLUDES=-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	$(LINC_CFLAGS) \
	$(GLIB_CFLAGS)

POA_TESTS_SRCS = poatest-basic01.c \
                 poatest-basic02.c \
                 poatest-basic03.c \
                 poatest-basic04.c \
                 poatest-basic05.c \
                 poatest-basic06.c \
                 poatest-basic07.c \
                 poatest-basic08.c \
                 poatest-basic09.c \
                 poatest-basic10.c

TESTS = poatest-basic01 \
        poatest-basic02 \
        poatest-basic03 \
        poatest-basic04 \
        poatest-basic06 \
        poatest-basic07 \
        poatest-basic08 \
        poatest-basic09 \
        poatest-basic10

#        poatest-basic05 \

check_PROGRAMS = $(TESTS)

LDADD = $(top_builddir)/src/orb/libORBit-2.la

POATEST_IDLOUT = poatest.h poatest-common.c poatest-skels.c poatest-stubs.c
$(POATEST_IDLOUT) : poatest.idl
	$(IDL) --showcpperrors poatest.idl

common_srcs = $(POATEST_IDLOUT) poatest-basic-shell.c poatest-exception.h

poatest_basic01_SOURCES = $(common_srcs) poatest-basic01.c 
poatest_basic02_SOURCES = $(common_srcs) poatest-basic02.c 
poatest_basic03_SOURCES = $(common_srcs) poatest-basic03.c 
poatest_basic04_SOURCES = $(common_srcs) poatest-basic04.c 
#poatest_basic05_SOURCES = $(common_srcs) poatest-basic05.c 
poatest_basic06_SOURCES = $(common_srcs) poatest-basic06.c 
poatest_basic07_SOURCES = $(common_srcs) poatest-basic07.c 
poatest_basic08_SOURCES = $(common_srcs) poatest-basic08.c 
poatest_basic09_SOURCES = $(common_srcs) poatest-basic09.c 
poatest_basic10_SOURCES = $(common_srcs) poatest-basic10.c 

BUILT_SOURCES = $(POATEST_IDLOUT)
CLEANFILES = dynany_built echo_built empty_built test1_built test_any_built $(BUILT_SOURCES)
EXTRA_DIST = poatest.idl README

README : $(POA_TESTS_SRCS)
	awk -F'*' 'BEGIN { intest = 0; } \
                   /^ \* Test/ { intest = 1; } \
                   intest == 1 && /^ \*\// { intest=0; print "\n"; } \
                   intest == 1 && /^ \*/ { print $$2; } ' \
                         $(POA_TESTS_SRCS) > README
