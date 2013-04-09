
CC =	gcc
CFLAGS =  -O0 -g 
CPP =	gcc -E
LINK =	$(CC)


ALL_INCS = -I src/core \
	-I src/event \
	-I src/event/modules \
	-I src/os/unix \
	-I src 


CORE_DEPS = src/core/nginx.h \
	src/core/____ngx_config.h \
	src/core/____ngx_core.h \
	src/core/____ngx_palloc.h \
	src/core/____ngx_buf.h \
	src/core/____ngx_array.h \
	src/core/____ngx_queue.h \
	src/core/____ngx_string.h \
	src/core/____ngx_rbtree.h \
	src/core/____ngx_times.h \
	src/core/ngx_connection.h \
	src/core/ngx_cycle.h \
	src/event/ngx_event.h \
	src/event/ngx_event_timer.h \
	src/event/ngx_event_posted.h \
	src/os/unix/____ngx_time.h \
	src/os/unix/____ngx_alloc.h \
	src/os/unix/____ngx_socket.h \
	src/os/unix/____ngx_linux_config.h \
	src/os/unix/____ngx_linux.h \
	src/____ngx_auto_config.h


CORE_INCS = -I src/core \
	-I src/event \
	-I src/event/modules \
	-I src/os/unix \
	-I src

$(shell mkdir -p objs/src/core)
$(shell mkdir -p objs/src/event)
$(shell mkdir -p objs/src/event/modules)
$(shell mkdir -p objs/src/http)
$(shell mkdir -p objs/src/os/unix)

objs/nginx:	objs/src/core/nginx.o \
	objs/src/core/____ngx_palloc.o \
	objs/src/core/____ngx_buf.o \
	objs/src/core/____ngx_array.o \
	objs/src/core/____ngx_queue.o \
	objs/src/core/____ngx_string.o \
	objs/src/core/____ngx_rbtree.o \
	objs/src/core/____ngx_times.o \
	objs/src/core/ngx_connection.o \
	objs/src/core/ngx_cycle.o \
	objs/src/event/ngx_event.o \
	objs/src/event/ngx_event_timer.o \
	objs/src/event/ngx_event_posted.o \
	objs/src/event/ngx_event_accept.o \
	objs/src/os/unix/____ngx_time.o \
	objs/src/os/unix/____ngx_alloc.o \
	objs/src/os/unix/____ngx_socket.o \
	objs/src/os/unix/____ngx_linux_init.o \
	objs/src/event/modules/ngx_epoll_module.o \
	objs/src/event/modules/ngx_select_module.o \
	objs/src/http/ngx_http_test.o \
	objs/src/____ngx_modules.o

	$(LINK) -o objs/nginx \
	objs/src/core/nginx.o \
	objs/src/core/____ngx_palloc.o \
	objs/src/core/____ngx_buf.o \
	objs/src/core/____ngx_array.o \
	objs/src/core/____ngx_queue.o \
	objs/src/core/____ngx_string.o \
	objs/src/core/____ngx_rbtree.o \
	objs/src/core/____ngx_times.o \
	objs/src/core/ngx_connection.o \
	objs/src/core/ngx_cycle.o \
	objs/src/event/ngx_event.o \
	objs/src/event/ngx_event_timer.o \
	objs/src/event/ngx_event_posted.o \
	objs/src/event/ngx_event_accept.o \
	objs/src/os/unix/____ngx_time.o \
	objs/src/os/unix/____ngx_alloc.o \
	objs/src/os/unix/____ngx_socket.o \
	objs/src/os/unix/____ngx_linux_init.o \
	objs/src/event/modules/ngx_epoll_module.o \
	objs/src/event/modules/ngx_select_module.o \
	objs/src/http/ngx_http_test.o \
	objs/src/____ngx_modules.o 
	

objs/src/____ngx_modules.o:	$(CORE_DEPS) \
	src/____ngx_modules.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/____ngx_modules.o \
		src/____ngx_modules.c


objs/src/core/nginx.o:	$(CORE_DEPS) \
	src/core/nginx.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/nginx.o \
		src/core/nginx.c


objs/src/core/____ngx_palloc.o:	$(CORE_DEPS) \
	src/core/____ngx_palloc.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_palloc.o \
		src/core/____ngx_palloc.c
		
		
objs/src/core/____ngx_buf.o:	$(CORE_DEPS) \
	src/core/____ngx_buf.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_buf.o \
		src/core/____ngx_buf.c


objs/src/core/____ngx_array.o:	$(CORE_DEPS) \
	src/core/____ngx_array.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_array.o \
		src/core/____ngx_array.c


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


objs/src/core/____ngx_rbtree.o:	$(CORE_DEPS) \
	src/core/____ngx_rbtree.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_rbtree.o \
		src/core/____ngx_rbtree.c


objs/src/core/____ngx_times.o:	$(CORE_DEPS) \
	src/core/____ngx_times.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/____ngx_times.o \
		src/core/____ngx_times.c


objs/src/core/ngx_connection.o:	$(CORE_DEPS) \
	src/core/ngx_connection.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_connection.o \
		src/core/ngx_connection.c


objs/src/core/ngx_cycle.o:	$(CORE_DEPS) \
	src/core/ngx_cycle.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/ngx_cycle.o \
		src/core/ngx_cycle.c


objs/src/event/ngx_event.o:	$(CORE_DEPS) \
	src/event/ngx_event.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event.o \
		src/event/ngx_event.c


objs/src/event/ngx_event_timer.o:	$(CORE_DEPS) \
	src/event/ngx_event_timer.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event_timer.o \
		src/event/ngx_event_timer.c


objs/src/event/ngx_event_posted.o:	$(CORE_DEPS) \
	src/event/ngx_event_posted.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event_posted.o \
		src/event/ngx_event_posted.c


objs/src/event/ngx_event_accept.o:	$(CORE_DEPS) \
	src/event/ngx_event_accept.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/ngx_event_accept.o \
		src/event/ngx_event_accept.c


objs/src/os/unix/____ngx_time.o:	$(CORE_DEPS) \
	src/os/unix/____ngx_time.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/____ngx_time.o \
		src/os/unix/____ngx_time.c


objs/src/os/unix/____ngx_alloc.o:	$(CORE_DEPS) \
	src/os/unix/____ngx_alloc.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/____ngx_alloc.o \
		src/os/unix/____ngx_alloc.c


objs/src/os/unix/____ngx_socket.o:	$(CORE_DEPS) \
	src/os/unix/____ngx_socket.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/____ngx_socket.o \
		src/os/unix/____ngx_socket.c


objs/src/os/unix/____ngx_linux_init.o:	$(CORE_DEPS) \
	src/os/unix/____ngx_linux_init.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/os/unix/____ngx_linux_init.o \
		src/os/unix/____ngx_linux_init.c


objs/src/event/modules/ngx_epoll_module.o:	$(CORE_DEPS) \
	src/event/modules/ngx_epoll_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/modules/ngx_epoll_module.o \
		src/event/modules/ngx_epoll_module.c


objs/src/event/modules/ngx_select_module.o:	$(CORE_DEPS) \
	src/event/modules/ngx_select_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/event/modules/ngx_select_module.o \
		src/event/modules/ngx_select_module.c
		
		
objs/src/http/ngx_http_test.o:	$(CORE_DEPS) \
	src/http/ngx_http_test.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/http/ngx_http_test.o \
		src/http/ngx_http_test.c


clean:
	rm -rf objs 
	
	