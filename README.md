mtudetect
---------

Detects the MTU in a network by sending ping-packets of varying size to a host.

Build
-----

Goto folder src and call make.

Installation
------------

This is an OpenWRT-package for GLUON and Freifunk.


Execution
---------

As root execute the following command.

	# mtudetect -?

	Usage: mtudetect <-d> <-y> <-m send_mtu> <-s set_mtu> <-i interval> <-r receive-timeout> <-t target-ip> <-f fastd-ip>

	        -d              debug, don't detach process
	        -y              simulate changes
	        -m <mtu>        the MTU to test
	        -s <mtu>        the MTU to set
	        -i <interval>   the test-interval
	        -r <timeout>    the receive-timeout for ping-response
	        -t <target-ip>  the machine to ping
	        -f <fastd-ip>   the fastd-ip to use



License
-------

This application is published under GPLv2 license.
