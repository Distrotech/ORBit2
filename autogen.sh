#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir

DIE=0

# Check for autoconf
(autoconf --version | grep -q "version 2.12") || 
(autoconf --version | grep -q "version 2.13") || {
	echo
	echo "You must have at minimum autoconf version 2.12 installed"
	echo "to compile ORBit. Download the appropriate package for"
	echo "your distribution, or get the source tarball at"
	echo "ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
}

# Check for libtool
(libtool --version | grep -q "1.2") || {
	echo
	echo "You must have at minimum libtool version 1.2 installed"
	echo "to compile ORBit. Download the appropriate package for"
	echo "your distribution, or get the source tarball at"
	echo "ftp://alpha.gnu.org/gnu/libtool-1.2d.tar.gz"
	DIE=1
}

# Check for automake
(automake --version | grep -q "1.3") ||
(automake --version | grep -q "1.4") ||{
	echo
	echo "You must have at minimum automake version 1.3 installed"
	echo "to compile ORBit. Download the appropriate package for"
	echo "your distribution, or get the source tarball at"
	echo "ftp://ftp.cygnus.com/pub/home/tromey/automake-1.3b.tar.gz"
	DIE=1
}


if test "$DIE" -eq 1; then
	exit 1
fi

(test -f src/orb/orbit_types.h) || {
	echo "You must run this script in the top-level ORBit directory"
	exit 1
}

if test -z "$*"; then
	echo "I am going to run ./configure with no arguments - if you wish "
        echo "to pass any to it, please specify them on the $0 command line."
fi

for i in .
do 
  echo processing $i
  (cd $i; \
    libtoolize --copy --force; \
    aclocal $ACLOCAL_FLAGS; autoheader; \
    automake --add-missing; \
    autoheader; \
    autoconf)
done

echo processing libIDL
(cd libIDL; \
    libtoolize --copy --force; \
    automake --add-missing; \
    aclocal $ACLOCAL_FLAGS; \
    autoconf)

cd $ORIGDIR

echo "Running $srcdir/configure --enable-maintainer-mode" "$@"
$srcdir/configure --enable-maintainer-mode "$@"

echo 
echo "Now type 'make' to compile ORBit."
