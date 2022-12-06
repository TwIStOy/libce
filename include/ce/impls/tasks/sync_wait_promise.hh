// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <atomic>
#include <cassert>
#include <coroutine>
#include <exception>

#include <ce/base/assert.hh>
#include <ce/impls/cpos/await_transform.hh>
#include <ce/impls/cpos/exchange_continuation_handle.hh>
#include <ce/impls/cpos/get_stop_token.hh>
#include <ce/impls/stop_token.hh>
#include <ce/impls/tasks/sync_wait_fwd.hh>
#include <memory>
#include <type_traits>
#include <variant>

namespace ce {

namespace _sync_wait_impl {

struct sync_wait_promise_base {
  std::atomic_bool *done_ = nullptr;

  struct final_awaiter {
    bool await_ready() const noexcept {
      return false;
    }

    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> co) const noexcept {
      // notify that sub-task is done
      *co.promise().done_ = true;
    }

    void await_resume() const noexcept {
    }
  };

  auto initial_suspend() noexcept {
    return std::suspend_always{};
  }

  auto final_suspend() noexcept {
    return final_awaiter{};
  }
};

template<typename T>
struct _return_function_base {
  auto yield_value(T&& value) {
    value_.template emplace<1>(std::addressof(value));
    return sync_wait_promise_base::final_awaiter{};
  }

  std::variant<std::monostate, std::remove_reference_t<T> *, std::exception_ptr>
      value_;
};

template<>
struct _return_function_base<void> {
  auto return_void() {
    value_.template emplace<1>();
    return sync_wait_promise_base::final_awaiter{};
  }

  std::variant<std::monostate, std::monostate, std::exception_ptr> value_;
};

}  // namespace _sync_wait_impl

template<typename T>
struct sync_wait_promise : _sync_wait_impl::sync_wait_promise_base,
                           _sync_wait_impl::_return_function_base<T> {
  using handle_t = std::coroutine_handle<sync_wait_promise>;

  sync_wait_promise() = default;

  void start(std::atomic_bool *done) {
    this->done_ = done;
    handle_t::from_promise(*this).resume();
  }

  auto get_return_object() noexcept {
    return sync_wait_task<T>{handle_t::from_promise(*this)};
  }

  auto unhandle_done() {
    return handle_t::from_promise(*this);
  }

  void unhandled_exception() {
    this->value_.template emplace<2>(std::current_exception());
  }

  T&& result() {
    if (this->value_.index() == 2) {
      std::rethrow_exception(std::get<2>(this->value_));
    }
    return static_cast<T&&>(*std::get<1>(this->value_));
  }

  auto await_transform(auto&& value) {
    return ::ce::await_transform(*this, CE_FWD(value));
  }

  ce::stop_token stoken_;

  friend auto tag_invoke(tag_t<get_stop_token>,
                         const sync_wait_promise& p) noexcept {
    return p.stoken_;
  }
};

}  // namespace ce
