/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>

typedef struct
{
   int signo;
   char *signame;
   char *name;
   void (*handler)(int signo);
} ngx_signal_t;

static void ngx_execute_proc(ngx_cycle_t *cycle, void *data);
static void ngx_signal_handler(int signo);
static void ngx_process_get_status(void);

int ngx_argc;
char **ngx_argv;
char **ngx_os_argv;

ngx_int_t ngx_process_slot;
ngx_int_t ngx_last_process;
ngx_process_t ngx_processes[NGX_MAX_PROCESSES];

ngx_signal_t signals[] = { { ngx_signal_value(NGX_RECONFIGURE_SIGNAL), "SIG" ngx_value(NGX_RECONFIGURE_SIGNAL), "reload", ngx_signal_handler },

{ ngx_signal_value(NGX_REOPEN_SIGNAL), "SIG" ngx_value(NGX_REOPEN_SIGNAL), "reopen", ngx_signal_handler },

{ ngx_signal_value(NGX_NOACCEPT_SIGNAL), "SIG" ngx_value(NGX_NOACCEPT_SIGNAL), "", ngx_signal_handler },

{ ngx_signal_value(NGX_TERMINATE_SIGNAL), "SIG" ngx_value(NGX_TERMINATE_SIGNAL), "stop", ngx_signal_handler },

{ ngx_signal_value(NGX_SHUTDOWN_SIGNAL), "SIG" ngx_value(NGX_SHUTDOWN_SIGNAL), "quit", ngx_signal_handler },

{ ngx_signal_value(NGX_CHANGEBIN_SIGNAL), "SIG" ngx_value(NGX_CHANGEBIN_SIGNAL), "", ngx_signal_handler },

{ SIGALRM, "SIGALRM", "", ngx_signal_handler },

{ SIGINT, "SIGINT", "", ngx_signal_handler },

{ SIGIO, "SIGIO", "", ngx_signal_handler },

{ SIGCHLD, "SIGCHLD", "", ngx_signal_handler },

{ SIGSYS, "SIGSYS, SIG_IGN", "", SIG_IGN },

{ SIGPIPE, "SIGPIPE, SIG_IGN", "", SIG_IGN },

{ 0, NULL, "", NULL } };

ngx_pid_t ngx_spawn_process(ngx_cycle_t *cycle, ngx_spawn_proc_pt proc, void *data, char *name, ngx_int_t respawn)
{
   ngx_pid_t pid;
   ngx_int_t s;

   if (respawn >= 0) {
      s = respawn;

   } else {
      for (s = 0; s < ngx_last_process; s++) {
         if (ngx_processes[s].pid == -1) {
            break;
         }
      }

      if (s == NGX_MAX_PROCESSES) {
         printf("no more than %d processes can be spawned\n", NGX_MAX_PROCESSES);
         return NGX_INVALID_PID;
      }
   }

   ngx_process_slot = s;

   pid = fork();

   switch (pid) {

   case -1:
      printf("fork() failed while spawning \"%s\"\n", name);
      return NGX_INVALID_PID;

   case 0:
      ngx_pid = ngx_getpid();
      proc(cycle, data);
      break;

   default:
      break;
   }

   printf("start %s process %d\n", name, pid);

   ngx_processes[s].pid = pid;
   ngx_processes[s].exited = 0;

   if (respawn >= 0) {
      return pid;
   }

   ngx_processes[s].proc = proc;
   ngx_processes[s].data = data;
   ngx_processes[s].name = name;
   ngx_processes[s].exiting = 0;

   switch (respawn) {

   case NGX_PROCESS_NORESPAWN:
      ngx_processes[s].respawn = 0;
      ngx_processes[s].just_spawn = 0;
      ngx_processes[s].detached = 0;
      break;

   case NGX_PROCESS_JUST_SPAWN:
      ngx_processes[s].respawn = 0;
      ngx_processes[s].just_spawn = 1;
      ngx_processes[s].detached = 0;
      break;

   case NGX_PROCESS_RESPAWN:
      ngx_processes[s].respawn = 1;
      ngx_processes[s].just_spawn = 0;
      ngx_processes[s].detached = 0;
      break;

   case NGX_PROCESS_JUST_RESPAWN:
      ngx_processes[s].respawn = 1;
      ngx_processes[s].just_spawn = 1;
      ngx_processes[s].detached = 0;
      break;

   case NGX_PROCESS_DETACHED:
      ngx_processes[s].respawn = 0;
      ngx_processes[s].just_spawn = 0;
      ngx_processes[s].detached = 1;
      break;
   }

   if (s == ngx_last_process) {
      ngx_last_process++;
   }

   return pid;
}

ngx_pid_t ngx_execute(ngx_cycle_t *cycle, ngx_exec_ctx_t *ctx)
{
   return ngx_spawn_process(cycle, ngx_execute_proc, ctx, ctx->name, NGX_PROCESS_DETACHED);
}

static void ngx_execute_proc(ngx_cycle_t *cycle, void *data)
{
   ngx_exec_ctx_t *ctx = data;

   if (execve(ctx->path, ctx->argv, ctx->envp) == -1) {
      printf("execve() failed while executing %s \"%s\"\n", ctx->name, ctx->path);
   }

   exit(1);
}

