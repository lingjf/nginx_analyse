#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_log)
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

H2CASE(ngx_log,"init")
{
   ngx_log_t *log = ngx_log_init(NULL);
	H2EQUAL_STRCMP("ngx_log_t{file=}", jeff_log_tustring(log));
}


