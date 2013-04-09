/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>

static ngx_int_t ngx_cmp_sockaddr(struct sockaddr *sa1, struct sockaddr *sa2);

volatile ngx_cycle_t *ngx_cycle;


ngx_cycle_t *
ngx_init_cycle(ngx_cycle_t *old_cycle)
{
   void *rv;
   char **senv, **env;
   ngx_uint_t i, n;
   ngx_log_t *log;
   ngx_pool_t *pool;
   ngx_cycle_t *cycle, **old;
   ngx_listening_t *ls, *nls;
   ngx_core_conf_t *ccf, *old_ccf;

   pool = ngx_create_pool(NGX_CYCLE_POOL_SIZE, log);
   if (pool == NULL) {
      return NULL;
   }

   cycle = ngx_pcalloc(pool, sizeof(ngx_cycle_t));
   if (cycle == NULL) {
      ngx_destroy_pool(pool);
      return NULL;
   }

   cycle->pool = pool;
   cycle->old_cycle = old_cycle;

   cycle->conf_prefix.len = old_cycle->conf_prefix.len;
   cycle->conf_prefix.data = ngx_pstrdup(pool, &old_cycle->conf_prefix);
   if (cycle->conf_prefix.data == NULL) {
      ngx_destroy_pool(pool);
      return NULL;
   }

   cycle->prefix.len = old_cycle->prefix.len;
   cycle->prefix.data = ngx_pstrdup(pool, &old_cycle->prefix);
   if (cycle->prefix.data == NULL) {
      ngx_destroy_pool(pool);
      return NULL;
   }

   cycle->conf_file.len = old_cycle->conf_file.len;
   cycle->conf_file.data = ngx_pnalloc(pool, old_cycle->conf_file.len + 1);
   if (cycle->conf_file.data == NULL) {
      ngx_destroy_pool(pool);
      return NULL;
   }
   ngx_cpystrn(cycle->conf_file.data, old_cycle->conf_file.data, old_cycle->conf_file.len + 1);

   n = old_cycle->listening.nelts ? old_cycle->listening.nelts : 10;

   cycle->listening.elts = ngx_pcalloc(pool, n * sizeof(ngx_listening_t));
   if (cycle->listening.elts == NULL) {
      ngx_destroy_pool(pool);
      return NULL;
   }

   cycle->listening.nelts = 0;
   cycle->listening.size = sizeof(ngx_listening_t);
   cycle->listening.nalloc = n;
   cycle->listening.pool = pool;

   senv = environ;

   {
      ccf = &cycle->core_conf;

      ccf->daemon = 0;
      ccf->worker_processes = 2;
      ngx_str_set(&ccf->pid, NGX_PID_PATH);
      ngx_str_set(&ccf->oldpid, NGX_OLDPID_PATH);
      ngx_array_init(&ccf->env, cycle->pool, 1, sizeof(ngx_str_t));
      {
         struct group *grp;
         struct passwd *pwd;
         ccf->username = NGX_USER;
         pwd = getpwnam(NGX_USER);
         if (pwd) {
            ccf->user = pwd->pw_uid;
         } else {
            ccf->user = 65534;
         }
         grp = getgrnam(NGX_GROUP);
         if (grp) {
            ccf->group = grp->gr_gid;
         } else {
            ccf->group = 65534;
         }
      }
   }

   if (ngx_process == NGX_PROCESS_SIGNALLER) {
      return cycle;
   }

   ccf = &cycle->core_conf;
   old_ccf = &old_cycle->core_conf;
   if (ccf->pid.len != old_ccf->pid.len || ngx_strcmp(ccf->pid.data, old_ccf->pid.data) != 0) {
      /* new pid file name */

      ngx_create_pidfile(&ccf->pid, log);
      ngx_delete_pidfile(old_cycle);
   }

   /* handle the listening sockets */

   if (old_cycle->listening.nelts) {
      ls = old_cycle->listening.elts;
      for (i = 0; i < old_cycle->listening.nelts; i++) {
         ls[i].remain = 0;
      }

      nls = cycle->listening.elts;
      for (n = 0; n < cycle->listening.nelts; n++) {

         for (i = 0; i < old_cycle->listening.nelts; i++) {
            if (ls[i].ignore) {
               continue;
            }

            if (ngx_cmp_sockaddr(nls[n].sockaddr, ls[i].sockaddr) == NGX_OK) {
               nls[n].fd = ls[i].fd;
               nls[n].previous = &ls[i];
               ls[i].remain = 1;

               if (ls[n].backlog != nls[i].backlog) {
                  nls[n].listen = 1;
               }

               if (ls[n].deferred_accept && !nls[n].deferred_accept) {
                  nls[n].delete_deferred = 1;

               } else if (ls[i].deferred_accept != nls[n].deferred_accept) {
                  nls[n].add_deferred = 1;
               }
               break;
            }
         }

         if (nls[n].fd == -1) {
            nls[n].open = 1;
         }
      }

   } else {
      ls = cycle->listening.elts;
      for (i = 0; i < cycle->listening.nelts; i++) {
         ls[i].open = 1;
         if (ls[i].deferred_accept) {
            ls[i].add_deferred = 1;
         }
      }
   }

   ngx_open_listening_sockets(cycle);

   ngx_configure_listening_sockets(cycle);

   /* close the unnecessary listening sockets */

   ls = old_cycle->listening.elts;
   for (i = 0; i < old_cycle->listening.nelts; i++) {
      if (ls[i].remain || ls[i].fd == -1) {
         continue;
      }
      close(ls[i].fd);
   }

   env = environ;
   environ = senv;

   ngx_destroy_pool(old_cycle->pool);
   cycle->old_cycle = NULL;

   environ = env;

   return cycle;
}

