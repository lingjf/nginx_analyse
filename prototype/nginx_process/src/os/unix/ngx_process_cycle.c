/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>

static void ngx_start_worker_processes(ngx_cycle_t *cycle, ngx_int_t n, ngx_int_t type);
static void ngx_signal_worker_processes(ngx_cycle_t *cycle, int signo);
static ngx_uint_t ngx_reap_children(ngx_cycle_t *cycle);
static void ngx_master_process_exit(ngx_cycle_t *cycle);
static void ngx_worker_process_cycle(ngx_cycle_t *cycle, void *data);
static void ngx_worker_process_init(ngx_cycle_t *cycle, ngx_uint_t priority);
static void ngx_worker_process_exit(ngx_cycle_t *cycle);

ngx_uint_t ngx_process;
ngx_pid_t ngx_pid;

sig_atomic_t ngx_reap;
sig_atomic_t ngx_sigalrm;
sig_atomic_t ngx_terminate;
sig_atomic_t ngx_quit;
ngx_uint_t ngx_exiting;
sig_atomic_t ngx_reconfigure;

sig_atomic_t ngx_change_binary;
ngx_pid_t ngx_new_binary;

uint64_t cpu_affinity;
static u_char master_process[] = "master process";

static ngx_cycle_t ngx_exit_cycle;

void ngx_master_process_cycle(ngx_cycle_t *cycle)
{
   char *title;
   u_char *p;
   size_t size;
   ngx_int_t i;
   ngx_uint_t n, sigio;
   sigset_t set;
   struct itimerval itv;
   ngx_uint_t live;
   ngx_uint_t delay;
   ngx_listening_t *ls;
   ngx_core_conf_t *ccf;

   sigemptyset(&set);
   sigaddset(&set, SIGCHLD);
   sigaddset(&set, SIGALRM);
   sigaddset(&set, SIGIO);
   sigaddset(&set, SIGINT);
   sigaddset(&set, ngx_signal_value(NGX_RECONFIGURE_SIGNAL));
   sigaddset(&set, ngx_signal_value(NGX_REOPEN_SIGNAL));
   sigaddset(&set, ngx_signal_value(NGX_NOACCEPT_SIGNAL));
   sigaddset(&set, ngx_signal_value(NGX_TERMINATE_SIGNAL));
   sigaddset(&set, ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
   sigaddset(&set, ngx_signal_value(NGX_CHANGEBIN_SIGNAL));

   if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
      printf("sigprocmask() failed");
   }

   sigemptyset(&set);

   size = sizeof(master_process);

   for (i = 0; i < ngx_argc; i++) {
      size += ngx_strlen(ngx_argv[i]) + 1;
   }

   title = ngx_pnalloc(cycle->pool, size);

   p = ngx_cpymem(title, master_process, sizeof(master_process) - 1);
   for (i = 0; i < ngx_argc; i++) {
      *p++ = ' ';
      p = ngx_cpystrn(p, (u_char *) ngx_argv[i], size);
   }

   ngx_setproctitle(title);

   ccf = &cycle->core_conf;

   ngx_start_worker_processes(cycle, ccf->worker_processes, NGX_PROCESS_RESPAWN);

   ngx_new_binary = 0;
   delay = 0;
   sigio = 0;
   live = 1;

   for (;;) {
      printf("master cycle %d\n", ngx_pid);
      if (delay) {
         if (ngx_sigalrm) {
            sigio = 0;
            delay *= 2;
            ngx_sigalrm = 0;
         }

         printf("termination cycle: %d\n", delay);

         itv.it_interval.tv_sec = 0;
         itv.it_interval.tv_usec = 0;
         itv.it_value.tv_sec = delay / 1000;
         itv.it_value.tv_usec = (delay % 1000) * 1000;

         if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
            printf("setitimer() failed\n");
         }
      }

      sigsuspend(&set); /* now set is empty , unblock signals while waiting, block them while handling */

      printf("master wake up, sigio %i\n", sigio);

      if (ngx_reap) {
         ngx_reap = 0;
         printf("reap children\n");
         live = ngx_reap_children(cycle);
      }

      if (!live && (ngx_terminate || ngx_quit)) {
         ngx_master_process_exit(cycle);
      }

      if (ngx_terminate) {
         if (delay == 0) {
            delay = 50;
         }

         if (sigio) {
            sigio--;
            continue;
         }

         sigio = ccf->worker_processes;

         if (delay > 1000) {
            ngx_signal_worker_processes(cycle, SIGKILL);
         } else {
            ngx_signal_worker_processes(cycle, ngx_signal_value(NGX_TERMINATE_SIGNAL));
         }

         continue;
      }

      if (ngx_quit) {
         ngx_signal_worker_processes(cycle, ngx_signal_value(NGX_SHUTDOWN_SIGNAL));

         ls = cycle->listening.elts;
         for (n = 0; n < cycle->listening.nelts; n++) {
            close(ls[n].fd);
         }
         cycle->listening.nelts = 0;

         continue;
      }

      if (ngx_reconfigure) {
         ngx_reconfigure = 0;

         if (ngx_new_binary) {
            ngx_start_worker_processes(cycle, ccf->worker_processes, NGX_PROCESS_RESPAWN);
            continue;
         }

         cycle = ngx_init_cycle(cycle);
         if (cycle == NULL) {
            cycle = (ngx_cycle_t *) ngx_cycle;
            continue;
         }

         ngx_cycle = cycle;
         ccf = &cycle->core_conf;
         ngx_start_worker_processes(cycle, ccf->worker_processes, NGX_PROCESS_JUST_RESPAWN);

         /* allow new processes to start */
         ngx_msleep(100);

         live = 1;
         ngx_signal_worker_processes(cycle, ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
      }

      if (ngx_change_binary) {
         ngx_change_binary = 0;
         ngx_new_binary = ngx_exec_new_binary(cycle, ngx_argv);
      }
   }
}

