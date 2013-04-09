/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#ifndef _NGX_BUF_H_INCLUDED_
#define _NGX_BUF_H_INCLUDED_

#include <____ngx_config.h>
#include <____ngx_core.h>

typedef struct ngx_buf_s ngx_buf_t;

struct ngx_buf_s
{
   u_char *pos;
   u_char *last;

   u_char *start; /* start of buffer */
   u_char *end; /* end of buffer */
};

#define ngx_buf_size(b)  (off_t) (b->last - b->pos)

ngx_buf_t *ngx_create_temp_buf(ngx_pool_t *pool, size_t size);

u_char *jeff_buf_tustring(ngx_buf_t *b);

#endif /* _NGX_BUF_H_INCLUDED_ */
