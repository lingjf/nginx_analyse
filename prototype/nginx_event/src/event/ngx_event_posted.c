
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <____ngx_config.h>
#include <____ngx_core.h>
#include <ngx_event.h>

ngx_event_t  *ngx_posted_accept_events;
ngx_event_t  *ngx_posted_events;


void
ngx_event_process_posted(ngx_cycle_t *cycle, ngx_event_t **posted)
{
    ngx_event_t  *ev;

    for ( ;; ) {

        ev = (ngx_event_t *) *posted;
        if (ev == NULL) {
            return;
        }

        ngx_delete_posted_event(ev);

        ev->handler(ev);
    }
}


