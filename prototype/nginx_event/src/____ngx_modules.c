
#include <____ngx_config.h>
#include <____ngx_core.h>



extern ngx_module_t  ngx_core_module;
extern ngx_module_t  ngx_events_module;
extern ngx_module_t  ngx_event_core_module;
extern ngx_module_t  ngx_epoll_module;
extern ngx_module_t  ngx_select_module;
extern ngx_module_t  ngx_http_test_module;

ngx_module_t *ngx_modules[] = {
    &ngx_core_module,
    &ngx_events_module,
    &ngx_event_core_module,
    &ngx_epoll_module,
    &ngx_select_module,
    &ngx_http_test_module,
    NULL
};

