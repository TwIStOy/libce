// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <event.h>

#include <cstdint>

#include <ce/config.h>

namespace ce {

class EventLoop {
 public:
  EventLoop();

  handle_id_t get_next_handle_id();

 private:
  ::event_base *base_;
  handle_id_t _next_handle_id = 0;
};

inline CE_ALWAYS_INLINE handle_id_t EventLoop::get_next_handle_id() {
  return _next_handle_id++;
}

}  // namespace ce
