/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_

#include <____ngx_config.h>
#include <____ngx_core.h>

#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     16384
#endif

#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2

struct ngx_cycle_s
{
   void ****conf_ctx;
   ngx_pool_t *pool;

   ngx_log_t *log;
   ngx_log_t new_log;

   ngx_list_t open_files;
   ngx_uint_t connection_n;

   ngx_cycle_t *old_cycle;

   ngx_str_t conf_file;
   ngx_str_t conf_param;
   ngx_str_t conf_prefix;
   ngx_str_t prefix;
   ngx_str_t lock_file;
   ngx_str_t hostname;
};

typedef struct
{
   ngx_flag_t daemon;
   ngx_flag_t master;

   ngx_msec_t timer_resolution;

   ngx_int_t worker_processes;
   ngx_int_t debug_points;

   ngx_int_t rlimit_nofile;
   ngx_int_t rlimit_sigpending;
   off_t rlimit_core;

   int priority;

   ngx_uint_t cpu_affinity_n;
   uint64_t *cpu_affinity;

   char *username;

   ngx_str_t working_directory;
   ngx_str_t lock_file;

   ngx_str_t pid;
   ngx_str_t oldpid;

   ngx_array_t env;
   char **environment;

} ngx_core_conf_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)

ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);

extern volatile ngx_cycle_t *ngx_cycle;
extern ngx_array_t ngx_old_cycles;
extern ngx_module_t ngx_core_module;
extern ngx_uint_t ngx_test_config;

#endif /* _NGX_CYCLE_H_INCLUDED_ */
