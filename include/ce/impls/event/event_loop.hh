// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <event.h>

#include <chrono>
#include <cstdint>

#include <ce/config.h>
#include <ce/impls/event/event_deleter.hh>

namespace ce {

class event_loop {
 public:
  static void enable_multithread();

  event_loop();

  handle_id_t get_next_handle_id();

  ::event_base *get_native_base();

  int run();

  event_util::event_ptr new_event(int fd, short events, event_callback_fn cb,
                                  void *arg);

  void active(::event *ev);

 private:
  ::event_base *base_;
  handle_id_t _next_handle_id = 0;
};

}  // namespace ce
