/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>
#include <ngx_event.h>

ngx_os_io_t ngx_io;

static void ngx_drain_connections(void);

ngx_listening_t *
ngx_create_listening(ngx_cycle_t *cycle, void *sockaddr, socklen_t socklen)
{
   size_t len;
   ngx_listening_t *ls;
   struct sockaddr *sa;
   u_char text[NGX_SOCKADDR_STRLEN];

   ls = ngx_array_push(&cycle->listening);
   if (ls == NULL) {
      return NULL;
   }

   ngx_memzero(ls, sizeof(ngx_listening_t));

   sa = ngx_palloc(cycle->pool, socklen);
   if (sa == NULL) {
      return NULL;
   }

   ngx_memcpy(sa, sockaddr, socklen);

   ls->sockaddr = sa;
   ls->socklen = socklen;

   len = ngx_sock_ntop(sa, text, NGX_SOCKADDR_STRLEN, 1);
   ls->addr_text.len = len;

   switch (ls->sockaddr->sa_family) {
   case AF_INET:
      ls->addr_text_max_len = NGX_INET_ADDRSTRLEN;
      break;
   default:
      break;
   }

   ls->addr_text.data = ngx_pnalloc(cycle->pool, len);
   if (ls->addr_text.data == NULL) {
      return NULL;
   }

   ngx_memcpy(ls->addr_text.data, text, len);

   ls->fd = (ngx_socket_t) -1;
   ls->type = SOCK_STREAM;

   ls->backlog = NGX_LISTEN_BACKLOG;

   return ls;
}

ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle)
{
   int reuseaddr;
   ngx_uint_t i, tries, failed = 0;
   ngx_err_t err;
   ngx_socket_t s;
   ngx_listening_t *ls;

   reuseaddr = 1;

   for (tries = 5; tries; tries--) {
      failed = 0;

      /* for each listening socket */

      ls = cycle->listening.elts;
      for (i = 0; i < cycle->listening.nelts; i++) {

         if (ls[i].fd != -1) {
            continue;
         }

         s = ngx_socket(ls[i].sockaddr->sa_family, ls[i].type, 0);
         if (s == -1) {
            return NGX_ERROR;
         }

         if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void *) &reuseaddr, sizeof(int)) == -1) {
            ngx_close_socket(s);
            return NGX_ERROR;
         }

         if (ngx_nonblocking(s) == -1) {
            ngx_close_socket(s);
            return NGX_ERROR;
         }

         if (bind(s, ls[i].sockaddr, ls[i].socklen) == -1) {
            err = ngx_socket_errno;
            if (err == NGX_EADDRINUSE) {
               continue;
            }

            ngx_close_socket(s);

            if (err != NGX_EADDRINUSE) {
               return NGX_ERROR;
            }

            failed = 1;
            continue;
         }

         if (listen(s, ls[i].backlog) == -1) {
            ngx_close_socket(s);
            return NGX_ERROR;
         }

         ls[i].fd = s;
      }

      if (!failed) {
         break;
      }

      printf("try again to bind() after 500ms");

      ngx_msleep(500);
   }

   if (failed) {
      return NGX_ERROR;
   }

   return NGX_OK;
}

void ngx_close_listening_sockets(ngx_cycle_t *cycle)
{
   ngx_uint_t i;
   ngx_listening_t *ls;
   ngx_connection_t *c;

   ls = cycle->listening.elts;
   for (i = 0; i < cycle->listening.nelts; i++) {

      c = ls[i].connection;

      if (c) {
         if (c->read->active) {
            if (ngx_event_flags & NGX_USE_EPOLL_EVENT) {

               /*
                * it seems that Linux-2.6.x OpenVZ sends events
                * for closed shared listening sockets unless
                * the events was explicitly deleted
                */

               ngx_del_event(c->read, NGX_READ_EVENT, 0);

            } else {
               ngx_del_event(c->read, NGX_READ_EVENT, NGX_CLOSE_EVENT);
            }
         }

         ngx_free_connection(c);

         c->fd = (ngx_socket_t) -1;
      }

      ngx_close_socket(ls[i].fd);

      ls[i].fd = (ngx_socket_t) -1;
   }
}

