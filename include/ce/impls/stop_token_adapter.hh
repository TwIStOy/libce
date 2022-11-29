// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <ce/impls/stop_callback.hh>
#include <ce/impls/stop_source.hh>
#include <ce/impls/stop_token.hh>
#include <optional>

namespace ce {

template<typename S>
struct stop_token_adapter {
  explicit stop_token_adapter(stop_source *source) : source_(source) {
  }

  stop_token from(S token) {
    const bool stop_possible = token.stop_possible();
    callback_.emplace(std::move(token), stop_source_functor(source_));
    return stop_possible ? source_->get_token() : stop_token{};
  }

  void unsubscribe() noexcept {
    callback_.reset();
  }

 private:
  using callback_t = stop_callback<stop_source_functor>;

  stop_source *source_;
  [[no_unique_address]] std::optional<callback_t> callback_;
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
