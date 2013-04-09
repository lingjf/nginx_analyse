/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#ifndef _NGX_FILES_H_INCLUDED_
#define _NGX_FILES_H_INCLUDED_

#include <____ngx_config.h>
#include <____ngx_core.h>

typedef int ngx_fd_t;
typedef struct stat ngx_file_info_t;
typedef ino_t ngx_file_uniq_t;

struct ngx_file_s {
    ngx_fd_t                   fd;
    ngx_str_t                  name;
    ngx_file_info_t            info;

    off_t                      offset;
    off_t                      sys_offset;

    ngx_log_t                 *log;

    unsigned                   valid_info:1;
    unsigned                   directio:1;
};

typedef struct
{
   size_t n;
   glob_t pglob;
   u_char *pattern;
   ngx_log_t *log;
   ngx_uint_t test;
} ngx_glob_t;

#define NGX_INVALID_FILE         -1
#define NGX_FILE_ERROR           -1

#define ngx_open_file(name, mode, create, access) open((const char *) name, mode|create, access)
#define ngx_open_file_n          "open()"

#define NGX_FILE_RDONLY          O_RDONLY
#define NGX_FILE_WRONLY          O_WRONLY
#define NGX_FILE_RDWR            O_RDWR
#define NGX_FILE_CREATE_OR_OPEN  O_CREAT
#define NGX_FILE_OPEN            0
#define NGX_FILE_TRUNCATE        O_CREAT|O_TRUNC
#define NGX_FILE_APPEND          O_WRONLY|O_APPEND
#define NGX_FILE_NONBLOCK        O_NONBLOCK

#define NGX_FILE_DEFAULT_ACCESS  0644
#define NGX_FILE_OWNER_ACCESS    0600

#define ngx_close_file           close
#define ngx_close_file_n         "close()"

ssize_t ngx_read_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset);
#define ngx_read_file_n          "read()"

ssize_t ngx_write_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset);
#define ngx_read_fd              read
#define ngx_read_fd_n            "read()"

#define ngx_write_fd             write
#define ngx_write_fd_n           "write()"

#define ngx_linefeed(p)          *p++ = LF;
#define NGX_LINEFEED_SIZE        1
#define NGX_LINEFEED             "\x0a"

#define ngx_fd_info(fd, sb)      fstat(fd, sb)
#define ngx_fd_info_n            "fstat()"
#define ngx_file_size(sb)        (sb)->st_size

ngx_int_t ngx_open_glob(ngx_glob_t *gl);
#define ngx_open_glob_n          "glob()"
ngx_int_t ngx_read_glob(ngx_glob_t *gl, ngx_str_t *name);
void ngx_close_glob(ngx_glob_t *gl);

#endif /* _NGX_FILES_H_INCLUDED_ */
