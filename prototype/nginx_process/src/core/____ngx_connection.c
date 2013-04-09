
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <____ngx_config.h>
#include <____ngx_core.h>



ngx_int_t
ngx_set_inherited_sockets(ngx_cycle_t *cycle)
{
    size_t                     len;
    ngx_uint_t                 i;
    ngx_listening_t           *ls;
    socklen_t                  olen;
    int                        timeout;

    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {
        ls[i].sockaddr = ngx_palloc(cycle->pool, NGX_SOCKADDRLEN);
        if (ls[i].sockaddr == NULL) {
            return NGX_ERROR;
        }

        ls[i].socklen = NGX_SOCKADDRLEN;
        if (getsockname(ls[i].fd, ls[i].sockaddr, &ls[i].socklen) == -1) {
            printf("getsockname() of the inherited " "socket #%d failed", ls[i].fd);
            ls[i].ignore = 1;
            continue;
        }

        len = ngx_max(NGX_INET_ADDRSTRLEN, NGX_UNIX_ADDRSTRLEN) + sizeof(":65535") - 1;
        ls[i].addr_text.data = ngx_pnalloc(cycle->pool, len);
        if (ls[i].addr_text.data == NULL) {
            return NGX_ERROR;
        }

        switch (ls[i].sockaddr->sa_family) {

        case AF_UNIX: {
             struct sockaddr_un *saun = (struct sockaddr_un *) ls[i].sockaddr;
             ls[i].addr_text_max_len = NGX_UNIX_ADDRSTRLEN;
             len = ngx_snprintf(ls[i].addr_text.data, len, "unix:%s%Z", saun->sun_path) - ls[i].addr_text.data - 1;
             break;
        }
        case AF_INET: {
             struct sockaddr_in *sin = (struct sockaddr_in *) ls[i].sockaddr;
             char *p = (u_char *) &sin->sin_addr;
             ls[i].addr_text_max_len = NGX_INET_ADDRSTRLEN;
             len = ngx_snprintf(ls[i].addr_text.data, len, "%ud.%ud.%ud.%ud:%d", p[0], p[1], p[2], p[3], ntohs(sin->sin_port)) - ls[i].addr_text.data;
             break;
        }
        default:
            ls[i].ignore = 1;
            continue;
        }

        ls[i].addr_text.len = len;

        ls[i].backlog = NGX_LISTEN_BACKLOG;

        olen = sizeof(int);
        if (getsockopt(ls[i].fd, SOL_SOCKET, SO_RCVBUF, (void *) &ls[i].rcvbuf, &olen) == -1) {
            ls[i].rcvbuf = -1;
        }

        olen = sizeof(int);
        if (getsockopt(ls[i].fd, SOL_SOCKET, SO_SNDBUF, (void *) &ls[i].sndbuf, &olen) == -1) {
            ls[i].sndbuf = -1;
        }

        timeout = 0;
        olen = sizeof(int);
        if (getsockopt(ls[i].fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, &olen) == -1) {
            continue;
        }

        if (olen < sizeof(int) || timeout == 0) {
            continue;
        }

        ls[i].deferred_accept = 1;
    }

    return NGX_OK;
}


ngx_int_t
ngx_open_listening_sockets(ngx_cycle_t *cycle)
{
    int               reuseaddr;
    ngx_uint_t        i, tries, failed;
    ngx_err_t         err;
    ngx_log_t        *log;
    ngx_socket_t      s;
    ngx_listening_t  *ls;

    reuseaddr = 1;
    failed = 0;

    log = cycle->log;

    /* TODO: configurable try number */

    for (tries = 5; tries; tries--) {
        failed = 0;

        /* for each listening socket */

        ls = cycle->listening.elts;
        for (i = 0; i < cycle->listening.nelts; i++) {

            if (ls[i].ignore) {
                continue;
            }

            if (ls[i].fd != -1) {
                continue;
            }

            if (ls[i].inherited) {

                /* TODO: close on exit */
                /* TODO: nonblocking */
                /* TODO: deferred accept */

                continue;
            }

            s = socket(ls[i].sockaddr->sa_family, ls[i].type, 0);
            if (s == -1) {
                return NGX_ERROR;
            }

            if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void *) &reuseaddr, sizeof(int)) == -1) {
                close(s);
                return NGX_ERROR;
            }

            /* TODO: close on exit */


            if (bind(s, ls[i].sockaddr, ls[i].socklen) == -1) {
                err = errno;

                close(s);

                if (err != NGX_EADDRINUSE) {
                    return NGX_ERROR;
                }

                failed = 1;

                continue;
            }

            if (ls[i].sockaddr->sa_family == AF_UNIX) {
                mode_t   mode;
                u_char  *name;

                name = ls[i].addr_text.data + sizeof("unix:") - 1;
                mode = (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

                chmod((char *) name, mode);
            }
            if (listen(s, ls[i].backlog) == -1) {
                close(s);
                return NGX_ERROR;
            }

            ls[i].listen = 1;

            ls[i].fd = s;
        }

        if (!failed) {
            break;
        }

        /* TODO: delay configurable */

        ngx_msleep(500);
    }

    if (failed) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


void
ngx_configure_listening_sockets(ngx_cycle_t *cycle)
{
    int                        keepalive;
    ngx_uint_t                 i;
    ngx_listening_t           *ls;
    int                        timeout;

    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {

        ls[i].log = *ls[i].logp;

        if (ls[i].rcvbuf != -1) {
            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_RCVBUF, (const void *) &ls[i].rcvbuf, sizeof(int)) == -1) {
            }
        }

        if (ls[i].sndbuf != -1) {
            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_SNDBUF, (const void *) &ls[i].sndbuf, sizeof(int)) == -1) {
            }
        }

        if (ls[i].keepalive) {
            keepalive = (ls[i].keepalive == 1) ? 1 : 0;
            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_KEEPALIVE, (const void *) &keepalive, sizeof(int)) == -1) {
            }
        }

        if (ls[i].keepidle) {
            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPIDLE, (const void *) &ls[i].keepidle, sizeof(int)) == -1) {
            }
        }

        if (ls[i].keepintvl) {
            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPINTVL, (const void *) &ls[i].keepintvl, sizeof(int)) == -1) {
            }
        }

        if (ls[i].keepcnt) {
            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPCNT, (const void *) &ls[i].keepcnt, sizeof(int)) == -1) {
            }
        }

        if (ls[i].listen) {

            /* change backlog via listen() */

            if (listen(ls[i].fd, ls[i].backlog) == -1) {
            }
        }

        /*
         * setting deferred mode should be last operation on socket,
         * because code may prematurely continue cycle on failure
         */

        if (ls[i].add_deferred || ls[i].delete_deferred) {

            if (ls[i].add_deferred) {
                timeout = (int) (ls[i].post_accept_timeout / 1000);

            } else {
                timeout = 0;
            }

            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, sizeof(int)) == -1) {
                continue;
            }
        }

        if (ls[i].add_deferred) {
            ls[i].deferred_accept = 1;
        }
    }
    return;
}


void
ngx_close_listening_sockets(ngx_cycle_t *cycle)
{
    ngx_uint_t         i;
    ngx_listening_t   *ls;

    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {
        if (close(ls[i].fd) == -1) {
        }

        ls[i].fd = (ngx_socket_t) -1;
    }
}



