/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>

volatile ngx_cycle_t *ngx_cycle;

ngx_cycle_t *
ngx_init_cycle(ngx_cycle_t *old_cycle)
{
   ngx_uint_t i, n;
   ngx_log_t *log;
   ngx_pool_t *pool;
   ngx_cycle_t *cycle;
   ngx_listening_t *ls;

   ngx_time_update();

   pool = ngx_create_pool(NGX_CYCLE_POOL_SIZE, log);
   if (pool == NULL) {
      return NULL;
   }
   pool->log = log;

   cycle = ngx_pcalloc(pool, sizeof(ngx_cycle_t));
   if (cycle == NULL) {
      ngx_destroy_pool(pool);
      return NULL;
   }

   cycle->pool = pool;

   n = 10;
   cycle->listening.elts = ngx_pcalloc(pool, n * sizeof(ngx_listening_t));
   if (cycle->listening.elts == NULL) {
      ngx_destroy_pool(pool);
      return NULL;
   }

   cycle->listening.nelts = 0;
   cycle->listening.size = sizeof(ngx_listening_t);
   cycle->listening.nalloc = n;
   cycle->listening.pool = pool;

   ngx_queue_init(&cycle->reusable_connections_queue);

   for (i = 0; ngx_modules[i]; i++) {
      if (ngx_modules[i]->init_module) {
         if (ngx_modules[i]->init_module(cycle) != NGX_OK) {
            /* fatal */
            exit(1);
         }
      }
   }

   ngx_open_listening_sockets(cycle);

   return cycle;
}

