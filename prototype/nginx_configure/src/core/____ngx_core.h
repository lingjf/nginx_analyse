
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CORE_H_INCLUDED_
#define _NGX_CORE_H_INCLUDED_


typedef struct ngx_module_s      ngx_module_t;
typedef struct ngx_conf_s        ngx_conf_t;
typedef struct ngx_cycle_s       ngx_cycle_t;
typedef struct ngx_pool_s        ngx_pool_t;
typedef struct ngx_chain_s       ngx_chain_t;
typedef struct ngx_log_s         ngx_log_t;
typedef struct ngx_array_s       ngx_array_t;
typedef struct ngx_open_file_s   ngx_open_file_t;
typedef struct ngx_command_s     ngx_command_t;
typedef struct ngx_file_s        ngx_file_t;
typedef struct ngx_event_s       ngx_event_t;
typedef int    ngx_connection_t;
typedef int    ngx_msec_t;
typedef int    ngx_msec_int_t;

#define  NGX_OK          0
#define  NGX_ERROR      -1
#define  NGX_AGAIN      -2
#define  NGX_BUSY       -3
#define  NGX_DONE       -4
#define  NGX_DECLINED   -5
#define  NGX_ABORT      -6


#include <____ngx_errno.h>
#include <____ngx_string.h>
#include <____ngx_files.h>
#include <____ngx_parse.h>
#include <____ngx_log.h>
#include <____ngx_alloc.h>
#include <____ngx_palloc.h>
#include <____ngx_buf.h>
#include <____ngx_queue.h>
#include <____ngx_array.h>
#include <____ngx_list.h>
#include <____ngx_hash.h>
#include <____ngx_inet.h>
#include <ngx_cycle.h>
#include <ngx_conf_file.h>


#define LF     (u_char) 10
#define CR     (u_char) 13
#define CRLF   "\x0d\x0a"


#define ngx_abs(value)       (((value) >= 0) ? (value) : - (value))
#define ngx_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define ngx_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))


#endif /* _NGX_CORE_H_INCLUDED_ */
