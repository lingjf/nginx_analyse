#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}


int gettimeofday_fake(struct timeval*tv, struct timezone*tz)
{
   tv->tv_sec = 1234567890;
   tv->tv_usec = 0;
   return 0;
}

H2UNIT(ngx_times)
{
	void setup()
	{
		H2STUB(gettimeofday, gettimeofday_fake);
		ngx_time_init();
	}

	void teardown()
	{
	}
};

H2CASE(ngx_times,"http time")
{
   H2EQUAL_INTEGER(1234567890, ngx_time());
   H2EQUAL_STRCMP("Fri, 13 Feb 2009 23:31:30 GMT", jeff_str_tustring((ngx_str_t*)&ngx_cached_http_time));
}
