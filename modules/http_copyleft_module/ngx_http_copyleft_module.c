

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_str_t                           copyleft;
} ngx_http_copyleft_loc_conf_t;


typedef struct {
    ngx_str_t                           copyleft;
} ngx_http_copyleft_ctx_t;

static char *ngx_http_copyleft(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_http_copyleft_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_copyleft_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t ngx_http_copyleft_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_copyleft_commands[] = {

    { ngx_string("copyleft"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_copyleft_loc_conf_t, copyleft),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_copyleft_module_ctx = {
    NULL,                                 /* proconfiguration */
    ngx_http_copyleft_init,               /* postconfiguration */

    NULL,                                 /* create main configuration */
    NULL,                                 /* init main configuration */

    NULL,                                 /* create server configuration */
    NULL,                                 /* merge server configuration */

    ngx_http_copyleft_create_loc_conf,    /* create location configuration */
    ngx_http_copyleft_merge_loc_conf      /* merge location configuration */
};


ngx_module_t  ngx_http_copyleft_module = {
    NGX_MODULE_V1,
    &ngx_http_copyleft_module_ctx,      /* module context */
    ngx_http_copyleft_commands,         /* module directives */
    NGX_HTTP_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    NULL,                               /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_output_header_filter_pt ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt   ngx_http_next_body_filter;


static ngx_int_t
ngx_http_copyleft_header_filter(ngx_http_request_t *r)
{
    ngx_http_copyleft_ctx_t       *ctx;
    ngx_http_copyleft_loc_conf_t  *lcf;

    lcf = ngx_http_get_module_loc_conf(r, ngx_http_copyleft_module);

    if (r->header_only || (r->method & NGX_HTTP_HEAD) || r != r->main || r->headers_out.status == NGX_HTTP_NO_CONTENT) {
        return ngx_http_next_header_filter(r);
    }

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_copyleft_ctx_t));
    if (ctx == NULL) {
       return NGX_ERROR;
    }

    if (!ngx_strncasecmp(r->headers_out.content_type.data, "text/html", ngx_min(sizeof("text/html"), r->headers_out.content_type.len))) {
       ctx->copyleft.len = sizeof("<!-- ") + lcf->copyleft.len + sizeof(" -->\n");
       ctx->copyleft.data = ngx_pcalloc(r->pool, ctx->copyleft.len + 1);
       ngx_sprintf(ctx->copyleft.data, "<!-- %V -->\n", &lcf->copyleft);

    } else if (!ngx_strncasecmp(r->headers_out.content_type.data, "text/css", ngx_min(sizeof("text/css"), r->headers_out.content_type.len))){
       ctx->copyleft.len = sizeof("/* ") + lcf->copyleft.len + sizeof(" */\n");
       ctx->copyleft.data = ngx_pcalloc(r->pool, ctx->copyleft.len + 1);
       ngx_sprintf(ctx->copyleft.data, "/* %V */\n", &lcf->copyleft);

    } else {
       return ngx_http_next_header_filter(r);
    }

    ngx_http_set_ctx(r, ctx, ngx_http_copyleft_module);

    if (r->headers_out.content_length_n != -1) {
        r->headers_out.content_length_n += ctx->copyleft.len;
    }

    if (r->headers_out.content_length) {
        r->headers_out.content_length->hash = 0;
        r->headers_out.content_length = NULL;
    }

    ngx_http_clear_accept_ranges(r);

    return ngx_http_next_header_filter(r);
}


static ngx_int_t
ngx_http_copyleft_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_buf_t             *buf;
    ngx_uint_t             last;
    ngx_chain_t           *cl, *nl;
    ngx_http_copyleft_ctx_t *ctx;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "http copyleft body filter");

    ctx = ngx_http_get_module_ctx(r, ngx_http_copyleft_module);
    if (ctx == NULL) {
        return ngx_http_next_body_filter(r, in);
    }

    last = 0;

    for (cl = in; cl; cl = cl->next) {
         if (cl->buf->last_buf) {
             last = 1;
             break;
         }
    }

    if (!last) {
        return ngx_http_next_body_filter(r, in);
    }

    buf = ngx_calloc_buf(r->pool);
    if (buf == NULL) {
        return NGX_ERROR;
    }

    buf->pos = ctx->copyleft.data;
    buf->last = buf->pos + ctx->copyleft.len;
    buf->start = buf->pos;
    buf->end = buf->last;
    buf->last_buf = 1;
    buf->memory = 1;

    if (ngx_buf_size(cl->buf) == 0) {
        cl->buf = buf;
    } else {
        nl = ngx_alloc_chain_link(r->pool);
        if (nl == NULL) {
            return NGX_ERROR;
        }

        nl->buf = buf;
        nl->next = NULL;
        cl->next = nl;
        cl->buf->last_buf = 0;
    }

    return ngx_http_next_body_filter(r, in);
}


static void *
ngx_http_copyleft_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_copyleft_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_copyleft_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}


static char *
ngx_http_copyleft_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_copyleft_loc_conf_t  *prev = parent;
    ngx_http_copyleft_loc_conf_t  *conf = child;

    ngx_conf_merge_str_value(conf->copyleft, prev->copyleft,  "");

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_copyleft_init(ngx_conf_t *cf)
{
    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_copyleft_body_filter;

    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_copyleft_header_filter;

    return NGX_OK;
}

