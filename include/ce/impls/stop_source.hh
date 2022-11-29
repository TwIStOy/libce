// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <thread>

#include <ce/config.h>
#include <ce/fwd/stop_token_fwd.hh>
#include <ce/impls/stop_callback_base.hh>

namespace ce {

class stop_source {
 public:
  stop_source() noexcept = default;

  ~stop_source();

  stop_source(const stop_source&)            = delete;
  stop_source(stop_source&&)                 = delete;
  stop_source& operator=(stop_source&&)      = delete;
  stop_source& operator=(const stop_source&) = delete;

  /*
   * request stop, invoke all callbacks
   */
  bool request_stop() noexcept;

  stop_token get_token() noexcept;

  bool stop_requested() const noexcept;

 private:
  friend class stop_callback_base;
  template<typename F>
  friend class stop_callback;

  uint8_t lock() noexcept;

  void unlock(uint8_t v) noexcept;

  bool try_lock_unless_stop_requested(bool _request_stop) noexcept;

  bool try_add_callback(stop_callback_base *callback) noexcept;

  void remove_callback(stop_callback_base *callback) noexcept;

  std::atomic<uint8_t> state_{0};
  stop_callback_base *head_ = nullptr;
  std::thread::id invoking_request_stop_thread_id_;
};

struct stop_source_functor {
  stop_source_functor(stop_source *s) noexcept : s_(s) {
  }

  inline CE_ALWAYS_INLINE void operator()() const noexcept {
    s_->request_stop();
  }

  stop_source *s_;
};

}  // namespace ce
