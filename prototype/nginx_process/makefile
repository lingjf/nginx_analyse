

CC =	gcc
CFLAGS =  -O0 -g 
CPP =	gcc -E
LINK =	$(CC)

CORE_DEPS = src/core/nginx.h \
	src/core/____ngx_config.h \
	src/core/____ngx_core.h \
	src/core/____ngx_palloc.h \
	src/core/____ngx_array.h \
	src/core/____ngx_string.h \
	src/core/____ngx_connection.h \
	src/core/ngx_cycle.h \
	src/os/unix/____ngx_alloc.h \
	src/os/unix/ngx_process.h \
	src/os/unix/ngx_setaffinity.h \
	src/os/unix/____ngx_setproctitle.h \
	src/os/unix/____ngx_atomic.h \
	src/os/unix/____ngx_linux.h \
	src/os/unix/ngx_process_cycle.h \
	src/os/unix/____ngx_linux_config.h \
	src/____ngx_auto_config.h

CORE_INCS = -I src -I src/core -I src/os/unix 

$(shell mkdir -p objs/src/core)
$(shell mkdir -p objs/src/os/unix)
$(shell mkdir -p objs/logs)
$(shell mkdir -p logs)

default: objs/nginx

objs/nginx:	\
	objs/src/core/____ngx_palloc.o \
	objs/src/core/____ngx_array.o \
	objs/src/core/____ngx_string.o \
	objs/src/core/____ngx_connection.o \
	objs/src/core/nginx.o \
	objs/src/core/ngx_cycle.o \
	objs/src/os/unix/____ngx_alloc.o \
	objs/src/os/unix/____ngx_setproctitle.o \
	objs/src/os/unix/____ngx_linux_init.o \
	objs/src/os/unix/ngx_process.o \
	objs/src/os/unix/ngx_daemon.o \
	objs/src/os/unix/ngx_setaffinity.o \
	objs/src/os/unix/ngx_process_cycle.o \

	$(LINK) -o objs/nginx \
	objs/src/core/____ngx_palloc.o \
	objs/src/core/____ngx_array.o \
	objs/src/core/____ngx_string.o \
	objs/src/core/____ngx_connection.o \
	objs/src/core/nginx.o \
	objs/src/core/ngx_cycle.o \
	objs/src/os/unix/____ngx_alloc.o \
	objs/src/os/unix/____ngx_setproctitle.o \
	objs/src/os/unix/____ngx_linux_init.o \
	objs/src/os/unix/ngx_process.o \
	objs/src/os/unix/ngx_daemon.o \
	objs/src/os/unix/ngx_setaffinity.o \
	objs/src/os/unix/ngx_process_cycle.o \
	 
	
clean:
	rm -rf objs logs
	

upgrade:
	/media/sf_Proj/nginx_process/objs/sbin/nginx -t

	kill -USR2 `cat /media/sf_Proj/nginx_process/objs/logs/nginx.pid`
	sleep 1
	test -f /media/sf_Proj/nginx_process/objs/logs/nginx.pid.oldbin

	kill -QUIT `cat /media/sf_Proj/nginx_process/objs/logs/nginx.pid.oldbin`

	

objs/src/core/nginx.o:	$(CORE_DEPS) src/core/nginx.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/core/nginx.o src/core/nginx.c


objs/src/core/____ngx_palloc.o:	$(CORE_DEPS) src/core/____ngx_palloc.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/core/____ngx_palloc.o src/core/____ngx_palloc.c


objs/src/core/____ngx_array.o:	$(CORE_DEPS) src/core/____ngx_array.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/core/____ngx_array.o src/core/____ngx_array.c


objs/src/core/____ngx_string.o:	$(CORE_DEPS) src/core/____ngx_string.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/core/____ngx_string.o src/core/____ngx_string.c


objs/src/core/____ngx_connection.o:	$(CORE_DEPS) src/core/____ngx_connection.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/core/____ngx_connection.o src/core/____ngx_connection.c


objs/src/core/ngx_cycle.o:	$(CORE_DEPS) src/core/ngx_cycle.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/core/ngx_cycle.o src/core/ngx_cycle.c


objs/src/os/unix/____ngx_alloc.o:	$(CORE_DEPS) src/os/unix/____ngx_alloc.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/os/unix/____ngx_alloc.o src/os/unix/____ngx_alloc.c


objs/src/os/unix/ngx_process.o:	$(CORE_DEPS) src/os/unix/ngx_process.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/os/unix/ngx_process.o src/os/unix/ngx_process.c


objs/src/os/unix/ngx_daemon.o:	$(CORE_DEPS) src/os/unix/ngx_daemon.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/os/unix/ngx_daemon.o src/os/unix/ngx_daemon.c


objs/src/os/unix/ngx_setaffinity.o:	$(CORE_DEPS) src/os/unix/ngx_setaffinity.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/os/unix/ngx_setaffinity.o src/os/unix/ngx_setaffinity.c


objs/src/os/unix/____ngx_setproctitle.o:	$(CORE_DEPS) src/os/unix/____ngx_setproctitle.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/os/unix/____ngx_setproctitle.o src/os/unix/____ngx_setproctitle.c


objs/src/os/unix/ngx_process_cycle.o:	$(CORE_DEPS) src/os/unix/ngx_process_cycle.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/os/unix/ngx_process_cycle.o src/os/unix/ngx_process_cycle.c


objs/src/os/unix/____ngx_linux_init.o:	$(CORE_DEPS) src/os/unix/____ngx_linux_init.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) -o objs/src/os/unix/____ngx_linux_init.o src/os/unix/____ngx_linux_init.c


	