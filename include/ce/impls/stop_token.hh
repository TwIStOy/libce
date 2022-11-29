// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <atomic>
#include <optional>
#include <thread>

#include <ce/base/cppfeature.hh>
#include <ce/fwd/stop_token_fwd.hh>

namespace ce {

class stop_token {
 public:
  template<typename F>
  using callback_type = stop_callback<F>;

  inline stop_token() noexcept : source_(nullptr) {
  }

  inline stop_token(const stop_token& other) noexcept = default;

  inline stop_token(stop_token&& other) noexcept
      : source_(std::exchange(other.source_, nullptr)) {
  }

  inline stop_token& operator=(const stop_token& other) noexcept = default;

  inline stop_token& operator=(stop_token&& other) noexcept {
    source_ = std::exchange(other.source_, nullptr);
    return *this;
  }

  bool stop_requested() const noexcept;

  bool stop_possible() const noexcept;

  void swap(stop_token& other) noexcept;

  friend inline bool operator==(const stop_token& a,
                                const stop_token& b) noexcept {
    return a.source_ == b.source_;
  }

  friend inline bool operator!=(const stop_token& a,
                                const stop_token& b) noexcept {
    return !(a == b);
  }

 private:
  friend stop_source;
  template<typename F>
  friend class stop_callback;

  inline explicit stop_token(stop_source *source) noexcept : source_(source) {
  }

  stop_source *source_;
};

}  // namespace ce
