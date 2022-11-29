// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <ce/impls/event/event_loop.hh>
#include <ce/impls/task.hh>

namespace ce {

template<typename T>
inline T sync_wait(Task<T> t) {
  event_loop loop;

  // TODO(hawtian):

  // loop.new_event(-1, EV_READ, event_callback_fn cb, void *arg);

  loop.run();
}

}  // namespace ce
