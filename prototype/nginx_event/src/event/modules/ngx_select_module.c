
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <____ngx_config.h>
#include <____ngx_core.h>
#include <ngx_event.h>

static ngx_int_t ngx_select_init(ngx_cycle_t *cycle, ngx_msec_t timer);
static void ngx_select_done(ngx_cycle_t *cycle);
static ngx_int_t ngx_select_add_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
static ngx_int_t ngx_select_del_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
static ngx_int_t ngx_select_process_events(ngx_cycle_t *cycle, ngx_msec_t timer, ngx_uint_t flags);
static void ngx_select_repair_fd_sets(ngx_cycle_t *cycle);

static fd_set master_read_fd_set;
static fd_set master_write_fd_set;
static fd_set work_read_fd_set;
static fd_set work_write_fd_set;

static ngx_int_t max_fd;
static ngx_uint_t nevents;

static ngx_event_t **event_index;

static ngx_str_t select_name = ngx_string("select");

ngx_event_module_t  ngx_select_module_ctx = {
    &select_name,

    {
        ngx_select_add_event,              /* add an event */
        ngx_select_del_event,              /* delete an event */
        ngx_select_add_event,              /* enable an event */
        ngx_select_del_event,              /* disable an event */
        NULL,                              /* add an connection */
        NULL,                              /* delete an connection */
        NULL,                              /* process the changes */
        ngx_select_process_events,         /* process the events */
        ngx_select_init,                   /* init the events */
        ngx_select_done                    /* done the events */
    }

};

ngx_module_t  ngx_select_module = {
    &ngx_select_module_ctx,                /* module context */
    NGX_EVENT_MODULE,                      /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
};

static ngx_int_t ngx_select_init(ngx_cycle_t *cycle, ngx_msec_t timer)
{
   ngx_event_t **index;

   if (event_index == NULL) {
      FD_ZERO(&master_read_fd_set);
      FD_ZERO(&master_write_fd_set);
      nevents = 0;
   }

   index = ngx_alloc(sizeof(ngx_event_t *) * 2 * cycle->connection_n, cycle->log);
   if (index == NULL) {
      return NGX_ERROR;
   }

   if (event_index) {
      ngx_memcpy(index, event_index, sizeof(ngx_event_t *) * nevents);
      ngx_free(event_index);
   }

   event_index = index;

   ngx_io = ngx_os_io;

   ngx_event_actions = ngx_select_module_ctx.actions;

   ngx_event_flags = NGX_USE_LEVEL_EVENT;

   max_fd = -1;

   return NGX_OK;
}

static void ngx_select_done(ngx_cycle_t *cycle)
{
   ngx_free(event_index);

   event_index = NULL;
}

static ngx_int_t ngx_select_add_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags)
{
   ngx_connection_t *c;

   c = ev->data;

   if (ev->index != NGX_INVALID_INDEX) {
      return NGX_OK;
   }

   if ((event == NGX_READ_EVENT && ev->write) || (event == NGX_WRITE_EVENT && !ev->write)) {
      return NGX_ERROR;
   }

   if (event == NGX_READ_EVENT) {
      FD_SET(c->fd, &master_read_fd_set);

   } else if (event == NGX_WRITE_EVENT) {
      FD_SET(c->fd, &master_write_fd_set);
   }

   if (max_fd != -1 && max_fd < c->fd) {
      max_fd = c->fd;
   }

   ev->active = 1;

   event_index[nevents] = ev;
   ev->index = nevents;
   nevents++;

   return NGX_OK;
}

static ngx_int_t ngx_select_del_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags)
{
   ngx_event_t *e;
   ngx_connection_t *c;

   c = ev->data;

   ev->active = 0;

   if (ev->index == NGX_INVALID_INDEX) {
      return NGX_OK;
   }

   if (event == NGX_READ_EVENT) {
      FD_CLR(c->fd, &master_read_fd_set);

   } else if (event == NGX_WRITE_EVENT) {
      FD_CLR(c->fd, &master_write_fd_set);
   }

   if (max_fd == c->fd) {
      max_fd = -1;
   }

   if (ev->index < --nevents) {
      e = event_index[nevents];
      event_index[ev->index] = e;
      e->index = ev->index;
   }

   ev->index = NGX_INVALID_INDEX;

   return NGX_OK;
}

static ngx_int_t ngx_select_process_events(ngx_cycle_t *cycle, ngx_msec_t timer, ngx_uint_t flags)
{
   int ready, nready;
   ngx_err_t err;
   ngx_uint_t i, found;
   ngx_event_t *ev, **queue;
   struct timeval tv, *tp;
   ngx_connection_t *c;

   if (max_fd == -1) {
      for (i = 0; i < nevents; i++) {
         c = event_index[i]->data;
         if (max_fd < c->fd) {
            max_fd = c->fd;
         }
      }
   }

   if (timer == NGX_TIMER_INFINITE) {
      tp = NULL;

   } else {
      tv.tv_sec = (long) (timer / 1000);
      tv.tv_usec = (long) ((timer % 1000) * 1000);
      tp = &tv;
   }

   work_read_fd_set = master_read_fd_set;
   work_write_fd_set = master_write_fd_set;

   ready = select(max_fd + 1, &work_read_fd_set, &work_write_fd_set, NULL, tp);

   err = (ready == -1) ? ngx_errno : 0;

   if (flags & NGX_UPDATE_TIME || ngx_event_timer_alarm) {
      ngx_time_update();
   }

   if (err) {
      if (err == NGX_EINTR) {
         if (ngx_event_timer_alarm) {
            ngx_event_timer_alarm = 0;
            printf("select() returned timer triggered \n");
            return NGX_OK;
         }
      }
      if (err == EBADF) {
         ngx_select_repair_fd_sets(cycle);
      }
      return NGX_ERROR;
   }
   if (ready == 0) {
      if (timer != NGX_TIMER_INFINITE) {
         return NGX_OK;
      }
      return NGX_ERROR;
   }

   nready = 0;

   for (i = 0; i < nevents; i++) {
      ev = event_index[i];
      c = ev->data;
      found = 0;

      if (ev->write) {
         if (FD_ISSET(c->fd, &work_write_fd_set)) {
            found = 1;
         }

      } else {
         if (FD_ISSET(c->fd, &work_read_fd_set)) {
            found = 1;
         }
      }

      if (found) {
         ev->ready = 1;

         queue = (ngx_event_t **) (ev->accept ? &ngx_posted_accept_events : &ngx_posted_events);
         ngx_locked_post_event(ev, queue);

         nready++;
      }
   }

   if (ready != nready) {
      ngx_select_repair_fd_sets(cycle);
   }

   return NGX_OK;
}

static void ngx_select_repair_fd_sets(ngx_cycle_t *cycle)
{
   int n;
   socklen_t len;
   ngx_socket_t s;

   for (s = 0; s <= max_fd; s++) {

      if (FD_ISSET(s, &master_read_fd_set) == 0) {
         continue;
      }

      len = sizeof(int);

      if (getsockopt(s, SOL_SOCKET, SO_TYPE, &n, &len) == -1) {
         FD_CLR(s, &master_read_fd_set);
      }
   }

   for (s = 0; s <= max_fd; s++) {

      if (FD_ISSET(s, &master_write_fd_set) == 0) {
         continue;
      }

      len = sizeof(int);

      if (getsockopt(s, SOL_SOCKET, SO_TYPE, &n, &len) == -1) {
         FD_CLR(s, &master_write_fd_set);
      }
   }

   max_fd = -1;
}

