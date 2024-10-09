#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "sys/poll.h"
#include <stdint.h>

enum EventLoopResult {
  EVENT_LOOP_OK = 0,
  EVENT_LOOP_OOM = 1,
  EVENT_LOOP_POLL_ERR = 2,
};

typedef struct {
  void (*callback)(void *data);
  void *data;
  int32_t repeats;
} PollData;

typedef struct {
  struct pollfd *pollfds;
  PollData *polldata;
  uint32_t len;
  uint32_t capacity;
} EventLoop;

void event_loop_init(EventLoop *event_loop);

enum EventLoopResult event_loop_insert_source(EventLoop *event_loop, int32_t fd,
                                              void (*callback)(void *data),
                                              void *data, int32_t repeats);

enum EventLoopResult event_loop_poll(EventLoop *event_loop);

void event_loop_deinit(EventLoop *event_loop);

#endif // EVENT_LOOP_H
