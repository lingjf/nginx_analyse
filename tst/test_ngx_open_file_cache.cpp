#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_open_file_cache)
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

H2CASE(ngx_open_file_cache,"open file cache init")
{
   ngx_open_file_cache_t * ofc = ngx_open_file_cache_init(pool, 100, 8);
	H2EQUAL_STRCMP("ngx_open_file_cache_t[0/100,8]", jeff_open_file_cache_tustring(ofc));
}
