// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#include <ce/impls/event/event_loop.hh>

#include <event2/event.h>
#include <event2/thread.h>

namespace ce {

void event_loop::enable_multithread() {
  evthread_use_pthreads();
}

event_loop::event_loop() {
  base_ = ::event_base_new();
}

handle_id_t event_loop::get_next_handle_id() {
  return _next_handle_id++;
}

auto event_loop::get_native_base() -> ::event_base * {
  return base_;
}

auto event_loop::new_event(int fd, short events, event_callback_fn cb,
                           void *arg) -> event_util::event_ptr {
  event_util::event_ptr ev{event_new(get_native_base(), fd, events, cb, arg)};
  return ev;
}

int event_loop::run() {
  return event_base_dispatch(get_native_base());
}

void event_loop::active(::event *ev) {
  ::event_active(ev, 0, 0);
}

}  // namespace ce
