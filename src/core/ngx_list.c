/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <ngx_config.h>
#include <ngx_core.h>

ngx_list_t *
ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
   ngx_list_t *list;

   list = ngx_palloc(pool, sizeof(ngx_list_t));
   if (list == NULL) {
      return NULL;
   }

   list->part.elts = ngx_palloc(pool, n * size);
   if (list->part.elts == NULL) {
      return NULL;
   }

   list->part.nelts = 0;
   list->part.next = NULL;
   list->last = &list->part;
   list->size = size;
   list->nalloc = n;
   list->pool = pool;

   return list;
}

void *
ngx_list_push(ngx_list_t *l)
{
   void *elt;
   ngx_list_part_t *last;

   last = l->last;

   if (last->nelts == l->nalloc) {

      /* the last part is full, allocate a new list part */

      last = ngx_palloc(l->pool, sizeof(ngx_list_part_t));
      if (last == NULL) {
         return NULL;
      }

      last->elts = ngx_palloc(l->pool, l->nalloc * l->size);
      if (last->elts == NULL) {
         return NULL;
      }

      last->nelts = 0;
      last->next = NULL;

      l->last->next = last;
      l->last = last;
   }

   elt = (char *) last->elts + l->size * last->nelts;
   last->nelts++;

   return elt;
}

u_char *jeff_list_tustring(ngx_list_t *l, u_char*(*e)(void*))
{
   static u_char buffer[1024 * 8];
   memset(buffer, 0, sizeof(buffer));
   if (!l) return "NULL";
   ngx_list_part_t *part = &l->part;
   u_char * data = part->elts;
   int part_count = 1;
   int elts_count = 0;
   int i;
   for (i = 0;; i++) {
      if (i >= part->nelts) {
         if (part->next == NULL) {
            break;
         }
         part = part->next;
         data = part->elts;
         i = 0;
         part_count++;
      }
      elts_count++;
   }
   ngx_snprintf(buffer, sizeof(buffer), "ngx_list_t[%d,%d/%d/%d]",
                l->size, part_count, elts_count, l->nalloc);
   return buffer;
}

