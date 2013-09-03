#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_output_chain)
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

H2CASE(ngx_output_chain,"update chains")
{
   ngx_output_chain_ctx_t occ;
   memset(&occ, 0, sizeof(occ));

   ngx_bufs_t bufs_in = {3, 64};
   occ.in = ngx_create_chain_of_bufs(pool, &bufs_in);
   ngx_buf_t * buf_in_1 = occ.in->buf;
   ngx_buf_t * buf_in_2 = occ.in->next->buf;
   ngx_buf_t * buf_in_3 = occ.in->next->next->buf;
   buf_in_1->pos += 64;
   buf_in_1->last += 64; // buf_in_1 handled finished
   buf_in_2->pos += 4;
   buf_in_2->last += 64; // buf_in_2 has 60 bytes
   buf_in_3->last += 20; // buf_in_3 has 20 bytes

   H2EQ_STRCMP("ngx_chain_t{ngx_buf_t{pos/last/end=64/64/64,temporary},ngx_buf_t{pos/last/end=4/64/64,temporary},ngx_buf_t{pos/last/end=0/20/64,temporary},}", jeff_chain_tustring(occ.in));
   H2EQ_STRCMP("NULL", jeff_chain_tustring(occ.busy));
   H2EQ_STRCMP("NULL", jeff_chain_tustring(occ.free));

   ngx_chain_update_chains(pool, &occ.free, &occ.busy, &occ.in, 0);
   H2EQ_STRCMP("NULL", jeff_chain_tustring(occ.in));
   H2EQ_STRCMP("ngx_chain_t{ngx_buf_t{pos/last/end=4/64/64,temporary},ngx_buf_t{pos/last/end=0/20/64,temporary},}", jeff_chain_tustring(occ.busy));
   H2EQ_STRCMP("ngx_chain_t{ngx_buf_t{pos/last/end=0/0/64,temporary},}", jeff_chain_tustring(occ.free));
}
