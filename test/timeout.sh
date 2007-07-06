#!/bin/sh

# This will test GIOP timeouts

echo "Running timeout server..."
./timeout-server &

sleep 1
 
echo "Running timeout client..."
./timeout-client 
retv=$?


killall lt-timeout-server

exit $retv
