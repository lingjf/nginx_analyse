
default:	build

clean:
	rm -rf Makefile /home/lingjf/running

build:
	$(MAKE) -f /home/lingjf/running/Makefile
	$(MAKE) -f /home/lingjf/running/Makefile manpage

ut:
	$(MAKE) -f /home/lingjf/running/Unitest.mk
	
install:
	$(MAKE) -f /home/lingjf/running/Makefile install

upgrade:
	/home/lingjf/running/sbin/nginx -t

	kill -USR2 `cat /home/lingjf/running/logs/nginx.pid`
	sleep 1
	test -f /home/lingjf/running/logs/nginx.pid.oldbin

	kill -QUIT `cat /home/lingjf/running/logs/nginx.pid.oldbin`
