LIBWRAP_PATH=@LIBWRAP_PATH@
LIB_WRAP=@LIB_WRAP@
if HAVE_HOSTS_ACCESS
libwrap_files=$(addsuffix .lo, $(basename $(filter %.o, $(shell ar t $(LIBWRAP_PATH)))))
libwrap_deps=
else
libwrap_files=
libwrap_deps=$(LIB_WRAP)
endif

libIIOPincludedir = $(includedir)/ORBit/IIOP

INCLUDES=-I.. -I$(top_srcdir)/src @GLIB_CFLAGS@ -I$(top_builddir)/src/ORBitutil

lib_LTLIBRARIES=libIIOP.la

libIIOPincludedir = $(includedir)/IIOP

libIIOP_la_SOURCES= \
	giop-connection.c \
	iiop-connection.c \
	giop-msg-buffer.c \
	giop-recv-buffer.c \
	giop-send-buffer.c \
	IIOP-private.h

libIIOPinclude_HEADERS = \
	giop-connection.h \
	iiop-connection.h \
	giop-msg-buffer.h \
	giop-recv-buffer.h \
	giop-send-buffer.h \
	giop-types.h \
	giop-msg.h \
	IIOP.h 

libIIOP_la_LIBADD=$(libwrap_files) $(libwrap_deps)

$(libwrap_files): make_libwrap_files

make_libwrap_files: $(LIBWRAP_PATH)
	ar x $(LIBWRAP_PATH)
	for I in $(basename $(libwrap_files)); do mv $$I.o $$I.lo; done
	ar x $(LIBWRAP_PATH)
	touch make_libwrap_files

BUILT_SOURCES=$(libwrap_files)

libIIOP_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

CLEANFILES=make_libwrap_files
