#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_slab)
{
	void setup()
	{
	}

	void teardown()
	{
	}
};

H2CASE(ngx_slab,"slab init")
{
	ngx_slab_pool_t slab;
	//ngx_slab_init(&slab);
	H2EQ_STRCMP("", jeff_slab_tustring(&slab));
}

H2CASE(ngx_slab,"slab alloc")
{
	ngx_slab_pool_t slab;
	//ngx_slab_init(&slab);
	//void *data1 = ngx_slab_alloc(&slab, 100);
	//H2CHECK(data1 != NULL);
	H2EQ_STRCMP("", jeff_slab_tustring(&slab));
}

