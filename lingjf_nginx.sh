#!/bin/sh

#sudo apt-get install libpcre3 libpcre3-dev

BASED=`pwd`

./configure \
	--prefix=${BASED}/objs \
	--with-file-aio \
	--add-module=${BASED}/modules/http_concat_module \
	--add-module=${BASED}/modules/http_copyleft_module
	  	

	
