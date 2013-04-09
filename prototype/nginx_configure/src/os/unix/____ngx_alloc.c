/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>

ngx_uint_t ngx_pagesize = 1024 * 4;
ngx_uint_t ngx_pagesize_shift;
ngx_uint_t ngx_cacheline_size = NGX_CPU_CACHE_LINE;

void *
ngx_alloc(size_t size, ngx_log_t *log)
{
   void * p = malloc(size);
   if (p == NULL) {
      printf("malloc(%uz) failed", size);
   }
   return p;
}

void *
ngx_calloc(size_t size, ngx_log_t *log)
{
   void *p = ngx_alloc(size, log);
   if (p) {
      ngx_memzero(p, size);
   }
   return p;
}

void *
ngx_memalign(size_t alignment, size_t size, ngx_log_t *log)
{
   void *p;
   int err;
   err = posix_memalign(&p, alignment, size);
   if (err) {
      printf("posix_memalign(%uz, %uz) failed", alignment, size);
      p = NULL;
   }
   return p;
}

