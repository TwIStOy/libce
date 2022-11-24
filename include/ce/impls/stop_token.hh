// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <atomic>
#include <optional>
#include <thread>

#include <ce/fwd/stop_token_fwd.hh>
#include <ce/impls/cppfeature.hh>

namespace ce {

class stop_callback_base {
 public:
  void execute() noexcept {
    (this->execute_)(this);
  }

 protected:
  friend class stop_source;

  using execute_fn = void(stop_callback_base *cb) noexcept;

  inline stop_callback_base(stop_source *source, execute_fn *execute) noexcept
      : source_(source), execute_(execute) {
  }

  inline void register_callback() noexcept;

  stop_source *source_;
  execute_fn *execute_;
  stop_callback_base *next_      = nullptr;
  stop_callback_base **prev_ptr_ = nullptr;
  bool *removed_during_callback_ = nullptr;
  std::atomic<bool> callback_completed_{false};
};

class stop_source {
 public:
  stop_source() noexcept = default;

  ~stop_source();

  stop_source(const stop_source&)            = delete;
  stop_source(stop_source&&)                 = delete;
  stop_source& operator=(stop_source&&)      = delete;
  stop_source& operator=(const stop_source&) = delete;

  bool request_stop() noexcept;

  stop_token get_token() noexcept;

  inline bool stop_requested() const noexcept {
    return (state_.load(std::memory_order_acquire) & kStopRequestedFlag) != 0;
  }

 private:
  friend class stop_callback_base;
  template<typename F>
  friend class stop_callback;

  uint8_t lock() noexcept;

  void unlock(uint8_t v) noexcept;

  bool try_lock_unless_stop_requested(bool setStopRequested) noexcept;

  bool try_add_callback(stop_callback_base *callback) noexcept;

  void remove_callback(stop_callback_base *callback) noexcept;

  static constexpr uint8_t kStopRequestedFlag = 1;
  static constexpr uint8_t kLockedFlag        = 2;

  std::atomic<uint8_t> state_{0};
  stop_callback_base *callbacks_ = nullptr;
  std::thread::id notifying_thread_id_;
};

inline void stop_callback_base::register_callback() noexcept {
  if (source_ != nullptr) {
    if (!source_->try_add_callback(this)) {
      source_ = nullptr;
      // Callback not registered because stop_requested() was true.
      // Execute inline here.
      execute();
    }
  }
}

class stop_token {
 public:
  template<typename F>
  using callback_type = stop_callback<F>;

  inline stop_token() noexcept : source_(nullptr) {
  }

  inline stop_token(const stop_token& other) noexcept = default;

  inline stop_token(stop_token&& other) noexcept
      : source_(std::exchange(other.source_, {})) {
  }

  inline stop_token& operator=(const stop_token& other) noexcept = default;

  inline stop_token& operator=(stop_token&& other) noexcept {
    source_ = std::exchange(other.source_, nullptr);
    return *this;
  }

  inline bool stop_requested() const noexcept {
    return source_ != nullptr && source_->stop_requested();
  }

  inline bool stop_possible() const noexcept {
    return source_ != nullptr;
  }

  inline void swap(stop_token& other) noexcept {
    std::swap(source_, other.source_);
  }

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

inline stop_token stop_source::get_token() noexcept {
  return stop_token{this};
}

template<typename F>
class stop_callback final : private stop_callback_base {
 public:
  inline stop_callback(stop_token token, auto&& func)
    requires std::convertible_to<CE_TYPEOF(func), F>
      : stop_callback_base(token.source_, &stop_callback::execute_impl),
        func_(CE_FWD(func)) {
    this->register_callback();
  }

  inline ~stop_callback() {
    if (source_ != nullptr) {
      source_->remove_callback(this);
    }
  }

 private:
  friend class stop_source;

  inline static void execute_impl(stop_callback_base *cb) noexcept {
    auto& self = *static_cast<stop_callback *>(cb);
    self.func_();
  }

  [[no_unique_address]] F func_;
};

struct forward_stop_request_to_inplace_stop_source {
  stop_source& source;

  explicit forward_stop_request_to_inplace_stop_source(stop_source& s) noexcept
      : source(s) {
  }

  void operator()() const noexcept {
    source.request_stop();
  }
};

template<typename StopToken>
class stop_token_adapter {
 public:
  stop_token subscribe(StopToken stoken) noexcept {
    const bool stop_possible = stoken.stop_possible();
    callback_.emplace(std::move(stoken),
                      forward_stop_request_to_inplace_stop_source{source_});
    return stop_possible ? source_.get_token() : stop_token{};
  }

  void unsubscribe() noexcept {
    callback_.reset();
  }

 private:
  using stop_callback =
      stop_callback<forward_stop_request_to_inplace_stop_source>;

  stop_source source_;
  [[no_unique_address]] std::optional<stop_callback> callback_;
};

template<>
class stop_token_adapter<stop_token> {
 public:
  stop_token subscribe(stop_token stoken) noexcept {
    return stoken;
  }

  void unsubscribe() noexcept {
  }
};

template<typename StopToken>
struct stop_token_adapter_subscription {
  stop_token subscribe(StopToken stoken) noexcept {
    isSubscribed_ = true;
    return adapter_.subscribe(std::move(stoken));
  }

  void unsubscribe() noexcept {
    if (isSubscribed_) {
      isSubscribed_ = false;
      adapter_.unsubscribe();
    }
  }

  ~stop_token_adapter_subscription() {
    unsubscribe();
  }

 private:
  bool isSubscribed_ = false;
  [[no_unique_address]] stop_token_adapter<StopToken> adapter_;
};

}  // namespace ce
