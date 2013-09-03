#include "h2unit.h"
#include "test_stub.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_cycle)
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

H2CASE(ngx_cycle,"init")
{
   ngx_cycle_t * c = setup_nginx_runtime();
   H2EQ_WILDCARD("ngx_cycle_t{prefix=*/nginx_analyse/objs/,conf_prefix=*/nginx_analyse/objs/conf/,conf_file=*/nginx_analyse/objs/conf/nginx.conf}", jeff_cycle_tustring(c));
}
