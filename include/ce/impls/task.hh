// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <coroutine>
#include <stop_token>
#include <utility>

#include <ce/base/assert.hh>
#include <ce/base/cppfeature.hh>
#include <ce/fwd/task_fwd.hh>
#include <ce/impls/cpos/get_stop_token.hh>
#include <ce/impls/stop_token_adapter.hh>
#include <ce/impls/task_promise.hh>

namespace ce {

namespace _task_impl {

template<typename ThisPromise, typename OtherPromise>
struct _awaiter {
  using result_type = typename ThisPromise::result_type;

  explicit _awaiter(std::coroutine_handle<ThisPromise> coro) noexcept
      : coro_(coro) {
  }

  _awaiter(_awaiter&& other) noexcept : coro_(std::exchange(other.coro_, {})) {
  }

  ~_awaiter() {
    if (coro_) {
      coro_.destroy();
    }
  }

  bool await_ready() noexcept {
    return false;
  }

  std::coroutine_handle<ThisPromise> await_suspend(
      std::coroutine_handle<OtherPromise> h) noexcept {
    CE_ASSERT(coro_);

    auto& promise   = coro_.promise();
    promise.handle_ = h;

    stop_token_adapter_.subscribe(get_stop_token(h.promise()));
    return coro_;
  }

  result_type await_resume() {
    // stopTokenAdapter_.unsubscribe();
    scope_guard defer{[this]() noexcept {
      std::exchange(coro_, {}).destroy();
    }};
    return coro_.promise().result();
  }

 private:
  std::coroutine_handle<ThisPromise> coro_;
  stop_token_adapter_subscription<
      tag_invoke_result_t<tag_t<get_stop_token>, OtherPromise>>
      stop_token_adapter_;
};

}  // namespace _task_impl

template<typename T>
class Task {
 public:
  using promise_type = _task_impl::promise<T>;

  ~Task() {
    if (handle_) {
      handle_.destroy();
    }
  }

  Task(Task&& other) noexcept : handle_(other.handle_) {
    other.handle_ = nullptr;
  }

  Task& operator=(Task&& other) noexcept {
    if (handle_) {
      handle_.destroy();
    }
    handle_       = other.handle_;
    other.handle_ = nullptr;
  }

 private:
  friend promise_type;

  explicit Task(std::coroutine_handle<promise_type> handle) noexcept
      : handle_(handle) {
  }

  template<typename OtherPromise>
  using _awaiter = _task_impl::_awaiter<promise_type, OtherPromise>;

  friend auto tag_invoke(tag_t<await_transform>, auto& rhs_promise,
                         Task&& t) noexcept {
    return _awaiter<CE_TYPEOF(rhs_promise)>{std::exchange(t.handle_, {})};
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};

}  // namespace ce
