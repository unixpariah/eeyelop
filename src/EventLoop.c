#include <EventLoop.h>
#include <assert.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>

void event_loop_init(EventLoop *event_loop) {
  event_loop->pollfds = NULL;
  event_loop->polldata = NULL;
  event_loop->len = 0;
  event_loop->capacity = 0;
}

uint32_t event_loop_grow_capacity(uint32_t current, uint32_t minimum) {
  uint32_t new_capacity = current;
  while (new_capacity < minimum) {
    new_capacity += new_capacity / 2 + 8;
  }
  return new_capacity;
}

enum EventLoopResult
event_loop_ensure_total_capacity_precise(EventLoop *event_loop,
                                         uint32_t new_capacity) {
  if (event_loop->capacity >= new_capacity) {
    return EVENT_LOOP_OK;
  }

  struct pollfd *new_pollfds =
      realloc(event_loop->pollfds, new_capacity * sizeof(struct pollfd));

  if (!new_pollfds) {
    return EVENT_LOOP_OOM;
  }

  PollData *new_polldata =
      realloc(event_loop->polldata, new_capacity * sizeof(PollData));

  if (!new_polldata) {
    free(new_pollfds);
    return EVENT_LOOP_OOM;
  }

  event_loop->capacity = new_capacity;
  event_loop->pollfds = new_pollfds;
  event_loop->polldata = new_polldata;

  return EVENT_LOOP_OK;
}

enum EventLoopResult event_loop_ensure_total_capacity(EventLoop *event_loop,
                                                      uint32_t new_capacity) {
  if (event_loop->capacity >= new_capacity) {
    return EVENT_LOOP_OK;
  }

  uint32_t better_capacity =
      event_loop_grow_capacity(event_loop->capacity, new_capacity);
  return event_loop_ensure_total_capacity_precise(event_loop, better_capacity);
}

enum EventLoopResult event_loop_insert_source(EventLoop *event_loop, int32_t fd,
                                              void (*callback)(void *data),
                                              void *data, int32_t repeats) {
  uint32_t new_len = event_loop->len + 1;
  if (event_loop_ensure_total_capacity_precise(event_loop, new_len) ==
      EVENT_LOOP_OOM) {
    return EVENT_LOOP_OOM;
  }

  assert(event_loop->len < event_loop->capacity);
  event_loop->len++;

  struct pollfd pollfd = {
      .fd = fd,
      .events = POLLIN,
      .revents = 0,
  };

  PollData polldata = {
      .data = data,
      .repeats = repeats,
      .callback = callback,
      .fd = fd,
  };

  event_loop->polldata[event_loop->len - 1] = polldata;
  event_loop->pollfds[event_loop->len - 1] = pollfd;

  return EVENT_LOOP_OK;
}

enum EventLoopResult event_loop_poll(EventLoop *event_loop) {
  for (int32_t i = (int32_t)event_loop->len - 1; i >= 0; i--) {
    PollData polldata = event_loop->polldata[i];
    if (polldata.repeats == 0) {
      event_loop_remove_source(event_loop, i);
      printf("still removes\n");
    }
  }

  int32_t events = poll(event_loop->pollfds, event_loop->len, -1);

  if (events == -1) {
    return EVENT_LOOP_POLL_ERR;
  }

  for (int i = 0; i < events; i++) {
    for (uint32_t j = 0; j < event_loop->len; j++) {
      struct pollfd pollfd = event_loop->pollfds[j];

      if (pollfd.revents & POLLIN) {
        PollData polldata = event_loop->polldata[j];
        polldata.callback(polldata.data);
        if (polldata.repeats > 0) {
          polldata.repeats--;
        }
      }
    }
  }

  return EVENT_LOOP_OK;
}

PollData event_loop_remove_source(EventLoop *event_loop, uint32_t index) {
  assert(index < 0 || index < event_loop->len);

  if (event_loop->len - 1 == index) {
    return event_loop_pop(event_loop);
  }

  PollData old_polldata = event_loop->polldata[index];
  event_loop->polldata[index] = event_loop_pop(event_loop);
  struct pollfd pollfd = {
      .fd = event_loop->polldata[index].fd,
      .events = POLLIN,
      .revents = 0,
  };
  event_loop->pollfds[index] = pollfd;

  return old_polldata;
}

PollData event_loop_pop(EventLoop *event_loop) {
  assert(event_loop->len > 0);

  PollData polldata = event_loop->polldata[event_loop->len - 1];

  event_loop->len--;

  struct pollfd *old_pollfd = &event_loop->pollfds[event_loop->len];

  old_pollfd->events = 0;
  old_pollfd->revents = 0;
  old_pollfd->fd = 0;

  PollData *old_polldata = &event_loop->polldata[event_loop->len];

  old_polldata->fd = 0;
  old_polldata->repeats = 0;
  old_polldata->data = 0;
  old_polldata->callback = 0;

  return polldata;
}

void event_loop_deinit(EventLoop *event_loop) {
  free(event_loop->pollfds);
  free(event_loop->polldata);
}
