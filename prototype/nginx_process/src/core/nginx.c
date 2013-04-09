/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>
#include <nginx.h>

static ngx_int_t ngx_add_inherited_sockets(ngx_cycle_t *cycle);
static ngx_int_t ngx_get_options(int argc, char * const *argv);
ngx_int_t ngx_process_options(ngx_cycle_t *cycle);
ngx_int_t ngx_save_argv(ngx_cycle_t *cycle, int argc, char * const *argv);

static char *ngx_signal;

static char **ngx_os_environ;

int ngx_cdecl main(int argc, char * const *argv)
{
   ngx_int_t i;
   ngx_log_t *log;
   ngx_cycle_t *cycle, init_cycle;
   ngx_core_conf_t *ccf;

   if (ngx_get_options(argc, argv) != NGX_OK) {
      return 1;
   }

   ngx_pid = ngx_getpid();

   ngx_memzero(&init_cycle, sizeof(ngx_cycle_t));
   init_cycle.log = log;
   ngx_cycle = &init_cycle;

   init_cycle.pool = ngx_create_pool(1024, log);
   if (init_cycle.pool == NULL) {
      return 1;
   }

   if (ngx_save_argv(&init_cycle, argc, argv) != NGX_OK) {
      return 1;
   }

   if (ngx_process_options(&init_cycle) != NGX_OK) {
      return 1;
   }

   if (ngx_os_init(log) != NGX_OK) {
      return 1;
   }

   if (ngx_add_inherited_sockets(&init_cycle) != NGX_OK) {
      return 1;
   }

   cycle = ngx_init_cycle(&init_cycle);
   if (cycle == NULL) {
      return 1;
   }

   if (ngx_signal) {
      return ngx_signal_process(cycle, ngx_signal);
   }

   ngx_cycle = cycle;

   ccf = &cycle->core_conf;

   ngx_process = NGX_PROCESS_MASTER;

   if (ngx_init_signals(cycle->log) != NGX_OK) {
      return 1;
   }

   if (ccf->daemon) {
      if (ngx_daemon(cycle->log) != NGX_OK) {
         return 1;
      }
   }

   if (ngx_create_pidfile(&ccf->pid, cycle->log) != NGX_OK) {
      return 1;
   }

   ngx_master_process_cycle(cycle);

   return 0;
}

static ngx_int_t ngx_add_inherited_sockets(ngx_cycle_t *cycle)
{
   u_char *p, *v, *inherited;
   ngx_int_t s;
   ngx_listening_t *ls;

   inherited = (u_char *) getenv(NGINX_VAR);
   if (inherited == NULL) {
      return NGX_OK;
   }

   printf("using inherited sockets from \"%s\"", inherited);

   if (ngx_array_init(&cycle->listening, cycle->pool, 10, sizeof(ngx_listening_t)) != NGX_OK) {
      return NGX_ERROR;
   }

   for (p = inherited, v = p; *p; p++) {
      if (*p == ':' || *p == ';') {
         s = ngx_atoi(v, p - v);
         if (s == NGX_ERROR) {
            printf("invalid socket number \"%s\"", v);
            break;
         }

         v = p + 1;

         ls = ngx_array_push(&cycle->listening);
         if (ls == NULL) {
            return NGX_ERROR;
         }

         ngx_memzero(ls, sizeof(ngx_listening_t));

         ls->fd = (ngx_socket_t) s;
      }
   }

   return ngx_set_inherited_sockets(cycle);
}