static void ngx_start_worker_processes(ngx_cycle_t *cycle, ngx_int_t n, ngx_int_t type)
{
   ngx_int_t i;

   for (i = 0; i < n; i++) {
      cpu_affinity = ngx_get_cpu_affinity(i);
      ngx_spawn_process(cycle, ngx_worker_process_cycle, NULL, "worker process", type);
   }
}

static void ngx_signal_worker_processes(ngx_cycle_t *cycle, int signo)
{
   ngx_int_t i;
   ngx_err_t err;

   for (i = 0; i < ngx_last_process; i++) {

      if (ngx_processes[i].detached || ngx_processes[i].pid == -1) {
         continue;
      }

      if (ngx_processes[i].just_spawn) { /* for couple workers */
         ngx_processes[i].just_spawn = 0;
         continue;
      }

      if (ngx_processes[i].exiting && signo == ngx_signal_value(NGX_SHUTDOWN_SIGNAL)) {
         continue;
      }

      if (kill(ngx_processes[i].pid, signo) == -1) {
         err = errno;
         printf("kill(%d, %d) failed\n", ngx_processes[i].pid, signo);

         if (err == NGX_ESRCH) {
            ngx_processes[i].exited = 1;
            ngx_processes[i].exiting = 0;
            ngx_reap = 1;
         }

         continue;
      }

      if (signo != ngx_signal_value(NGX_REOPEN_SIGNAL)) {
         ngx_processes[i].exiting = 1;
      }
   }
}

static ngx_uint_t ngx_reap_children(ngx_cycle_t *cycle)
{
   ngx_int_t i, n;
   ngx_uint_t live;
   ngx_core_conf_t *ccf;

   live = 0;
   for (i = 0; i < ngx_last_process; i++) {

      if (ngx_processes[i].pid == -1) {
         continue;
      }

      if (ngx_processes[i].exited) {
         if (ngx_processes[i].respawn && !ngx_processes[i].exiting && !ngx_terminate && !ngx_quit) {
            if (ngx_spawn_process(cycle, ngx_processes[i].proc, ngx_processes[i].data, ngx_processes[i].name, i) == NGX_INVALID_PID) {
               printf("could not respawn %s\n", ngx_processes[i].name);
               continue;
            }
            live = 1;
            continue;
         }

         if (ngx_processes[i].pid == ngx_new_binary) { /* upgrade failed */

            ccf = &cycle->core_conf;

            rename((char *) ccf->oldpid.data, (char *) ccf->pid.data);

            ngx_new_binary = 0;
         }

         if (i == ngx_last_process - 1) {
            ngx_last_process--;
         } else {
            ngx_processes[i].pid = -1;
         }

      } else if (ngx_processes[i].exiting || !ngx_processes[i].detached) {
         live = 1;
      }
   }

   return live;
}

