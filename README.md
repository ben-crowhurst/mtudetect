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

	# ./mtudetect 8.8.8.8 1426
	# echo "Result=$?"

Prints on stdout: 

	type:0, code:0, packetlength:1426
	Result: Ping MTU ok(0)
	Result=0


When subcode 13 is detected the result-value is:

	type:3, code:13, packetlength:...
	Result: Ping forbidden(2)
	Result=2	


License
-------

This application is published under GPLv2 license.
