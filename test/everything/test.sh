#!/bin/sh

# This is a generic script for firing up a server, waiting for it to write
# its stringified IOR to a file, then firing up a server

# We had issues creating this once, and it gives us a 'clean' slate.
rm -Rf "/tmp/orbit-$USER"

for params in '--ORBIIOPIPv4=0 --ORBIIOPUSock=1'		\
	      '--ORBIIOPIPv4=1 --ORBIIOPUSock=0'		\
	      '--ORBIIOPIPv4=0 --ORBIIOPUSock=1 --gen-imodule'	\
	      '--ORBIIOPIPv4=1 --ORBIIOPUSock=0 --gen-imodule'
do

    ./server $params &

    until test -s iorfile; do sleep 1; done

    if ./client $params; then
	echo "============================================================="
	echo "Test passed with params: $params"
	echo "============================================================="
	rm iorfile
    else
        echo "============================================================="
	echo "Test failed with params: $params"
        echo "============================================================="
	kill $!
	rm iorfile
	exit 1
    fi

done
