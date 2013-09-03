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
	H2EQ_STRCMP("ngx_buf_t{pos/last/end=0/0/68,temporary}", jeff_buf_tustring(buf));
	H2EQ_MATH(0, ngx_buf_size(buf));
}

H2UNIT(ngx_chain)
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

H2CASE(ngx_chain,"alloc chain link")
{
   ngx_chain_t *chain = ngx_alloc_chain_link(pool);
   //chain is not initialized
	//H2EQ_STRCMP("", jeff_chain_tustring(chain));
   ngx_free_chain(pool, chain);
   H2EQ_STRCMP("ngx_pool_t[max=88,small=1,large=0,cleanup=0,chain=1]", jeff_pool_tustring(pool));
}

H2CASE(ngx_chain,"create chain of bufs")
{
   ngx_bufs_t bufs = {2, 64};
   ngx_chain_t *chain = ngx_create_chain_of_bufs(pool, &bufs);
	H2EQ_STRCMP("ngx_chain_t{ngx_buf_t{pos/last/end=0/0/64,temporary},ngx_buf_t{pos/last/end=0/0/64,temporary},}", jeff_chain_tustring(chain));
}

H2CASE(ngx_chain,"add copy chain")
{
   ngx_chain_t *chain_0 = NULL;

   ngx_bufs_t bufs_1 = {2, 64};
   ngx_chain_t *chain_1 = ngx_create_chain_of_bufs(pool, &bufs_1);
   H2EQ_STRCMP("ngx_chain_t{ngx_buf_t{pos/last/end=0/0/64,temporary},ngx_buf_t{pos/last/end=0/0/64,temporary},}", jeff_chain_tustring(chain_1));

   ngx_bufs_t bufs_2 = {2, 32};
   ngx_chain_t *chain_2 = ngx_create_chain_of_bufs(pool, &bufs_2);
   H2EQ_STRCMP("ngx_chain_t{ngx_buf_t{pos/last/end=0/0/32,temporary},ngx_buf_t{pos/last/end=0/0/32,temporary},}", jeff_chain_tustring(chain_2));

   H2EQ_MATH(NGX_OK, ngx_chain_add_copy(pool, &chain_0, chain_1));
   H2EQ_STRCMP("ngx_chain_t{ngx_buf_t{pos/last/end=0/0/64,temporary},ngx_buf_t{pos/last/end=0/0/64,temporary},}", jeff_chain_tustring(chain_0));

   H2EQ_MATH(NGX_OK, ngx_chain_add_copy(pool, &chain_0, chain_2));
   H2EQ_STRCMP("ngx_chain_t{ngx_buf_t{pos/last/end=0/0/64,temporary},ngx_buf_t{pos/last/end=0/0/64,temporary},ngx_buf_t{pos/last/end=0/0/32,temporary},ngx_buf_t{pos/last/end=0/0/32,temporary},}", jeff_chain_tustring(chain_0));
}


