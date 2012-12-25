#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_array)
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

H2CASE(ngx_array,"array create")
{
	ngx_array_t * array = ngx_array_create(pool, 10, 40);
	H2EQUAL_STRCMP("ngx_array_t{size=40,nelts/alloc=0/10}", jeff_array_tustring(array, NULL));
	ngx_array_destroy(array);
}
