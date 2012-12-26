#!/bin/sh


./configure \
	--without-http_rewrite_module \
	--without-http_gzip_module \
	--prefix=`pwd`/objs  	

	