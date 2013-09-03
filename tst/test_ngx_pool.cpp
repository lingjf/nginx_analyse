#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_pool)
{

	void setup()
	{
	}

	void teardown()
	{
	}
};

H2CASE(ngx_pool,"pool create")
{
	ngx_pool_t * pool;
	pool = ngx_create_pool(128, NULL);
	H2EQ_STRCMP("ngx_pool_t[max=88,small=1,large=0,cleanup=0,chain=0]", jeff_pool_tustring(pool));
	ngx_destroy_pool(pool);
}

H2CASE(ngx_pool,"pool palloc with small")
{
	ngx_pool_t * pool;
	pool = ngx_create_pool(128, NULL);
	void * data1 = ngx_palloc(pool, 60);
	H2EQ_TRUE(data1 != NULL);
	void * data2 = ngx_palloc(pool, 60);
	H2EQ_TRUE(data2 != NULL);
	H2EQ_STRCMP("ngx_pool_t[max=88,small=2,large=0,cleanup=0,chain=0]", jeff_pool_tustring(pool));
	ngx_destroy_pool(pool);
}

H2CASE(ngx_pool,"pool palloc with large")
{
	ngx_pool_t * pool;
	pool = ngx_create_pool(128, NULL);
	void * data1 = ngx_palloc(pool, 100);
	H2EQ_TRUE(data1 != NULL);
	void * data2 = ngx_palloc(pool, 100);
	H2EQ_TRUE(data2 != NULL);
	H2EQ_STRCMP("ngx_pool_t[max=88,small=1,large=2,cleanup=0,chain=0]", jeff_pool_tustring(pool));
	ngx_destroy_pool(pool);
}
