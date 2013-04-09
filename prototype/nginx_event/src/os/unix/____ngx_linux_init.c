/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>
#include <ngx_event.h>


struct rlimit rlmt;

ngx_os_io_t ngx_os_io = {
    ngx_unix_recv,
    ngx_udp_unix_recv,
    ngx_unix_send,
    0
};

ngx_int_t ngx_os_init(ngx_log_t *log)
{
   ngx_uint_t n;

   ngx_pagesize = getpagesize();
   ngx_cacheline_size = NGX_CPU_CACHE_LINE;

   for (n = ngx_pagesize; n >>= 1; ngx_pagesize_shift++) { /* void */
   }

   if (getrlimit(RLIMIT_NOFILE, &rlmt) == -1) {
      return NGX_ERROR;
   }

   srandom(ngx_time());

   return NGX_OK;
}


#define NGX_IOVS  16


ssize_t ngx_unix_recv(ngx_connection_t *c, u_char *buf, size_t size)
{
   ssize_t n;
   ngx_err_t err;
   ngx_event_t *rev;

   rev = c->read;

   do {
      n = recv(c->fd, buf, size, 0);

      if (n == 0) {
         rev->ready = 0;
         rev->eof = 1;
         return n;

      } else if (n > 0) {
         if ((size_t) n < size && !(ngx_event_flags & NGX_USE_GREEDY_EVENT)) {
            rev->ready = 0;
         }

         return n;
      }

      err = ngx_socket_errno;

      if (err == NGX_EAGAIN || err == NGX_EINTR) {
         n = NGX_AGAIN;

      } else {
         n = NGX_ERROR;
         break;
      }

   } while (err == NGX_EINTR);

   rev->ready = 0;

   if (n == NGX_ERROR) {
      rev->error = 1;
   }

   return n;
}


ssize_t ngx_udp_unix_recv(ngx_connection_t *c, u_char *buf, size_t size)
{
   ssize_t n;
   ngx_err_t err;
   ngx_event_t *rev;

   rev = c->read;

   do {
      n = recv(c->fd, buf, size, 0);
      if (n >= 0) {
         return n;
      }

      err = ngx_socket_errno;

      if (err == NGX_EAGAIN || err == NGX_EINTR) {
         n = NGX_AGAIN;
      } else {
         n = NGX_ERROR;
         break;
      }

   } while (err == NGX_EINTR);

   rev->ready = 0;

   if (n == NGX_ERROR) {
      rev->error = 1;
   }

   return n;
}


ssize_t ngx_unix_send(ngx_connection_t *c, u_char *buf, size_t size)
{
   ssize_t n;
   ngx_err_t err;
   ngx_event_t *wev;

   wev = c->write;

   for (;;) {
      n = send(c->fd, buf, size, 0);

      if (n > 0) {
         if (n < (ssize_t) size) {
            wev->ready = 0;
         }

         c->sent += n;

         return n;
      }

      err = ngx_socket_errno;

      if (n == 0) {
         wev->ready = 0;
         return n;
      }

      if (err == NGX_EAGAIN || err == NGX_EINTR) {
         wev->ready = 0;

         if (err == NGX_EAGAIN) {
            return NGX_AGAIN;
         }

      } else {
         wev->error = 1;
         return NGX_ERROR;
      }
   }
}