ngx_connection_t *
ngx_get_connection(ngx_socket_t s, ngx_log_t *log)
{
   ngx_uint_t instance;
   ngx_event_t *rev, *wev;
   ngx_connection_t *c;

   c = ngx_cycle->free_connections;
   if (c == NULL) {
      ngx_drain_connections();
      c = ngx_cycle->free_connections;
   }

   if (c == NULL) {
      return NULL;
   }

   ngx_cycle->free_connections = c->data;
   ngx_cycle->free_connection_n--;

   rev = c->read;
   wev = c->write;

   ngx_memzero(c, sizeof(ngx_connection_t));

   c->read = rev;
   c->write = wev;
   c->fd = s;
   c->log = log;

   instance = rev->instance;

   ngx_memzero(rev, sizeof(ngx_event_t));
   ngx_memzero(wev, sizeof(ngx_event_t));

   rev->instance = !instance;
   wev->instance = !instance;

   rev->index = NGX_INVALID_INDEX;
   wev->index = NGX_INVALID_INDEX;

   rev->data = c;
   wev->data = c;

   wev->write = 1;

   return c;
}

void ngx_free_connection(ngx_connection_t *c)
{
   c->data = ngx_cycle->free_connections;
   ngx_cycle->free_connections = c;
   ngx_cycle->free_connection_n++;
}

void ngx_close_connection(ngx_connection_t *c)
{
   ngx_socket_t fd;

   if (c->fd == -1) {
      return;
   }

   if (c->read->timer_set) {
      ngx_del_timer(c->read);
   }

   if (c->write->timer_set) {
      ngx_del_timer(c->write);
   }

   if (ngx_del_conn) {
      ngx_del_conn(c, NGX_CLOSE_EVENT);

   } else {
      if (c->read->active || c->read->disabled) {
         ngx_del_event(c->read, NGX_READ_EVENT, NGX_CLOSE_EVENT);
      }

      if (c->write->active || c->write->disabled) {
         ngx_del_event(c->write, NGX_WRITE_EVENT, NGX_CLOSE_EVENT);
      }
   }

   if (c->read->prev) {
      ngx_delete_posted_event(c->read);
   }

   if (c->write->prev) {
      ngx_delete_posted_event(c->write);
   }

   c->read->closed = 1;
   c->write->closed = 1;

   ngx_reusable_connection(c, 0);

   ngx_free_connection(c);

   fd = c->fd;
   c->fd = (ngx_socket_t) -1;

   ngx_close_socket(fd);
}

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable)
{
   if (c->reusable) {
      ngx_queue_remove(&c->queue);
   }

   c->reusable = reusable;

   if (reusable) {
      /* need cast as ngx_cycle is volatile */

      ngx_queue_insert_head( (ngx_queue_t *) &ngx_cycle->reusable_connections_queue, &c->queue);
   }
}

static void ngx_drain_connections(void)
{
   ngx_int_t i;
   ngx_queue_t *q;
   ngx_connection_t *c;

   for (i = 0; i < 32; i++) {
      if (ngx_queue_empty(&ngx_cycle->reusable_connections_queue)) {
         break;
      }

      q = ngx_queue_last(&ngx_cycle->reusable_connections_queue);
      c = ngx_queue_data(q, ngx_connection_t, queue);

      c->close = 1;
      c->read->handler(c->read);
   }
}

u_char *jeff_listening_tustring(ngx_listening_t *l)
{
   static u_char buffer[1024 * 8];
   memset(buffer, 0, sizeof(buffer));
   if (!l) return "NULL";
   ngx_snprintf(buffer, sizeof(buffer), "ngx_listening_t{address=%V}", &l->addr_text);
   return buffer;
}

u_char *jeff_connection_tustring(ngx_connection_t *c)
{
   static u_char buffer[1024 * 8];
   memset(buffer, 0, sizeof(buffer));
   if (!c) return "NULL";
   ngx_snprintf(buffer, sizeof(buffer), "ngx_connection_t{address=%V}", &c->addr_text);
   return buffer;
}

