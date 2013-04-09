/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>
#include <nginx.h>

ngx_int_t ngx_ncpu;
ngx_int_t ngx_max_sockets;
ngx_uint_t ngx_inherited_nonblocking;
ngx_uint_t ngx_tcp_nodelay_and_tcp_nopush;

struct rlimit rlmt;

ngx_int_t ngx_os_init(ngx_log_t *log)
{
   ngx_uint_t n;

   ngx_init_setproctitle(log);

   ngx_pagesize = getpagesize();
   ngx_cacheline_size = NGX_CPU_CACHE_LINE;

   for (n = ngx_pagesize; n >>= 1; ngx_pagesize_shift++) { /* void */
   }

   if (ngx_ncpu == 0) {
      ngx_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
   }

   if (ngx_ncpu < 1) {
      ngx_ncpu = 1;
   }

   if (getrlimit(RLIMIT_NOFILE, &rlmt) == -1) {
      return NGX_ERROR;
   }

   ngx_max_sockets = (ngx_int_t) rlmt.rlim_cur;

   ngx_inherited_nonblocking = 1;

   return NGX_OK;
}

