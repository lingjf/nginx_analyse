
#ifndef TEST_STUB_H_
#define TEST_STUB_H_

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
}

ngx_cycle_t * setup_nginx_runtime();


#endif /* TEST_STUB_H_ */
