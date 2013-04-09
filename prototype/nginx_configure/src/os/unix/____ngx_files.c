/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <____ngx_config.h>
#include <____ngx_core.h>

ssize_t ngx_read_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset)
{
   ssize_t n;

   if (file->sys_offset != offset) {
      if (lseek(file->fd, offset, SEEK_SET) == -1) {
         return NGX_ERROR;
      }

      file->sys_offset = offset;
   }

   n = read(file->fd, buf, size);

   if (n == -1) {
      return NGX_ERROR;
   }

   file->sys_offset += n;

   file->offset += n;

   return n;
}

ssize_t ngx_write_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset)
{
   ssize_t n, written;

   written = 0;

   if (file->sys_offset != offset) {
      if (lseek(file->fd, offset, SEEK_SET) == -1) {
         return NGX_ERROR;
      }

      file->sys_offset = offset;
   }

   for (;;) {
      n = write(file->fd, buf + written, size);

      if (n == -1) {
         return NGX_ERROR;
      }

      file->offset += n;
      written += n;

      if ((size_t) n == size) {
         return written;
      }

      size -= n;
   }
}

ngx_int_t ngx_open_glob(ngx_glob_t *gl)
{
   int n;

   n = glob((char *) gl->pattern, GLOB_NOSORT, NULL, &gl->pglob);

   if (n == 0) {
      return NGX_OK;
   }

   return NGX_ERROR;
}

ngx_int_t ngx_read_glob(ngx_glob_t *gl, ngx_str_t *name)
{
   size_t count;

   count = (size_t) gl->pglob.gl_pathc;

   if (gl->n < count) {

      name->len = (size_t) ngx_strlen(gl->pglob.gl_pathv[gl->n]);
      name->data = (u_char *) gl->pglob.gl_pathv[gl->n];
      gl->n++;

      return NGX_OK;
   }

   return NGX_DONE;
}

void ngx_close_glob(ngx_glob_t *gl)
{
   globfree(&gl->pglob);
}

