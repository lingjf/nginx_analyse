
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <____ngx_config.h>
#include <____ngx_core.h>

typedef struct ngx_http_request_s
{
   ngx_pool_t *pool;
   ngx_connection_t *connection;

   ngx_buf_t *buf, *out;
} ngx_http_request_t;

#define NGX_HTTP_MODULE           0x50545448   /* "HTTP" */

static ngx_int_t ngx_http_test_module_init(ngx_cycle_t *cycle);
static void ngx_http_init_connection(ngx_connection_t *c);
static void ngx_http_init_request(ngx_event_t *rev);
static void ngx_http_empty_handler(ngx_event_t *ev);
static ssize_t ngx_http_read_request_header(ngx_http_request_t *r);
static void ngx_http_process_request_line(ngx_event_t *rev);
static ngx_int_t ngx_http_parse_request_line(ngx_http_request_t *r);
static void ngx_http_process_request_headers(ngx_event_t *rev);
static ngx_int_t ngx_http_parse_header_line(ngx_http_request_t *r);
static void ngx_http_process_request(ngx_http_request_t *r);
static void ngx_http_reader(ngx_event_t *rev);
static void ngx_http_writer(ngx_event_t *wev);
static void ngx_http_close_connection(ngx_connection_t *c);
u_char *jeff_http_request_tustring(ngx_http_request_t *r);

ngx_module_t  ngx_http_test_module = {
    NULL,                                  /* module context */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    ngx_http_test_module_init,             /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
};

