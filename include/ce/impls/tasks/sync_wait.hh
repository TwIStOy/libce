// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <ce/concepts/awaiter_traits.hh>
#include <ce/impls/task.hh>
#include <ce/impls/tasks/sync_wait_promise.hh>

namespace ce {

template<typename T>
struct sync_wait_task {
  using promise_type = sync_wait_promise<T>;
  using handle_t     = std::coroutine_handle<promise_type>;

  explicit sync_wait_task(handle_t coro) noexcept : coro_(coro) {
  }

  sync_wait_task(sync_wait_task&& rhs) noexcept
      : coro_(std::exchange(rhs.coro_, handle_t{})) {
  }

  ~sync_wait_task() {
    if (coro_) {
      coro_.destroy();
    }
  }

  sync_wait_task(const sync_wait_task&)            = delete;
  sync_wait_task& operator=(const sync_wait_task&) = delete;

  void start(std::atomic_bool *done) noexcept {
    coro_.promise().start(done);
  }

  decltype(auto) result() {
    return coro_.promise().result();
  }

  handle_t coro_;
};

template<typename T,
         typename R = typename concepts::awaiter_traits<T>::awaiter_result_t>
sync_wait_task<R> make_sync_wait_task(T&& a) {
  if constexpr (std::same_as<R, void>) {
    co_await std::forward<T>(a);
  } else {
    co_yield co_await std::forward<T>(a);
  }
}

}  // namespace ce
