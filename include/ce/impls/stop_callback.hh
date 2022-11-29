// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <ce/impls/stop_callback_base.hh>

#include <ce/base/cppfeature.hh>
#include <ce/impls/stop_source.hh>
#include <ce/impls/stop_token.hh>

namespace ce {

template<typename F>
class stop_callback final : private stop_callback_base {
 public:
  inline stop_callback(stop_token token, F&& func)
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

}  // namespace ce
