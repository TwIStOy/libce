// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <chrono>
#include <concepts>
#include <coroutine>
#include <variant>

#include <ce/config.h>
#include <ce/base/cppfeature.hh>
#include <ce/impls/event_deleter.hh>
#include <ce/impls/event_loop.hh>

namespace ce::awaiters {

struct SleepAwaiter {
  SleepAwaiter(EventLoop *loop, std::chrono::milliseconds duration) noexcept
      : loop_(loop), duration_(std::move(duration)) {
  }

  SleepAwaiter(EventLoop *loop, const timeval *tv) noexcept
      : loop_(loop), duration_(tv) {
  }

  bool await_ready() const noexcept {
    return false;
  }

  void await_suspend(std::coroutine_handle<> co) noexcept {
    ev_ = loop_->new_event(-1, EV_READ, &SleepAwaiter::_event_callback, this);

    std::visit(
        [ev = ev_.get()](const auto& duration) {
          if constexpr (std::same_as<CE_TYPEOF(duration),
                                     std::chrono::milliseconds>) {
            timeval t;
            t.tv_sec  = duration.count() / 1000;
            t.tv_usec = (duration.count() % 1000) * 1000ull;
            ::evtimer_add(ev, &t);
          } else {
            ::evtimer_add(ev, duration);
          }
        },
        duration_);

    co_ = co;
  }

  void await_resume() noexcept {
  }

 private:
  static void _event_callback(int fd, short events, void *context) {
    (void)fd;
    (void)events;
    auto self = static_cast<SleepAwaiter *>(context);
    self->co_.resume();
  }

 private:
  EventLoop *loop_;
  std::variant<std::chrono::milliseconds, const timeval *> duration_;
  event_util::EventPtr ev_;
  std::coroutine_handle<> co_;
};

inline CE_ALWAYS_INLINE auto async_sleep(EventLoop *loop,
                                         std::chrono::milliseconds duration) {
  return SleepAwaiter{loop, duration};
}

inline CE_ALWAYS_INLINE auto async_sleep_common(
    EventLoop *loop, std::chrono::milliseconds duration) {
  timeval t;
  t.tv_sec  = duration.count() / 1000;
  t.tv_usec = (duration.count() % 1000) * 1000ull;

  auto tv = ::event_base_init_common_timeout(loop->get_native_base(), &t);

  return SleepAwaiter{loop, tv};
}

}  // namespace ce::awaiters
