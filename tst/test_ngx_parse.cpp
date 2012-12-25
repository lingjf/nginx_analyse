#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


H2UNIT(ngx_parse_size)
{
	void setup()
	{
	}

	void teardown()
	{
	}
};

H2CASE(ngx_parse_size,"size=0")
{
   ngx_str_t s = ngx_string("0");
   H2EQUAL_INTEGER(0, ngx_parse_size(&s));
}

H2CASE(ngx_parse_size,"size=1")
{
   ngx_str_t s = ngx_string("1");
   H2EQUAL_INTEGER(1, ngx_parse_size(&s));
}

H2CASE(ngx_parse_size,"size=123456")
{
   ngx_str_t s = ngx_string("123456");
   H2EQUAL_INTEGER(123456, ngx_parse_size(&s));
}

H2CASE(ngx_parse_size,"size=4k")
{
   ngx_str_t s = ngx_string("4k");
   H2EQUAL_INTEGER(4 * 1024, ngx_parse_size(&s));
}

H2CASE(ngx_parse_size,"size=8M")
{
   ngx_str_t s = ngx_string("8M");
   H2EQUAL_INTEGER(8 * 1024 * 1024, ngx_parse_size(&s));
}


H2UNIT(ngx_parse_offset)
{
   void setup()
   {
   }

   void teardown()
   {
   }
};

H2CASE(ngx_parse_offset,"offset=0")
{
   ngx_str_t s = ngx_string("0");
   H2EQUAL_INTEGER(0, ngx_parse_offset(&s));
}

H2CASE(ngx_parse_offset,"offset=2k")
{
   ngx_str_t s = ngx_string("2k");
   H2EQUAL_INTEGER(2 * 1024, ngx_parse_offset(&s));
}

H2CASE(ngx_parse_offset,"offset=4M")
{
   ngx_str_t s = ngx_string("4M");
   H2EQUAL_INTEGER(4 * 1024 * 1024, ngx_parse_offset(&s));
}

H2CASE(ngx_parse_offset,"offset=1G")
{
   ngx_str_t s = ngx_string("1G");
   H2EQUAL_INTEGER(1 * 1024 * 1024 * 1024, ngx_parse_offset(&s));
}


H2UNIT(ngx_parse_time)
{
   void setup()
   {
   }

   void teardown()
   {
   }
};

H2CASE(ngx_parse_time,"time=0s")
{
   ngx_str_t s = ngx_string("0");
   H2EQUAL_INTEGER(0, ngx_parse_time(&s, 1));
}
