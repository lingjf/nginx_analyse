#include "h2unit.h"
#include "test_stub.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_conf_file)
{
	ngx_pool_t * pool;
	void setup()
	{
		pool = ngx_create_pool(4000, NULL);
	}

	void teardown()
	{
		ngx_destroy_pool(pool);
	}
};

H2CASE(ngx_conf_file,"set flag slot")
{
   char *rv;
   ngx_str_t * s;
   ngx_flag_t result;
   ngx_conf_t cf = {0};
   ngx_command_t cmd = {0};

   // on
   cf.args = ngx_array_create(pool, 3, sizeof(ngx_str_t));
   s = (ngx_str_t *)ngx_array_push(cf.args);
   ngx_str_set(s, "daemon");
   s = (ngx_str_t *)ngx_array_push(cf.args);
   ngx_str_set(s, "on");

   result = NGX_CONF_UNSET;
   rv = ngx_conf_set_flag_slot(&cf, &cmd, (void*)&result);
   H2EQUAL_INTEGER(NGX_CONF_OK, rv);
   H2EQUAL_INTEGER(1, result);

   // off
   cf.args = ngx_array_create(pool, 3, sizeof(ngx_str_t));
   s = (ngx_str_t *)ngx_array_push(cf.args);
   ngx_str_set(s, "daemon");
   s = (ngx_str_t *)ngx_array_push(cf.args);
   ngx_str_set(s, "off");

   result = NGX_CONF_UNSET;
   rv = ngx_conf_set_flag_slot(&cf, &cmd, (void*)&result);
   H2EQUAL_INTEGER(NGX_CONF_OK, rv);
   H2EQUAL_INTEGER(0, result);
}

H2CASE(ngx_conf_file,"set str slot")
{
   char *rv;
   ngx_str_t * s;
   ngx_str_t result = {0, NULL};
   ngx_conf_t cf = {0};
   ngx_command_t cmd = {0};

   cf.args = ngx_array_create(pool, 3, sizeof(ngx_str_t));
   s = (ngx_str_t *)ngx_array_push(cf.args);
   ngx_str_set(s, "hello");
   s = (ngx_str_t *)ngx_array_push(cf.args);
   ngx_str_set(s, "world");

   rv = ngx_conf_set_str_slot(&cf, &cmd, (void*)&result);
   H2EQUAL_INTEGER(NGX_CONF_OK, rv);
   H2EQUAL_STRCMP("world", jeff_str_tustring(&result));
}

H2CASE(ngx_conf_file,"set slot")
{
   //setup_nginx_runtime();
}
