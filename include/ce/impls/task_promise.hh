// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <coroutine>
#include <exception>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

#include <ce/fwd/task_fwd.hh>

#include <ce/impls/continuations.hh>
#include <ce/impls/cpos/await_transform.hh>
#include <ce/impls/cpos/exchange_continuation_handle.hh>
#include <ce/impls/cpos/get_stop_token.hh>
#include <ce/impls/cpos/visit_continuation.hh>
#include <ce/impls/stop_token.hh>

namespace ce::_task_impl {

struct promise_base {
  struct final_awaiter_base {
    [[nodiscard]] static inline constexpr bool await_ready() noexcept {
      return false;
    }

    inline constexpr void await_resume() noexcept {
      // do nothing
    }
  };

  virtual ~promise_base() = default;

  promise_base() = default;

  [[nodiscard]] static inline constexpr auto initial_suspend() noexcept {
    return std::suspend_always{};
  }

  auto unhandle_done() noexcept {
    return handle_.done();
  }

  friend stop_token tag_invoke(tag_t<get_stop_token>,
                               const promise_base& p) noexcept {
    return p.stoken_;
  }

  friend continuation_handle<> tag_invoke(
      tag_t<exchange_continuation>, promise_base& p,
      continuation_handle<> handle) noexcept {
    return std::exchange(p.handle_, handle);
  }

  friend void tag_invoke(tag_t<visit_continuation>, const promise_base& p,
                         auto&& func) {
    visit_continuations(p.handle_, CE_FWD(func));
  }

  stop_token stoken_;
  continuation_handle<> handle_;
};

template<typename T>
struct _return_function_base {
  void return_value(auto&& value) noexcept {
    value_.template emplace<1>(CE_FWD(value));
  }

  decltype(auto) result() {
    if (this->value_.index() == 2) {
      std::rethrow_exception(std::get<2>(this->value_));
    }
    return std::get<1>(this->value_);
  }

  std::variant<std::monostate, T, std::exception_ptr> value_;
};

template<>
struct _return_function_base<void> {
  void return_void() noexcept {
    value_.template emplace<1>();
  }

  void result() {
    if (this->value_.index() == 2) {
      std::rethrow_exception(std::get<2>(this->value_));
    }
  }

  std::variant<std::monostate, std::monostate, std::exception_ptr> value_;
};

template<typename T>
struct promise : public promise_base, _return_function_base<T> {
  using result_type = T;

  void unhandled_exception() noexcept {
    this->value_.template emplace<2>(std::current_exception());
  }

  [[nodiscard]] static inline constexpr auto final_suspend() noexcept {
    struct final_awaiter : final_awaiter_base {
      auto await_suspend(std::coroutine_handle<promise> h) noexcept {
        return h.promise().handle_.handle();
      }
    };

    return final_awaiter{};
  }

  task<T> get_return_object() noexcept {
    return task<T>{std::coroutine_handle<promise>::from_promise(*this)};
  }

  auto await_transform(auto&& value) noexcept(
      tag_nothrow_invocable_v<tag_t<ce::await_transform>, promise&,
                              decltype(value)>)
    requires tag_invocable<tag_t<ce::await_transform>, promise&,
                           decltype(value)>
  {
    // invoke await_transform cpo
    return ::ce::await_transform(*this, CE_FWD(value));
  }
};

}  // namespace ce::_task_impl