static void ngx_master_process_exit(ngx_cycle_t *cycle)
{
   ngx_uint_t i;

   ngx_delete_pidfile(cycle);

   printf("exit master process %d\n", ngx_pid);

   ngx_close_listening_sockets(cycle);

   ngx_cycle = &ngx_exit_cycle;

   ngx_destroy_pool(cycle->pool);

   exit(0);
}

static void ngx_worker_process_cycle(ngx_cycle_t *cycle, void *data)
{
   ngx_process = NGX_PROCESS_WORKER;

   ngx_worker_process_init(cycle, 1);

   ngx_setproctitle("worker process");

   for (;;) {
      printf("worker cycle %d\n", ngx_pid);

      if (ngx_exiting) {
         /*if no more pending events */
         ngx_worker_process_exit(cycle);
      }

      usleep(3000 * 1000);

      if (ngx_terminate) {
         ngx_worker_process_exit(cycle);
      }

      if (ngx_quit) {
         ngx_quit = 0;
         ngx_setproctitle("worker process is shutting down");
         if (!ngx_exiting) {
            ngx_close_listening_sockets(cycle);
            ngx_exiting = 1;
         }
      }
   }
}

static void ngx_worker_process_init(ngx_cycle_t *cycle, ngx_uint_t priority)
{
   sigset_t set;
   ngx_int_t n;
   ngx_uint_t i;
   struct rlimit rlmt;
   ngx_core_conf_t *ccf;
   ngx_listening_t *ls;

   if (ngx_set_environment(cycle, NULL) == NULL) {
      /* fatal */
      exit(2);
   }

   ccf = &cycle->core_conf;

   if (priority && ccf->priority != 0) {
      if (setpriority(PRIO_PROCESS, 0, ccf->priority) == -1) {
         printf("setpriority(%d) failed\n", ccf->priority);
      }
   }
   rlmt.rlim_cur = (rlim_t) ccf->rlimit_nofile;
   rlmt.rlim_max = (rlim_t) ccf->rlimit_nofile;

   if (setrlimit(RLIMIT_NOFILE, &rlmt) == -1) {
      printf("setrlimit(RLIMIT_NOFILE, %i) failed\n", ccf->rlimit_nofile);
   }

   rlmt.rlim_cur = (rlim_t) ccf->rlimit_core;
   rlmt.rlim_max = (rlim_t) ccf->rlimit_core;

   if (setrlimit(RLIMIT_CORE, &rlmt) == -1) {
      printf("setrlimit(RLIMIT_CORE, %lld) failed\n", ccf->rlimit_core);
   }

   if (geteuid() == 0) {
      if (setgid(ccf->group) == -1) {
         printf("setgid(%d) failed\n", ccf->group);
         /* fatal */
         exit(2);
      }

      if (initgroups(ccf->username, ccf->group) == -1) {
         printf("initgroups(%s, %d) failed\n", ccf->username, ccf->group);
      }

      if (setuid(ccf->user) == -1) {
         printf("setuid(%d) failed\n", ccf->user);
         /* fatal */
         exit(2);
      }
   }

   if (cpu_affinity) {
      ngx_setaffinity(cpu_affinity, cycle->log);
   }

   /* allow coredump after setuid() in Linux 2.4.x */

   if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) == -1) {
      printf("prctl(PR_SET_DUMPABLE) failed\n");
   }

   sigemptyset(&set);

   if (sigprocmask(SIG_SETMASK, &set, NULL) == -1) {
      printf("sigprocmask() failed\n");
   }

   /*
    * disable deleting previous events for the listening sockets because
    * in the worker processes there are no events at all at this point
    */
   ls = cycle->listening.elts;
   for (i = 0; i < cycle->listening.nelts; i++) {
      ls[i].previous = NULL;
   }
}

static void ngx_worker_process_exit(ngx_cycle_t *cycle)
{
   ngx_cycle = &ngx_exit_cycle;

   ngx_destroy_pool(cycle->pool);

   printf("exit worker process %d\n", ngx_pid);

   exit(0);
}