static ngx_int_t ngx_http_test_module_init(ngx_cycle_t *cycle)
{
   ngx_listening_t *ls;
   struct sockaddr_in my_addr;

   my_addr.sin_family = AF_INET;
   my_addr.sin_port = htons(8000);
   my_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
   bzero(&(my_addr.sin_zero), 8);

   ls = ngx_create_listening(cycle, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
   if (ls == NULL) {
      return NGX_ERROR;
   }

   ls->handler = ngx_http_init_connection;

   return NGX_OK;
}

void ngx_http_init_connection(ngx_connection_t *c)
{
   ngx_event_t *rev;

   printf("%s accepted\n", jeff_connection_tustring(c));
   rev = c->read;
   rev->handler = ngx_http_init_request;
   c->write->handler = ngx_http_empty_handler;

   ngx_add_timer(rev, 1000);

   if (ngx_handle_read_event(rev, 0) != NGX_OK) {
      return;
   }
}

static void ngx_http_init_request(ngx_event_t *rev)
{
   ngx_connection_t *c;
   ngx_http_request_t *r;

   c = rev->data;

   printf("http request arrived\n");

   if (rev->timedout) {
      printf("client timed out\n");
      ngx_http_close_connection(c);
      return;
   }

   r = ngx_pcalloc(c->pool, sizeof(ngx_http_request_t));
   r->pool = c->pool;
   r->connection = c;
   r->buf = ngx_create_temp_buf(c->pool, 1024 * 4);
   c->data = r;

   rev->handler = ngx_http_process_request_line;
   rev->handler(rev);
}

void ngx_http_empty_handler(ngx_event_t *ev)
{
   return;
}

static void ngx_http_process_request_line(ngx_event_t *rev)
{
   ssize_t n;
   ngx_int_t rc, rv;
   ngx_http_request_t *r;
   ngx_connection_t *c;

   c = rev->data;
   r = c->data;

   if (rev->timedout) {
      printf("client timed out\n");
      ngx_http_close_connection(c);
      return;
   }

   rc = NGX_AGAIN;

   for (;;) {

      if (rc == NGX_AGAIN) {
         n = ngx_http_read_request_header(r);
         if (n == NGX_AGAIN || n == NGX_ERROR) {
            return;
         }
      }

      rc = ngx_http_parse_request_line(r);
      if (rc == NGX_OK) {
         printf("request line done\n");
         rev->handler = ngx_http_process_request_headers;
         ngx_http_process_request_headers(rev);
         return;
      }

   }
}

static ssize_t ngx_http_read_request_header(ngx_http_request_t *r)
{
   ssize_t n;
   ngx_event_t *rev;
   ngx_connection_t *c;

   c = r->connection;
   rev = c->read;

   n = r->buf->last - r->buf->pos;
   if (n > 0) {
      return n;
   }

   if (rev->ready) {
      n = c->recv(c, r->buf->last, r->buf->end - r->buf->last);
   } else {
      n = NGX_AGAIN;
   }

   if (n == NGX_AGAIN) {
      if (!rev->timer_set) {
         ngx_add_timer(rev, 100);
      }

      ngx_handle_read_event(rev, 0);
      return NGX_AGAIN;
   }

   if (n == 0) {
      printf("client prematurely closed connection");
   }

   if (n == 0 || n == NGX_ERROR) {
      return NGX_ERROR;
   }

   r->buf->last += n;

   printf("%s received\n", jeff_http_request_tustring(r));

   return n;
}

static ngx_int_t ngx_http_parse_request_line(ngx_http_request_t *r)
{
   ssize_t n;
   u_char *p;

   n = r->buf->last - r->buf->pos;
   p = ngx_strnstr(r->buf->pos, "\r\n", n);
   if (p == NULL) {
      r->buf->pos = r->buf->last;
      return NGX_AGAIN;
   }
   r->buf->pos = p + 2;
   return NGX_OK;
}

static void ngx_http_process_request_headers(ngx_event_t *rev)
{
   u_char *p;
   ssize_t n;
   ngx_int_t rc;
   ngx_connection_t *c;
   ngx_http_request_t *r;

   c = rev->data;
   r = c->data;

   if (rev->timedout) {
      printf("client timed out\n");
      ngx_http_close_connection(c);
      rev->timedout = 0;
      return;
   }

   rc = NGX_AGAIN;

   for (;;) {

      if (rc == NGX_AGAIN) {
         n = ngx_http_read_request_header(r);
         if (n == NGX_AGAIN || n == NGX_ERROR) {
            return;
         }
      }

      rc = ngx_http_parse_header_line(r);
      if (rc == NGX_OK) {
         printf("request header done \n");
         ngx_http_process_request(r);
         return;
      }
   }
}

static ngx_int_t ngx_http_parse_header_line(ngx_http_request_t *r)
{
   ssize_t n;
   u_char *p;

   n = r->buf->last - r->buf->pos;
   p = ngx_strnstr(r->buf->pos, "\r\n", n);
   if (p == NULL) {
      r->buf->pos = r->buf->last;
      return NGX_AGAIN;
   }
   r->buf->pos = p + 2;
   return NGX_OK;
}



static void ngx_http_process_request(ngx_http_request_t *r)
{
   ngx_connection_t *c;

   c = r->connection;

   if (c->read->timer_set) {
      ngx_del_timer(c->read);
   }

   c->read->handler = ngx_http_reader;
   c->write->handler = ngx_http_writer;

   static u_char *response = (u_char *) "HTTP/1.1 200 OK" CRLF
   "Content-Type: text/plain" CRLF
   "Content-Length: 10" CRLF
   CRLF
   "Event Done";

   r->out = ngx_palloc(r->pool, sizeof(ngx_buf_t));
   r->out->start = response;
   r->out->pos = r->out->start;
   r->out->end = response + ngx_strlen(response);
   r->out->last = r->out->end;

   ngx_http_writer(c->write);
}

static void ngx_http_reader(ngx_event_t *rev)
{
   ssize_t n;
   ngx_connection_t *c;
   ngx_http_request_t *r;

   c = rev->data;
   r = c->data;

   if (rev->ready) {
      n = c->recv(c, r->buf->last, r->buf->end - r->buf->last);
      if (n > 0) {
         r->buf->last += n;
      }
      if (n == 0) {
         /* connection closed by client */
         ngx_http_close_connection(c);
      }
      rev->ready = 0;
   }
}

static void ngx_http_writer(ngx_event_t *wev)
{
   ssize_t n;
   ngx_connection_t *c;
   ngx_http_request_t *r;

   c = wev->data;
   r = c->data;

   if (wev->ready) {
      n = c->send(c, r->out->pos, r->out->last - r->out->pos);
      if (n > 0) {
         r->out->pos += n;
         if (r->out->last == r->out->pos) {
            /* http request completed */
            ngx_del_event(wev, NGX_WRITE_EVENT, 0);
            wev->handler = ngx_http_empty_handler;
         } else {
            /* wait for write event */
            ngx_handle_write_event(c->write, 0);
         }
      }
      wev->ready = 0;
   }
}

static void ngx_http_close_connection(ngx_connection_t *c)
{
   ngx_pool_t *pool;

   pool = c->pool;

   printf("%s closed \n", jeff_connection_tustring(c));

   ngx_close_connection(c);

   ngx_destroy_pool(pool);
}

u_char *jeff_http_request_tustring(ngx_http_request_t *r)
{
   static u_char buffer[1024 * 8];
   ngx_str_t t = {r->buf->last - r->buf->start, r->buf->start};
   memset(buffer, 0, sizeof(buffer));
   if (!r) return "NULL";
   ngx_snprintf(buffer, sizeof(buffer), "ngx_http_request_t{\n%V}", &t);
   return buffer;
}
