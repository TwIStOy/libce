// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <thread>

#include <ce/base/cppfeature.hh>
#include <ce/impls/tasks/sync_wait.hh>

namespace ce {

auto sync_wait(auto&& t) {
  auto tt               = make_sync_wait_task(CE_FWD(t));
  std::atomic_bool done = false;
  tt.start(&done);
  while (!done) {
    std::this_thread::yield();
  }
  return tt.result();
}

}  // namespace ce
