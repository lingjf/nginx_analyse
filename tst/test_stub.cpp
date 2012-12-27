#include "h2unit.h"
#include "test_stub.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>

ngx_int_t ngx_process_options(ngx_cycle_t *cycle);
ngx_int_t ngx_save_argv(ngx_cycle_t *cycle, int argc, char * const *argv);
}

ngx_int_t ngx_open_listening_sockets_fake(ngx_cycle_t *cycle)
{
   return NGX_OK;
}
void ngx_configure_listening_sockets_fake(ngx_cycle_t *cycle)
{
   return;
}
void ngx_close_listening_sockets_fake(ngx_cycle_t *cycle)
{
   return;
}

ngx_cycle_t * setup_nginx_runtime()
{
   H2STUB(ngx_open_listening_sockets, ngx_open_listening_sockets_fake);
   H2STUB(ngx_configure_listening_sockets, ngx_configure_listening_sockets_fake);
   H2STUB(ngx_close_listening_sockets, ngx_close_listening_sockets_fake);

   char *argv[] = { (char*) "nginx" };
   int argc = sizeof(argv) / sizeof(argv[0]);

   ngx_log_t *log;
   ngx_cycle_t *cycle, init_cycle;
   ngx_core_conf_t *ccf;

   ngx_debug_init();

   if (ngx_strerror_init() != NGX_OK) {
      return NULL;
   }

   /* TODO */ngx_max_sockets = -1;

   ngx_time_init();

   ngx_pid = ngx_getpid();

   log = ngx_log_init(NULL);
   if (log == NULL) {
      return NULL;
   }

   ngx_memzero(&init_cycle, sizeof(ngx_cycle_t));
   init_cycle.log = log;
   ngx_cycle = &init_cycle;

   init_cycle.pool = ngx_create_pool(1024, log);
   if (init_cycle.pool == NULL) {
      return NULL;
   }

   if (ngx_save_argv(&init_cycle, argc, argv) != NGX_OK) {
      return NULL;
   }

   if (ngx_process_options(&init_cycle) != NGX_OK) {
      return NULL;
   }

   if (ngx_os_init(log) != NGX_OK) {
      return NULL;
   }

   if (ngx_crc32_table_init() != NGX_OK) {
      return NULL;
   }

   ngx_max_module = 0;
   for (int i = 0; ngx_modules[i]; i++) {
      ngx_modules[i]->index = ngx_max_module++;
   }

   cycle = ngx_init_cycle(&init_cycle);
   if (cycle == NULL) {
      return NULL;
   }

   return cycle;
}
