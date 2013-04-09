/*
 * Copyright (C) Nginx, Inc.
 */

#ifndef _NGX_SETAFFINITY_H_INCLUDED_
#define _NGX_SETAFFINITY_H_INCLUDED_


void ngx_setaffinity(uint64_t cpu_affinity, ngx_log_t *log);

#endif /* _NGX_SETAFFINITY_H_INCLUDED_ */
