
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <____ngx_config.h>
#include <____ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;

struct ngx_listening_s {
    ngx_socket_t        fd;

    struct sockaddr    *sockaddr;
    socklen_t           socklen;    /* size of sockaddr */
    size_t              addr_text_max_len;
    ngx_str_t           addr_text;

    int                 type;
    int                 backlog;

    /* handler of accepted connection */
    ngx_connection_handler_pt   handler;

    ngx_connection_t   *connection;
};


struct ngx_connection_s {
    void               *data;
    ngx_event_t        *read;
    ngx_event_t        *write;

    ngx_socket_t        fd;

    ngx_recv_pt         recv;
    ngx_send_pt         send;

    ngx_listening_t    *listening;

    off_t               sent;

    ngx_log_t          *log;
    ngx_pool_t         *pool;

    struct sockaddr    *sockaddr;
    socklen_t           socklen;
    ngx_str_t           addr_text;

    ngx_queue_t         queue;
    unsigned            reusable:1;
    unsigned            close:1;

    unsigned            sndlowat:1;
};

ngx_listening_t *ngx_create_listening(ngx_cycle_t *cycle, void *sockaddr, socklen_t socklen);
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);
void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);
void ngx_close_connection(ngx_connection_t *c);

u_char *jeff_listening_tustring(ngx_listening_t *l);
u_char *jeff_connection_tustring(ngx_connection_t *c);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
