SUBDIRS=include src

m4datadir=$(datadir)/aclocal
m4data_DATA=linc.m4

EXTRA_DIST=	linc.m4 linc-config.in linc.pc.in

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=linc.pc

$(pkgconfig_DATA): config.status

bin_SCRIPTS = linc-config

