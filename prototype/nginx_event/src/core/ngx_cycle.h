
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


#define NGX_CORE_MODULE      0x45524F43  /* "CORE" */
#define NGX_CONF_MODULE      0x464E4F43  /* "CONF" */


struct ngx_module_s {
    void                 *ctx;
    ngx_uint_t            type;

    ngx_int_t           (*init_master)(ngx_log_t *log);

    ngx_int_t           (*init_module)(ngx_cycle_t *cycle);

    ngx_int_t           (*init_process)(ngx_cycle_t *cycle);
    void                (*exit_process)(ngx_cycle_t *cycle);

    void                (*exit_master)(ngx_cycle_t *cycle);
};


struct ngx_cycle_s {
    ngx_pool_t               *pool;
    ngx_log_t                *log;

    ngx_connection_t         *free_connections;
    ngx_uint_t                free_connection_n;

    ngx_queue_t               reusable_connections_queue;

    ngx_array_t               listening;

    ngx_uint_t                connection_n;
    ngx_connection_t         *connections;
    ngx_event_t              *read_events;
    ngx_event_t              *write_events;
};


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);

extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_module_t           ngx_core_module;
extern ngx_module_t  *ngx_modules[];

#endif /* _NGX_CYCLE_H_INCLUDED_ */