ngx_int_t ngx_init_signals(ngx_log_t *log)
{
   ngx_signal_t *sig;
   struct sigaction sa;

   for (sig = signals; sig->signo != 0; sig++) {
      ngx_memzero(&sa, sizeof(struct sigaction));
      sa.sa_handler = sig->handler;
      sigemptyset(&sa.sa_mask);
      if (sigaction(sig->signo, &sa, NULL) == -1) {
         printf("sigaction(%s) failed\n", sig->signame);
         return NGX_ERROR;
      }
   }

   return NGX_OK;
}

void ngx_signal_handler(int signo)
{
   char *action;
   ngx_int_t ignore;
   ngx_err_t err;
   ngx_signal_t *sig;

   ignore = 0;

   err = errno;

   for (sig = signals; sig->signo != 0; sig++) {
      if (sig->signo == signo) {
         break;
      }
   }

   action = "";

   switch (ngx_process) {

   case NGX_PROCESS_MASTER:
      switch (signo) {

      case ngx_signal_value(NGX_SHUTDOWN_SIGNAL):
         ngx_quit = 1;
         action = ", shutting down";
         break;

      case ngx_signal_value(NGX_TERMINATE_SIGNAL):
      case SIGINT:
         ngx_terminate = 1;
         action = ", exiting";
         break;

      case ngx_signal_value(NGX_RECONFIGURE_SIGNAL):
         ngx_reconfigure = 1;
         action = ", reconfiguring";
         break;

      case ngx_signal_value(NGX_CHANGEBIN_SIGNAL):
         if (getppid() > 1 || ngx_new_binary > 0) {

            /* Ignore the signal in the new binary if its parent is not the init process, i.e. the old binary's process is still running.
             * Or ignore the signal in the old binary's process if the new binary's process is already running. */

            action = ", ignoring";
            ignore = 1;
            break;
         }

         ngx_change_binary = 1;
         action = ", changing binary";
         break;

      case SIGALRM:
         ngx_sigalrm = 1;
         break;

      case SIGCHLD:
         ngx_reap = 1;
         break;
      }

      break;

   case NGX_PROCESS_WORKER:
      switch (signo) {

      case ngx_signal_value(NGX_SHUTDOWN_SIGNAL):
         ngx_quit = 1;
         action = ", shutting down";
         break;

      case ngx_signal_value(NGX_TERMINATE_SIGNAL):
      case SIGINT:
         ngx_terminate = 1;
         action = ", exiting";
         break;

      case ngx_signal_value(NGX_RECONFIGURE_SIGNAL):
      case ngx_signal_value(NGX_CHANGEBIN_SIGNAL):
      case SIGIO:
         action = ", ignoring";
         break;
      }

      break;
   }

   if (ignore) {
      printf("the changing binary signal is ignored: you should shutdown or terminate before either old or new binary's process\n");
   }

   if (signo == SIGCHLD) {
      ngx_process_get_status();
   }

   errno = err;
}

static void ngx_process_get_status(void)
{
   int status;
   char *process;
   ngx_pid_t pid;
   ngx_err_t err;
   ngx_int_t i;
   ngx_uint_t one;

   one = 0;

   for (;;) {
      pid = waitpid(-1, &status, WNOHANG);
      if (pid == 0) {
         return;
      }

      if (pid == -1) {
         err = errno;
         if (err == NGX_EINTR) {
            continue;
         }
         if (err == NGX_ECHILD && one) {
            return;
         }
         return;
      }

      one = 1;
      process = "unknown process";

      for (i = 0; i < ngx_last_process; i++) {
         if (ngx_processes[i].pid == pid) {
            ngx_processes[i].exited = 1;
            process = ngx_processes[i].name;
            break;
         }
      }

      if (WTERMSIG(status)) {
         printf("%s %d exited on signal %d\n", process, pid, WTERMSIG(status));
      } else {
         printf("%s %d exited with code %d\n", process, pid, WEXITSTATUS(status));
      }

      if (WEXITSTATUS(status) == 2 && ngx_processes[i].respawn) {
         printf("%s %d exited with fatal code %d and cannot be respawned\n", process, pid, WEXITSTATUS(status));
         ngx_processes[i].respawn = 0;
      }
      printf("%s\n", jeff_process_tustring(&ngx_processes[i]));
   }
}

ngx_int_t ngx_os_signal_process(ngx_cycle_t *cycle, char *name, ngx_int_t pid)
{
   ngx_signal_t *sig;

   for (sig = signals; sig->signo != 0; sig++) {
      if (ngx_strcmp(name, sig->name) == 0) {
         if (kill(pid, sig->signo) != -1) {
            return 0;
         }
         printf("kill(%d, %d) failed\n", pid, sig->signo);
      }
   }

   return 1;
}

u_char *jeff_process_tustring(ngx_process_t *p)
{
   static u_char buffer[1024 * 8];
   memset(buffer, 0, sizeof(buffer));
   if (!p) return "NULL";
   ngx_snprintf(buffer, sizeof(buffer), "ngx_process_t{pid=%P,exiting=%d,exited=%d,detached=%d,respawn=%d,just_spawn=%d}", p->pid, p->exiting, p->exited, p->detached, p->respawn, p->just_spawn);
   return buffer;
}
