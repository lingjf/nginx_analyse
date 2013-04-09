
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CORE_H_INCLUDED_
#define _NGX_CORE_H_INCLUDED_

typedef int                      ngx_fd_t;
typedef int                      ngx_log_t;
typedef int                      ngx_err_t;

typedef struct ngx_module_s      ngx_module_t;
typedef struct ngx_cycle_s       ngx_cycle_t;
typedef struct ngx_pool_s        ngx_pool_t;
typedef struct ngx_array_s       ngx_array_t;
typedef struct ngx_event_s       ngx_event_t;
typedef struct ngx_connection_s  ngx_connection_t;

typedef void (*ngx_event_handler_pt)(ngx_event_t *ev);
typedef void (*ngx_connection_handler_pt)(ngx_connection_t *c);


#define  NGX_OK          0
#define  NGX_ERROR      -1
#define  NGX_AGAIN      -2
#define  NGX_BUSY       -3
#define  NGX_DONE       -4
#define  NGX_DECLINED   -5
#define  NGX_ABORT      -6


#define NGX_EPERM         EPERM
#define NGX_ENOENT        ENOENT
#define NGX_ENOPATH       ENOENT
#define NGX_ESRCH         ESRCH
#define NGX_EINTR         EINTR
#define NGX_ECHILD        ECHILD
#define NGX_ENOMEM        ENOMEM
#define NGX_EACCES        EACCES
#define NGX_EBUSY         EBUSY
#define NGX_EEXIST        EEXIST
#define NGX_EXDEV         EXDEV
#define NGX_ENOTDIR       ENOTDIR
#define NGX_EISDIR        EISDIR
#define NGX_EINVAL        EINVAL
#define NGX_ENFILE        ENFILE
#define NGX_EMFILE        EMFILE
#define NGX_ENOSPC        ENOSPC
#define NGX_EPIPE         EPIPE
#define NGX_EINPROGRESS   EINPROGRESS
#define NGX_EADDRINUSE    EADDRINUSE
#define NGX_ECONNABORTED  ECONNABORTED
#define NGX_ECONNRESET    ECONNRESET
#define NGX_ENOTCONN      ENOTCONN
#define NGX_ETIMEDOUT     ETIMEDOUT
#define NGX_ECONNREFUSED  ECONNREFUSED
#define NGX_ENAMETOOLONG  ENAMETOOLONG
#define NGX_ENETDOWN      ENETDOWN
#define NGX_ENETUNREACH   ENETUNREACH
#define NGX_EHOSTDOWN     EHOSTDOWN
#define NGX_EHOSTUNREACH  EHOSTUNREACH
#define NGX_ENOSYS        ENOSYS
#define NGX_ECANCELED     ECANCELED
#define NGX_EILSEQ        EILSEQ
#define NGX_ENOMOREFILES  0


#define NGX_EAGAIN        EAGAIN


#define ngx_errno                  errno
#define ngx_socket_errno           errno


#define LF     (u_char) 10
#define CR     (u_char) 13
#define CRLF   "\x0d\x0a"


#define ngx_abs(value)       (((value) >= 0) ? (value) : - (value))
#define ngx_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define ngx_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))

#define NGX_INET_ADDRSTRLEN   (sizeof("255.255.255.255") - 1)
#define NGX_SOCKADDR_STRLEN   (NGX_INET_ADDRSTRLEN + sizeof(":65535") - 1)
#define NGX_SOCKADDRLEN       512

#define ngx_memory_barrier() __sync_synchronize()

#include <____ngx_rbtree.h>
#include <____ngx_time.h>
#include <____ngx_socket.h>
#include <____ngx_string.h>
#include <____ngx_alloc.h>
#include <____ngx_palloc.h>
#include <____ngx_buf.h>
#include <____ngx_queue.h>
#include <____ngx_array.h>
#include <____ngx_times.h>
#include <____ngx_linux.h>
#include <ngx_cycle.h>
#include <ngx_connection.h>
#include <ngx_event.h>

#endif /* _NGX_CORE_H_INCLUDED_ */
