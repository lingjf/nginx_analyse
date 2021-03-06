#!/bin/sh

#sudo apt-get install libpcre3 libpcre3-dev

BASED=`pwd`

./configure \
	--prefix=${BASED}/objs \
	--with-file-aio \
	--with-http_degradation_module \
	--with-http_stub_status_module \
	--add-module=${BASED}/modules/http_concat_module \
	--add-module=${BASED}/modules/http_copyleft_module
	  	

	
