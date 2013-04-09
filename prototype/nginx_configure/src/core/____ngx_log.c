
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <____ngx_config.h>
#include <____ngx_core.h>


static char *ngx_error_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t  ngx_errlog_commands[] = {

    {ngx_string("error_log"),
     NGX_MAIN_CONF|NGX_CONF_1MORE,
     ngx_error_log,
     0,
     0,
     NULL},

    ngx_null_command
};


static ngx_core_module_t  ngx_errlog_module_ctx = {
    ngx_string("errlog"),
    NULL,
    NULL
};


ngx_module_t  ngx_errlog_module = {
    NGX_MODULE_V1,
    &ngx_errlog_module_ctx,                /* module context */
    ngx_errlog_commands,                   /* module directives */
    NGX_CORE_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_log_t        ngx_log;

void
ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err, const char *fmt, ...)
{
}



void ngx_cdecl
ngx_log_abort(ngx_err_t err, const char *fmt, ...)
{
}


void ngx_cdecl
ngx_log_stderr(ngx_err_t err, const char *fmt, ...)
{
}


u_char *
ngx_log_errno(u_char *buf, u_char *last, ngx_err_t err)
{
    return "";
}


ngx_log_t *
ngx_log_init(u_char *prefix)
{
    return &ngx_log;
}


ngx_log_t *
ngx_log_create(ngx_cycle_t *cycle, ngx_str_t *name)
{
    return &ngx_log;
}


static char *
ngx_error_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t  *value, name;

    if (cf->cycle->new_log.file) {
        return "is duplicate";
    }

    value = cf->args->elts;

    if (ngx_strcmp(value[1].data, "stderr") == 0) {
        ngx_str_null(&name);

    } else {
        name = value[1];
    }

    cf->cycle->new_log.file = ngx_conf_open_file(cf->cycle, &name);
    if (cf->cycle->new_log.file == NULL) {
        return NULL;
    }

    if (cf->args->nelts == 2) {
        cf->cycle->new_log.log_level = NGX_LOG_ERR;
        return NGX_CONF_OK;
    }

    cf->cycle->new_log.log_level = 0;

    return NGX_CONF_OK;
}

