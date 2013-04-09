/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#ifndef _NGX_HTTP_H_INCLUDED_
#define _NGX_HTTP_H_INCLUDED_

#include <____ngx_config.h>
#include <____ngx_core.h>

#include <ngx_http_config.h>
#include <ngx_http_request.h>
#include <ngx_http_core_module.h>

typedef struct
{
   ngx_uint_t http_version;
   ngx_uint_t code;
   ngx_uint_t count;
   u_char *start;
   u_char *end;
} ngx_http_status_t;

#define ngx_http_get_module_ctx(r, module)  (r)->ctx[module.ctx_index]
#define ngx_http_set_ctx(r, c, module)      r->ctx[module.ctx_index] = c;

ngx_int_t ngx_http_add_location(ngx_conf_t *cf, ngx_queue_t **locations, ngx_http_core_loc_conf_t *clcf);
ngx_int_t ngx_http_add_listen(ngx_conf_t *cf, ngx_http_core_srv_conf_t *cscf, ngx_http_listen_opt_t *lsopt);


#define NGX_HTTP_LAST   1
#define NGX_HTTP_FLUSH  2

time_t ngx_http_parse_time(u_char *value, size_t len);
size_t ngx_http_get_time(char *buf, time_t t);

char *ngx_http_types_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_merge_types(ngx_conf_t *cf, ngx_array_t **keys, ngx_hash_t *types_hash, ngx_array_t **prev_keys, ngx_hash_t *prev_types_hash, ngx_str_t *default_types);
ngx_int_t ngx_http_set_default_types(ngx_conf_t *cf, ngx_array_t **types, ngx_str_t *default_type);

extern ngx_module_t ngx_http_module;

extern ngx_str_t ngx_http_html_default_types[];

#endif /* _NGX_HTTP_H_INCLUDED_ */
