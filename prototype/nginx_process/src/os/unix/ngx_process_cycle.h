/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#ifndef _NGX_PROCESS_CYCLE_H_INCLUDED_
#define _NGX_PROCESS_CYCLE_H_INCLUDED_

#include <____ngx_config.h>
#include <____ngx_core.h>

#define NGX_PROCESS_SINGLE     0
#define NGX_PROCESS_MASTER     1
#define NGX_PROCESS_SIGNALLER  2
#define NGX_PROCESS_WORKER     3
#define NGX_PROCESS_HELPER     4

void ngx_master_process_cycle(ngx_cycle_t *cycle);

extern ngx_uint_t ngx_process;
extern ngx_pid_t ngx_pid;
extern ngx_pid_t ngx_new_binary;
extern ngx_uint_t ngx_exiting;

extern sig_atomic_t ngx_reap;
extern sig_atomic_t ngx_sigalrm;
extern sig_atomic_t ngx_quit;
extern sig_atomic_t ngx_terminate;
extern sig_atomic_t ngx_reconfigure;
extern sig_atomic_t ngx_change_binary;

#endif /* _NGX_PROCESS_CYCLE_H_INCLUDED_ */
