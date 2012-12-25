#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}

H2UNIT(ngx_inet)
{
   void setup()
   {
   }

   void teardown()
   {
   }
};

H2CASE(ngx_inet,"inet_addr")
{
   u_char t[] = "1.2.3.4";
   H2EQUAL_INTEGER(0x04030201, ngx_inet_addr(t, sizeof(t)-1));
}

H2CASE_IGNORE(ngx_inet,"sock_ntop")
{
   u_char t[64];
   struct sockaddr_in sa;
   sa.sin_port = 80;
   sa.sin_addr.s_addr = 0x04030201;
   int sz = ngx_sock_ntop((struct sockaddr*)&sa, t, 64, 1);
   H2EQUAL_INTEGER(0, sz);
   H2EQUAL_STRCMP("", t);
}
