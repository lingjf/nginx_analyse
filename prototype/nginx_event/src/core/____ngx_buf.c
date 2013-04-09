/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>

ngx_buf_t *
ngx_create_temp_buf(ngx_pool_t *pool, size_t size)
{
   ngx_buf_t *b;

   b = ngx_palloc(pool, sizeof(ngx_buf_t));
   if (b == NULL) {
      return NULL;
   }

   b->start = ngx_palloc(pool, size);
   if (b->start == NULL) {
      return NULL;
   }

   b->pos = b->start;
   b->last = b->start;
   b->end = b->last + size;

   return b;
}

u_char *jeff_buf_tustring(ngx_buf_t *b)
{
   static u_char buffer[1024 * 8];
   memset(buffer, 0, sizeof(buffer));
   if (!b) return "NULL";
   ngx_snprintf(buffer, sizeof(buffer), "ngx_buf_t{end-start=%d,last-pos=%d}", b->end - b->start, b->last - b->pos);
   return buffer;
}

