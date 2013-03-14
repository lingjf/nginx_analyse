
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_buf_t *
ngx_create_temp_buf(ngx_pool_t *pool, size_t size)
{
    ngx_buf_t *b;

    b = ngx_calloc_buf(pool);
    if (b == NULL) {
        return NULL;
    }

    b->start = ngx_palloc(pool, size);
    if (b->start == NULL) {
        return NULL;
    }

    /*
     * set by ngx_calloc_buf():
     *
     *     b->file_pos = 0;
     *     b->file_last = 0;
     *     b->file = NULL;
     *     b->shadow = NULL;
     *     b->tag = 0;
     *     and flags
     */

    b->pos = b->start;
    b->last = b->start;
    b->end = b->last + size;
    b->temporary = 1;

    return b;
}


ngx_chain_t *
ngx_alloc_chain_link(ngx_pool_t *pool)
{
    ngx_chain_t  *cl;

    cl = pool->chain;

    if (cl) {
        pool->chain = cl->next;
        return cl;
    }

    cl = ngx_palloc(pool, sizeof(ngx_chain_t));
    if (cl == NULL) {
        return NULL;
    }

    return cl;
}


ngx_chain_t *
ngx_create_chain_of_bufs(ngx_pool_t *pool, ngx_bufs_t *bufs)
{
    u_char       *p;
    ngx_int_t     i;
    ngx_buf_t    *b;
    ngx_chain_t  *chain, *cl, **ll;

    p = ngx_palloc(pool, bufs->num * bufs->size);
    if (p == NULL) {
        return NULL;
    }

    ll = &chain;

    for (i = 0; i < bufs->num; i++) {

        b = ngx_calloc_buf(pool);
        if (b == NULL) {
            return NULL;
        }

        /*
         * set by ngx_calloc_buf():
         *
         *     b->file_pos = 0;
         *     b->file_last = 0;
         *     b->file = NULL;
         *     b->shadow = NULL;
         *     b->tag = 0;
         *     and flags
         *
         */

        b->pos = p;
        b->last = p;
        b->temporary = 1;

        b->start = p;
        p += bufs->size;
        b->end = p;

        cl = ngx_alloc_chain_link(pool);
        if (cl == NULL) {
            return NULL;
        }

        cl->buf = b;
        *ll = cl;
        ll = &cl->next;
    }

    *ll = NULL;

    return chain;
}

/*
 * Copy ngx_chain_t in 'in' not but ngx_buf_t, and append to the end of 'chain'
 */
ngx_int_t
ngx_chain_add_copy(ngx_pool_t *pool, ngx_chain_t **chain, ngx_chain_t *in)
{
    ngx_chain_t  *cl, **ll;

    ll = chain;

    for (cl = *chain; cl; cl = cl->next) {
        ll = &cl->next;
    }

    while (in) {
        cl = ngx_alloc_chain_link(pool);
        if (cl == NULL) {
            return NGX_ERROR;
        }

        cl->buf = in->buf;
        *ll = cl;
        ll = &cl->next;
        in = in->next;
    }

    *ll = NULL;

    return NGX_OK;
}


ngx_chain_t *
ngx_chain_get_free_buf(ngx_pool_t *p, ngx_chain_t **free)
{
    ngx_chain_t  *cl;

    if (*free) {
        cl = *free;
        *free = cl->next;
        cl->next = NULL;
        return cl;
    }

    cl = ngx_alloc_chain_link(p);
    if (cl == NULL) {
        return NULL;
    }

    cl->buf = ngx_calloc_buf(p);
    if (cl->buf == NULL) {
        return NULL;
    }

    cl->next = NULL;

    return cl;
}

/*
 * 将out中已处理掉的挂入free， 未处理掉的挂入busy
 */
void
ngx_chain_update_chains(ngx_pool_t *p, ngx_chain_t **free, ngx_chain_t **busy, ngx_chain_t **out, ngx_buf_tag_t tag)
{
    ngx_chain_t  *cl;

    if (*busy == NULL) {
        *busy = *out;

    } else {
        for (cl = *busy; cl->next; cl = cl->next) { /* void */ }

        cl->next = *out;
    }

    *out = NULL;

    while (*busy) {
        cl = *busy;

        if (ngx_buf_size(cl->buf) != 0) {
            break;
        }

        if (cl->buf->tag != tag) {
            *busy = cl->next;
            ngx_free_chain(p, cl);
            continue;
        }

        cl->buf->pos = cl->buf->start;
        cl->buf->last = cl->buf->start;

        *busy = cl->next;
        cl->next = *free;
        *free = cl;
    }
}

u_char *jeff_buf_tustring(ngx_buf_t *b)
{
   static u_char buffer[1024 * 8];
   memset(buffer, 0, sizeof(buffer));
   if (!b) return "NULL";
   ngx_snprintf(buffer, sizeof(buffer), "ngx_buf_t{end-start=%d,last-pos=%d", b->end - b->start, b->last - b->pos);
   if (b->temporary) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",temporary");
   }
   if (b->memory) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",memory");
   }
   if (b->mmap) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",mmap");
   }
   if (b->recycled) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",recycled");
   }
   if (b->in_file) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",in_file");
   }
   if (b->flush) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",flush");
   }
   if (b->sync) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",sync");
   }
   if (b->last_buf) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",last_buf");
   }
   if (b->last_in_chain) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",last_in_chain");
   }
   if (b->last_shadow) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",last_shadow");
   }
   if (b->temp_file) {
      ngx_snstrcatf(buffer, sizeof(buffer), ",temp_file");
   }
   ngx_snstrcatf(buffer, sizeof(buffer), "}");
   return buffer;
}

u_char *jeff_chain_tustring(ngx_chain_t *c)
{
   static u_char buffer[1024 * 8];
   memset(buffer, 0, sizeof(buffer));
   if (!c) return "NULL";
   ngx_snprintf(buffer, sizeof(buffer), "ngx_chain_t{");
   for (; c ; c = c->next) {
      ngx_buf_t *b = c->buf;
      if (b) {
         ngx_snstrcatf(buffer, sizeof(buffer), "%s,",jeff_buf_tustring(b));
      }
   }
   ngx_snstrcatf(buffer, sizeof(buffer), "}");
   return buffer;
}
