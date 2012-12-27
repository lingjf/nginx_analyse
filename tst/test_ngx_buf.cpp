#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_buf)
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

H2CASE(ngx_buf,"create temp buf")
{
   ngx_buf_t *buf = ngx_create_temp_buf(pool, 68);
	H2EQUAL_STRCMP("ngx_buf_t{end-start=68,last-pos=0,temporary}", jeff_buf_tustring(buf));
}

H2CASE(ngx_buf,"alloc chain link")
{
   ngx_chain_t *chain = ngx_alloc_chain_link(pool);
   //chain is not initialized
	//H2EQUAL_STRCMP("", jeff_chain_tustring(chain));
   ngx_free_chain(pool, chain);
   H2EQUAL_STRCMP("ngx_pool_t[max=88,small=1,large=0,cleanup=0,chain=1]", jeff_pool_tustring(pool));
}

H2CASE(ngx_buf,"create chain of bufs")
{
   ngx_bufs_t bufs = {2, 64};
   ngx_chain_t *chain = ngx_create_chain_of_bufs(pool, &bufs);
	H2EQUAL_STRCMP("ngx_chain_t{ngx_buf_t{end-start=64,last-pos=0,temporary},ngx_buf_t{end-start=64,last-pos=0,temporary},}", jeff_chain_tustring(chain));
}
