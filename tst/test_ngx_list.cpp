#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_list)
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

H2CASE(ngx_list,"list create")
{
	ngx_list_t * list = ngx_list_create(pool, 10, 40);
	H2EQUAL_STRCMP("ngx_list_t[40,1/0/10]", jeff_list_tustring(list, NULL));
}


