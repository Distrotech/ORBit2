AUTOMAKE_OPTIONS = 1.3

SUBDIRS=libIDL src include test docs

bin_SCRIPTS=orbit-config

EXTRA_DIST=ORBit.spec.in ORBit2.m4 ORBit-2.0.pc ORBit-CosNaming-2.0.pc

dist-hook: ORBit.spec
	cp ORBit.spec $(distdir)

rpms: distcheck
	rpm -ta $(top_builddir)/@PACKAGE@-@VERSION@.tar.gz

m4datadir=$(datadir)/aclocal
m4data_DATA=ORBit2.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ORBit-2.0.pc ORBit-CosNaming-2.0.pc
