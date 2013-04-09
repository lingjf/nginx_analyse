/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>
#include <ngx_event.h>

static ngx_int_t ngx_enable_accept_events(ngx_cycle_t *cycle);
static ngx_int_t ngx_disable_accept_events(ngx_cycle_t *cycle);
static void ngx_close_accepted_connection(ngx_connection_t *c);

void ngx_event_accept(ngx_event_t *ev)
{
   socklen_t socklen;
   ngx_err_t err;
   ngx_socket_t s;
   ngx_event_t *rev, *wev;
   ngx_listening_t *ls;
   ngx_connection_t *c, *lc;
   u_char sa[NGX_SOCKADDRLEN];

   if (ev->timedout) {
      if (ngx_enable_accept_events((ngx_cycle_t *) ngx_cycle) != NGX_OK) {
         return;
      }

      ev->timedout = 0;
   }

   ev->available = 1;

   lc = ev->data;
   ls = lc->listening;
   ev->ready = 0;

   do {
      socklen = NGX_SOCKADDRLEN;
      s = accept(lc->fd, (struct sockaddr *) sa, &socklen);
      if (s == -1) {
         err = ngx_socket_errno;
         if (err == NGX_EAGAIN) {
            return;
         }

         if (err == NGX_ECONNABORTED) {
            if (ev->available) {
               continue;
            }
         }

         if (err == NGX_EMFILE || err == NGX_ENFILE) {
            if (ngx_disable_accept_events((ngx_cycle_t *) ngx_cycle) != NGX_OK) {
               return;
            }

            ngx_add_timer(ev, 100);
         }

         return;
      }

      c = ngx_get_connection(s, ev->log);
      if (c == NULL) {
         ngx_close_socket(s);
         return;
      }

      c->pool = ngx_create_pool(1024 * 4, ev->log);
      if (c->pool == NULL) {
         ngx_close_accepted_connection(c);
         return;
      }

      c->sockaddr = ngx_palloc(c->pool, socklen);
      if (c->sockaddr == NULL) {
         ngx_close_accepted_connection(c);
         return;
      }

      ngx_memcpy(c->sockaddr, sa, socklen);

      ngx_nonblocking(s);

      c->recv = ngx_recv;
      c->send = ngx_send;

      c->socklen = socklen;
      c->listening = ls;

      rev = c->read;
      wev = c->write;

      wev->ready = 1;

      c->addr_text.data = ngx_pnalloc(c->pool, ls->addr_text_max_len);
      if (c->addr_text.data == NULL) {
         ngx_close_accepted_connection(c);
         return;
      }

      c->addr_text.len = ngx_sock_ntop(c->sockaddr, c->addr_text.data, ls->addr_text_max_len, 0);
      if (c->addr_text.len == 0) {
         ngx_close_accepted_connection(c);
         return;
      }

      if (ngx_add_conn && (ngx_event_flags & NGX_USE_EPOLL_EVENT) == 0) {
         if (ngx_add_conn(c) == NGX_ERROR) {
            ngx_close_accepted_connection(c);
            return;
         }
      }

      ls->handler(c); /* ngx_http_init_connection */

   } while (ev->available);
}

static ngx_int_t ngx_enable_accept_events(ngx_cycle_t *cycle)
{
   ngx_uint_t i;
   ngx_listening_t *ls;
   ngx_connection_t *c;

   ls = cycle->listening.elts;
   for (i = 0; i < cycle->listening.nelts; i++) {
      c = ls[i].connection;

      if (c->read->active) {
         continue;
      }

      if (ngx_add_event(c->read, NGX_READ_EVENT, 0) == NGX_ERROR) {
         return NGX_ERROR;
      }
   }

   return NGX_OK;
}

static ngx_int_t ngx_disable_accept_events(ngx_cycle_t *cycle)
{
   ngx_uint_t i;
   ngx_listening_t *ls;
   ngx_connection_t *c;

   ls = cycle->listening.elts;
   for (i = 0; i < cycle->listening.nelts; i++) {

      c = ls[i].connection;

      if (!c->read->active) {
         continue;
      }

      if (ngx_del_event(c->read, NGX_READ_EVENT, NGX_DISABLE_EVENT) == NGX_ERROR) {
         return NGX_ERROR;
      }
   }

   return NGX_OK;
}

static void ngx_close_accepted_connection(ngx_connection_t *c)
{
   ngx_socket_t fd;

   ngx_free_connection(c);

   fd = c->fd;
   c->fd = (ngx_socket_t) -1;

   ngx_close_socket(fd);

   if (c->pool) {
      ngx_destroy_pool(c->pool);
   }
}

