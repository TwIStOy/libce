// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#include <event2/event.h>

#include <ce/impls/event/event_loop.hh>

namespace ce {

handle_id_t EventLoop::get_next_handle_id() {
  return _next_handle_id++;
}

auto EventLoop::get_native_base() -> ::event_base * {
  return base_;
}

auto EventLoop::new_event(int fd, short events, event_callback_fn cb, void *arg)
    -> event_util::EventPtr {
  event_util::EventPtr ev{event_new(get_native_base(), fd, events, cb, arg)};
  return ev;
}

int EventLoop::run() {
  return event_base_dispatch(get_native_base());
}

void EventLoop::active(::event *ev) {
  ::event_active(ev, 0, 0);
}

}  // namespace ce