char **
ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last)
{
   char **p, **env;
   ngx_str_t *var;
   ngx_uint_t i, n;
   ngx_core_conf_t *ccf;

   ccf = &cycle->core_conf;

   if (last == NULL && ccf->environment) {
      return ccf->environment;
   }

   var = ccf->env.elts;

   for (i = 0; i < ccf->env.nelts; i++) {
      if (ngx_strcmp(var[i].data, "TZ") == 0 || ngx_strncmp(var[i].data, "TZ=", 3) == 0) {
         goto tz_found;
      }
   }

   var = ngx_array_push(&ccf->env);
   if (var == NULL) {
      return NULL;
   }

   var->len = 2;
   var->data = (u_char *) "TZ";

   var = ccf->env.elts;

   tz_found:

   n = 0;

   for (i = 0; i < ccf->env.nelts; i++) {

      if (var[i].data[var[i].len] == '=') {
         n++;
         continue;
      }

      for (p = ngx_os_environ; *p; p++) {

         if (ngx_strncmp(*p, var[i].data, var[i].len) == 0 && (*p)[var[i].len] == '=') {
            n++;
            break;
         }
      }
   }

   if (last) {
      env = ngx_alloc((*last + n + 1) * sizeof(char *), cycle->log);
      *last = n;

   } else {
      env = ngx_palloc(cycle->pool, (n + 1) * sizeof(char *));
   }

   if (env == NULL) {
      return NULL;
   }

   n = 0;

   for (i = 0; i < ccf->env.nelts; i++) {
      if (var[i].data[var[i].len] == '=') {
         env[n++] = (char *) var[i].data;
         continue;
      }

      for (p = ngx_os_environ; *p; p++) {
         if (ngx_strncmp(*p, var[i].data, var[i].len) == 0 && (*p)[var[i].len] == '=') {
            env[n++] = *p;
            break;
         }
      }
   }

   env[n] = NULL;

   if (last == NULL) {
      ccf->environment = env;
      environ = env;
   }

   return env;
}

ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char * const *argv)
{
   char **env, *var;
   u_char *p;
   ngx_uint_t i, n;
   ngx_pid_t pid;
   ngx_exec_ctx_t ctx;
   ngx_core_conf_t *ccf;
   ngx_listening_t *ls;

   ngx_memzero(&ctx, sizeof(ngx_exec_ctx_t));

   ctx.path = argv[0];
   ctx.name = "new binary process";
   ctx.argv = argv;

   n = 2;
   env = ngx_set_environment(cycle, &n);
   if (env == NULL) {
      return NGX_INVALID_PID;
   }

   var = ngx_alloc(sizeof(NGINX_VAR) + cycle->listening.nelts * (NGX_INT32_LEN + 1) + 2, cycle->log);

   p = ngx_cpymem(var, NGINX_VAR "=", sizeof(NGINX_VAR));

   ls = cycle->listening.elts;
   for (i = 0; i < cycle->listening.nelts; i++) {
      p = ngx_sprintf(p, "%ud;", ls[i].fd);
   }

   *p = '\0';

   env[n++] = var;

   /* allocate the spare 300 bytes for the new binary process title */

   env[n++] = "SPARE=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
               "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
               "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
               "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
               "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

   env[n] = NULL;

   ctx.envp = (char * const *) env;

   ccf = &cycle->core_conf;

   rename(ccf->pid.data, ccf->oldpid.data);

   pid = ngx_execute(cycle, &ctx);

   ngx_free(env);
   ngx_free(var);

   return pid;
}

static ngx_int_t ngx_get_options(int argc, char * const *argv)
{
   u_char *p;
   ngx_int_t i;

   for (i = 1; i < argc; i++) {

      p = (u_char *) argv[i];

      if (*p++ != '-') {
         return NGX_ERROR;
      }

      while (*p) {

         switch (*p++) {

         case 's':
            if (*p) {
               ngx_signal = (char *) p;

            } else if (argv[++i]) {
               ngx_signal = argv[i];

            } else {
               return NGX_ERROR;
            }

            if (ngx_strcmp(ngx_signal, "stop") == 0 || ngx_strcmp(ngx_signal, "quit") == 0 || ngx_strcmp(ngx_signal, "reopen") == 0 || ngx_strcmp(ngx_signal, "reload") == 0) {
               ngx_process = NGX_PROCESS_SIGNALLER;
               goto next;
            }

            return NGX_ERROR;

         default:
            return NGX_ERROR;
         }
      }

      next:

      continue;
   }

   return NGX_OK;
}

ngx_int_t ngx_save_argv(ngx_cycle_t *cycle, int argc, char * const *argv)
{
   size_t len;
   ngx_int_t i;

   ngx_os_argv = (char **) argv;
   ngx_argc = argc;

   ngx_argv = ngx_alloc((argc + 1) * sizeof(char *), cycle->log);
   if (ngx_argv == NULL) {
      return NGX_ERROR;
   }

   for (i = 0; i < argc; i++) {
      len = ngx_strlen(argv[i]) + 1;

      ngx_argv[i] = ngx_alloc(len, cycle->log);
      if (ngx_argv[i] == NULL) {
         return NGX_ERROR;
      }

      (void) ngx_cpystrn((u_char *) ngx_argv[i], (u_char *) argv[i], len);
   }

   ngx_argv[i] = NULL;

   ngx_os_environ = environ;

   return NGX_OK;
}

ngx_int_t ngx_process_options(ngx_cycle_t *cycle)
{
   ngx_str_set(&cycle->conf_prefix, NGX_CONF_PREFIX);
   ngx_str_set(&cycle->prefix, NGX_PREFIX);
   ngx_str_set(&cycle->conf_file, NGX_CONF_PATH);

   return NGX_OK;
}

uint64_t ngx_get_cpu_affinity(ngx_uint_t n)
{
   ngx_core_conf_t *ccf;

   ccf = &ngx_cycle->core_conf;

   if (ccf->cpu_affinity == NULL) {
      return 0;
   }

   if (ccf->cpu_affinity_n > n) {
      return ccf->cpu_affinity[n];
   }

   return ccf->cpu_affinity[ccf->cpu_affinity_n - 1];
}
