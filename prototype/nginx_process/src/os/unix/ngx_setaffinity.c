/*
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>

void ngx_setaffinity(uint64_t cpu_affinity, ngx_log_t *log)
{
   cpu_set_t mask;
   ngx_uint_t i;

   CPU_ZERO(&mask);
   i = 0;
   do {
      if (cpu_affinity & 1) {
         CPU_SET(i, &mask);
      }
      i++;
      cpu_affinity >>= 1;
   } while (cpu_affinity);

   if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1) {
      printf("sched_setaffinity() failed");
   }
}

