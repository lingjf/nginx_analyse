/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>
#include <ngx_event.h>

typedef struct
{
   ngx_uint_t events;
   ngx_uint_t aio_requests;
} ngx_epoll_conf_t;

static ngx_int_t ngx_epoll_init(ngx_cycle_t *cycle, ngx_msec_t timer);
static void ngx_epoll_done(ngx_cycle_t *cycle);
static ngx_int_t ngx_epoll_add_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
static ngx_int_t ngx_epoll_del_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
static ngx_int_t ngx_epoll_add_connection(ngx_connection_t *c);
static ngx_int_t ngx_epoll_del_connection(ngx_connection_t *c, ngx_uint_t flags);
static ngx_int_t ngx_epoll_process_events(ngx_cycle_t *cycle, ngx_msec_t timer, ngx_uint_t flags);

static void *ngx_epoll_create_conf(ngx_cycle_t *cycle);
static char *ngx_epoll_init_conf(ngx_cycle_t *cycle, void *conf);

static ngx_str_t epoll_name = ngx_string("epoll");

static ngx_command_t ngx_epoll_commands[] = {
   { ngx_string("epoll_events"), NGX_EVENT_CONF | NGX_CONF_TAKE1, ngx_conf_set_num_slot, 0, offsetof(ngx_epoll_conf_t, events), NULL },
   { ngx_string("worker_aio_requests"), NGX_EVENT_CONF | NGX_CONF_TAKE1, ngx_conf_set_num_slot, 0, offsetof(ngx_epoll_conf_t, aio_requests), NULL },
   ngx_null_command
};

ngx_event_module_t ngx_epoll_module_ctx = {
   &epoll_name,
   ngx_epoll_create_conf, /* create configuration */
   ngx_epoll_init_conf, /* init configuration */
   {
      ngx_epoll_add_event, /* add an event */
      ngx_epoll_del_event, /* delete an event */
      ngx_epoll_add_event, /* enable an event */
      ngx_epoll_del_event, /* disable an event */
      ngx_epoll_add_connection, /* add an connection */
      ngx_epoll_del_connection, /* delete an connection */
      NULL, /* process the changes */
      ngx_epoll_process_events, /* process the events */
      ngx_epoll_init, /* init the events */
      ngx_epoll_done, /* done the events */
   }
};

ngx_module_t ngx_epoll_module = {
   NGX_MODULE_V1,
   &ngx_epoll_module_ctx, /* module context */
   ngx_epoll_commands, /* module directives */
   NGX_EVENT_MODULE, /* module type */
   NULL, /* init master */
   NULL, /* init module */
   NULL, /* init process */
   NULL, /* init thread */
   NULL, /* exit thread */
   NULL, /* exit process */
   NULL, /* exit master */
   NGX_MODULE_V1_PADDING
};

static ngx_int_t ngx_epoll_init(ngx_cycle_t *cycle, ngx_msec_t timer)
{
   ngx_epoll_conf_t *epcf;
   epcf = ngx_event_get_conf(cycle->conf_ctx, ngx_epoll_module);
   return NGX_OK;
}

static void ngx_epoll_done(ngx_cycle_t *cycle)
{
}

static ngx_int_t ngx_epoll_add_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags)
{
   return NGX_OK;
}

static ngx_int_t ngx_epoll_del_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags)
{
   return NGX_OK;
}

static ngx_int_t ngx_epoll_add_connection(ngx_connection_t *c)
{
   return NGX_OK;
}

static ngx_int_t ngx_epoll_del_connection(ngx_connection_t *c, ngx_uint_t flags)
{
   return NGX_OK;
}

static ngx_int_t ngx_epoll_process_events(ngx_cycle_t *cycle, ngx_msec_t timer, ngx_uint_t flags)
{
   return NGX_OK;
}

static void *
ngx_epoll_create_conf(ngx_cycle_t *cycle)
{
   ngx_epoll_conf_t *epcf;

   epcf = ngx_palloc(cycle->pool, sizeof(ngx_epoll_conf_t));
   if (epcf == NULL) {
      return NULL;
   }

   epcf->events = NGX_CONF_UNSET;
   epcf->aio_requests = NGX_CONF_UNSET;

   return epcf;
}

static char *
ngx_epoll_init_conf(ngx_cycle_t *cycle, void *conf)
{
   ngx_epoll_conf_t *epcf = conf;

   ngx_conf_init_uint_value(epcf->events, 512);
   ngx_conf_init_uint_value(epcf->aio_requests, 32);

   return NGX_CONF_OK;
}