static ngx_int_t ngx_cmp_sockaddr(struct sockaddr *sa1, struct sockaddr *sa2)
{
   struct sockaddr_in *sin1, *sin2;
   struct sockaddr_un *saun1, *saun2;

   if (sa1->sa_family != sa2->sa_family) {
      return NGX_DECLINED;
   }

   switch (sa1->sa_family) {

   case AF_UNIX:
      saun1 = (struct sockaddr_un *) sa1;
      saun2 = (struct sockaddr_un *) sa2;

      if (ngx_memcmp(&saun1->sun_path, &saun2->sun_path, sizeof(saun1->sun_path)) != 0) {
         return NGX_DECLINED;
      }

      break;

   default: /* AF_INET */

      sin1 = (struct sockaddr_in *) sa1;
      sin2 = (struct sockaddr_in *) sa2;

      if (sin1->sin_port != sin2->sin_port) {
         return NGX_DECLINED;
      }

      if (sin1->sin_addr.s_addr != sin2->sin_addr.s_addr) {
         return NGX_DECLINED;
      }

      break;
   }

   return NGX_OK;
}

ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log)
{
   size_t len;
   ngx_fd_t file;
   u_char pid[NGX_INT64_LEN + 2];

   if (ngx_process > NGX_PROCESS_MASTER) {
      return NGX_OK;
   }

   len = ngx_snprintf(pid, NGX_INT64_LEN + 2, "%P%N", ngx_pid) - pid;
   file = open(name->data, O_RDWR | O_CREAT | O_TRUNC, 0644);
   write(file, pid, len);
   close(file);

   return NGX_OK;
}

void ngx_delete_pidfile(ngx_cycle_t *cycle)
{
   u_char *name;

   name = ngx_new_binary ? cycle->core_conf.oldpid.data : cycle->core_conf.pid.data;

   unlink(name);
}

ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig)
{
   ssize_t n;
   ngx_int_t pid;
   ngx_fd_t file;
   u_char buf[NGX_INT64_LEN + 2];

   file = open(cycle->core_conf.pid.data, O_RDONLY | 0, 0644);

   n = read(file, buf, NGX_INT64_LEN + 2);

   close(file);

   while (n-- && (buf[n] == CR || buf[n] == LF )) { /* void */
   }

   pid = ngx_atoi(buf, ++n);

   if (pid == NGX_ERROR) {
      return 1;
   }

   return ngx_os_signal_process(cycle, sig, pid);

}

