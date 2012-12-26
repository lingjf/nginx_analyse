
default:	ut

clean:
	rm -rf Makefile objs

build:
	$(MAKE) -f objs/Makefile
	$(MAKE) -f objs/Makefile manpage

ut:
	$(MAKE) -f objs/Unitest.mk
	
install:
	$(MAKE) -f objs/Makefile install

upgrade:
	/media/sf_Proj/nginx_analyse/objs/sbin/nginx -t

	kill -USR2 `cat /media/sf_Proj/nginx_analyse/objs/logs/nginx.pid`
	sleep 1
	test -f /media/sf_Proj/nginx_analyse/objs/logs/nginx.pid.oldbin

	kill -QUIT `cat /media/sf_Proj/nginx_analyse/objs/logs/nginx.pid.oldbin`
