#include "h2unit.h"
#include "test_stub.h"


H2UNIT(ngx_http_variables)
{
	ngx_pool_t * pool;
	void setup()
	{
		pool = ngx_create_pool(128, NULL);
	}

	void teardown()
	{
		ngx_destroy_pool(pool);
	}
};

H2CASE(ngx_http_variables,"add core vars")
{
   ngx_cycle_t * c = setup_nginx_runtime();
   ngx_conf_t cf;
   cf.cycle = c;
   cf.ctx = c->conf_ctx[ngx_http_module.index];
   cf.pool = c->pool;
   cf.temp_pool = c->pool;
   ngx_int_t rv = ngx_http_variables_add_core_vars(&cf);
   H2EQUAL_INTEGER(NGX_OK, rv);
   ngx_http_core_main_conf_t  *cmcf = (ngx_http_core_main_conf_t*)ngx_http_cycle_get_module_main_conf(c, ngx_http_core_module);
   //H2EQUAL_STRCMP("", jeff_hash_keys_arrays_tustring(cmcf->variables_keys, 0));
   //printf("%s\n", jeff_array_tustring(&cmcf->variables, (u_char*(*)(void*))jeff_http_variable_tustring));
}

