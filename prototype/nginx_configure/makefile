
CC =	gcc
CFLAGS =  -O0 -g 
CPP =	gcc -E
LINK =	$(CC)


ALL_INCS = -I src/core \
	-I src/event \
	-I src/event/modules \
	-I src/os/unix \
	-I src \
	-I src/http \
	-I src/http/modules \
	-I src/mail


CORE_DEPS = src/core/nginx.h \
	src/core/____ngx_config.h \
	src/core/____ngx_core.h \
	src/core/____ngx_log.h \
	src/core/____ngx_palloc.h \
	src/core/____ngx_array.h \
	src/core/____ngx_list.h \
	src/core/____ngx_hash.h \
	src/core/____ngx_buf.h \
	src/core/____ngx_queue.h \
	src/core/____ngx_string.h \
	src/core/____ngx_parse.h \
	src/core/____ngx_inet.h \
	src/core/ngx_cycle.h \
	src/core/ngx_conf_file.h \
	src/event/ngx_event.h \
	src/os/unix/____ngx_errno.h \
	src/os/unix/____ngx_alloc.h \
	src/os/unix/____ngx_files.h \
	src/os/unix/____ngx_linux_config.h \
	src/____ngx_auto_config.h


CORE_INCS = -I src/core \
	-I src/event \
	-I src/event/modules \
	-I src/os/unix \
	-I src


HTTP_DEPS = src/http/ngx_http.h \
	src/http/ngx_http_core_module.h \
	src/http/ngx_http_config.h \


HTTP_INCS = -I src/http \
	-I src/http/modules

$(shell mkdir -p objs/src/core)
$(shell mkdir -p objs/src/event)
$(shell mkdir -p objs/src/event/modules)
$(shell mkdir -p objs/src/http)
$(shell mkdir -p objs/src/http/modules)
$(shell mkdir -p objs/src/os/unix)
$(shell mkdir -p objs/logs)
$(shell mkdir -p logs)

default: objs/nginx

objs/nginx:	objs/src/core/nginx.o \
	objs/src/core/____ngx_log.o \
	objs/src/core/____ngx_palloc.o \
	objs/src/core/____ngx_array.o \
	objs/src/core/____ngx_list.o \
	objs/src/core/____ngx_hash.o \
	objs/src/core/____ngx_buf.o \
	objs/src/core/____ngx_queue.o \
	objs/src/core/____ngx_string.o \
	objs/src/core/____ngx_parse.o \
	objs/src/core/____ngx_inet.o \
	objs/src/core/ngx_cycle.o \
	objs/src/core/ngx_conf_file.o \
	objs/src/event/ngx_event.o \
	objs/src/os/unix/____ngx_alloc.o \
	objs/src/os/unix/____ngx_files.o \
	objs/src/event/modules/ngx_epoll_module.o \
	objs/src/http/ngx_http.o \
	objs/src/http/ngx_http_core_module.o \
	objs/src/http/modules/ngx_http_static_module.o \
	objs/src/ngx_modules.o

	$(LINK) -o objs/nginx \
	objs/src/core/nginx.o \
	objs/src/core/____ngx_log.o \
	objs/src/core/____ngx_palloc.o \
	objs/src/core/____ngx_array.o \
	objs/src/core/____ngx_list.o \
	objs/src/core/____ngx_hash.o \
	objs/src/core/____ngx_buf.o \
	objs/src/core/____ngx_queue.o \
	objs/src/core/____ngx_string.o \
	objs/src/core/____ngx_parse.o \
	objs/src/core/____ngx_inet.o \
	objs/src/core/ngx_cycle.o \
	objs/src/core/ngx_conf_file.o \
	objs/src/event/ngx_event.o \
	objs/src/os/unix/____ngx_alloc.o \
	objs/src/os/unix/____ngx_files.o \
	objs/src/event/modules/ngx_epoll_module.o \
	objs/src/http/ngx_http.o \
	objs/src/http/ngx_http_core_module.o \
	objs/src/http/modules/ngx_http_static_module.o \
	objs/src/ngx_modules.o 
	


objs/src/ngx_modules.o:	$(CORE_DEPS) \
	src/ngx_modules.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/ngx_modules.o \
		src/ngx_modules.c


objs/src/core/nginx.o:	$(CORE_DEPS) \
	src/core/nginx.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/nginx.o \
		src/core/nginx.c


objs/src/core/____ngx_log.o:	$(CORE_DEPS) \
	src/core/____ngx_log.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_log.o \
		src/core/____ngx_log.c


objs/src/core/____ngx_palloc.o:	$(CORE_DEPS) \
	src/core/____ngx_palloc.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_palloc.o \
		src/core/____ngx_palloc.c


objs/src/core/____ngx_array.o:	$(CORE_DEPS) \
	src/core/____ngx_array.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_array.o \
		src/core/____ngx_array.c


objs/src/core/____ngx_list.o:	$(CORE_DEPS) \
	src/core/____ngx_list.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_list.o \
		src/core/____ngx_list.c


objs/src/core/____ngx_hash.o:	$(CORE_DEPS) \
	src/core/____ngx_hash.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_hash.o \
		src/core/____ngx_hash.c


objs/src/core/____ngx_buf.o:	$(CORE_DEPS) \
	src/core/____ngx_buf.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_buf.o \
		src/core/____ngx_buf.c


objs/src/core/____ngx_queue.o:	$(CORE_DEPS) \
	src/core/____ngx_queue.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_queue.o \
		src/core/____ngx_queue.c


objs/src/core/____ngx_string.o:	$(CORE_DEPS) \
	src/core/____ngx_string.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_string.o \
		src/core/____ngx_string.c


objs/src/core/____ngx_parse.o:	$(CORE_DEPS) \
	src/core/____ngx_parse.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_parse.o \
		src/core/____ngx_parse.c


objs/src/core/____ngx_inet.o:	$(CORE_DEPS) \
	src/core/____ngx_inet.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_inet.o \
		src/core/____ngx_inet.c


objs/src/core/ngx_cycle.o:	$(CORE_DEPS) \
	src/core/ngx_cycle.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_cycle.o \
		src/core/ngx_cycle.c


objs/src/core/ngx_conf_file.o:	$(CORE_DEPS) \
	src/core/ngx_conf_file.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_conf_file.o \
		src/core/ngx_conf_file.c


objs/src/event/ngx_event.o:	$(CORE_DEPS) \
	src/event/ngx_event.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event.o \
		src/event/ngx_event.c



objs/src/os/unix/____ngx_alloc.o:	$(CORE_DEPS) \
	src/os/unix/____ngx_alloc.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/____ngx_alloc.o \
		src/os/unix/____ngx_alloc.c


objs/src/os/unix/____ngx_files.o:	$(CORE_DEPS) \
	src/os/unix/____ngx_files.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/____ngx_files.o \
		src/os/unix/____ngx_files.c


objs/src/event/modules/ngx_epoll_module.o:	$(CORE_DEPS) \
	src/event/modules/ngx_epoll_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/modules/ngx_epoll_module.o \
		src/event/modules/ngx_epoll_module.c


objs/src/http/ngx_http.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http.o \
		src/http/ngx_http.c


objs/src/http/ngx_http_core_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http_core_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http_core_module.o \
		src/http/ngx_http_core_module.c


objs/src/http/modules/ngx_http_static_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_static_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_static_module.o \
		src/http/modules/ngx_http_static_module.c


objs/src/http/modules/ngx_http_memcached_module.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/modules/ngx_http_memcached_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/modules/ngx_http_memcached_module.o \
		src/http/modules/ngx_http_memcached_module.c


clean:
	rm -rf objs logs