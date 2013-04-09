/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>


ngx_module_t ngx_core_module = {
   NULL,                                  /* module context */
   NGX_CORE_MODULE,                       /* module type */
   NULL,                                  /* init master */
   NULL,                                  /* init module */
   NULL,                                  /* init process */
   NULL,                                  /* exit process */
   NULL,                                  /* exit master */
};


int ngx_cdecl main(int argc, char * const *argv)
{
   ngx_int_t i;
   ngx_log_t *log;
   ngx_cycle_t *cycle;

   ngx_time_init();

   if (ngx_os_init(log) != NGX_OK) {
      return 1;
   }

   cycle = ngx_init_cycle(NULL);
   if (cycle == NULL) {
      return 1;
   }

   ngx_cycle = cycle;

   for (i = 0; ngx_modules[i]; i++) {
      if (ngx_modules[i]->init_process) {
         ngx_modules[i]->init_process(cycle);
      }
   }

   for (;;) {

      ngx_process_events_and_timers(cycle);

   }

   return 0;
}


