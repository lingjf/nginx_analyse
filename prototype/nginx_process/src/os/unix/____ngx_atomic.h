/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#ifndef _NGX_ATOMIC_H_INCLUDED_
#define _NGX_ATOMIC_H_INCLUDED_

#include <____ngx_config.h>
#include <____ngx_core.h>

/* GCC 4.1 builtin atomic operations */

#define NGX_HAVE_ATOMIC_OPS  1

typedef long ngx_atomic_int_t;
typedef unsigned long ngx_atomic_uint_t;

#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)

typedef volatile ngx_atomic_uint_t ngx_atomic_t;

#define ngx_atomic_cmp_set(lock, old, set) __sync_bool_compare_and_swap(lock, old, set)

#define ngx_atomic_fetch_add(value, add) __sync_fetch_and_add(value, add)

#define ngx_memory_barrier() __sync_synchronize()

#define ngx_cpu_pause()

#define ngx_trylock(lock)  (*(lock) == 0 && ngx_atomic_cmp_set(lock, 0, 1))
#define ngx_unlock(lock)    *(lock) = 0

static inline void
ngx_spinlock(ngx_atomic_t *lock, ngx_atomic_int_t value, ngx_uint_t spin)
{
    ngx_uint_t  i, n;

    for ( ;; ) {

        if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, value)) {
            return;
        }

        if (ngx_ncpu > 1) {

            for (n = 1; n < spin; n <<= 1) {

                for (i = 0; i < n; i++) {
                    ngx_cpu_pause();
                }

                if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, value)) {
                    return;
                }
            }
        }

        ngx_sched_yield();
    }
}


#endif /* _NGX_ATOMIC_H_INCLUDED_ */
