
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_REQUEST_H_INCLUDED_
#define _NGX_HTTP_REQUEST_H_INCLUDED_



typedef struct ngx_http_server_name_s  ngx_http_server_name_t;


typedef struct {
     ngx_hash_combined_t              names;

     ngx_uint_t                       nregex;
     ngx_http_server_name_t          *regex;
} ngx_http_virtual_names_t;




#endif /* _NGX_HTTP_REQUEST_H_INCLUDED_ */
