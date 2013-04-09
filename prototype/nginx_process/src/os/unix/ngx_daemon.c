/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>

ngx_int_t ngx_daemon(ngx_log_t *log)
{
   int fd;

   switch (fork()) {
   case -1:
      printf("fork() failed");
      return NGX_ERROR;

   case 0: /* child process */
      break;

   default: /* parent process*/
      exit(0);
   }

   ngx_pid = ngx_getpid();

   if (setsid() == -1) {
      printf("setsid() failed");
      return NGX_ERROR;
   }

   umask(0);

   fd = open("/dev/null", O_RDWR);
   if (fd == -1) {
      printf("open(\"/dev/null\") failed");
      return NGX_ERROR;
   }

   if (dup2(fd, STDIN_FILENO) == -1) {
      printf("dup2(STDIN) failed");
      return NGX_ERROR;
   }

   if (dup2(fd, STDOUT_FILENO) == -1) {
      printf("dup2(STDOUT) failed");
      return NGX_ERROR;
   }

   if (fd > STDERR_FILENO) {
      if (close(fd) == -1) {
         printf("close() failed");
         return NGX_ERROR;
      }
   }

   return NGX_OK;
}
