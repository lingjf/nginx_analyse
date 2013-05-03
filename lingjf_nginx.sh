#!/bin/sh

#sudo apt-get install libpcre3 libpcre3-dev

BASED=`pwd`

./configure \
	--with-file-aio \
	--prefix=${BASED}/objs  	

	
