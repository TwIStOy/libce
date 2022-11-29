// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#include <ce/impls/stop_callback_base.hh>
#include <ce/impls/stop_source.hh>
#include <ce/impls/stop_token.hh>

namespace ce {

stop_callback_base::stop_callback_base(stop_source *source,
                                       callback_fn *callback) noexcept
    : source_(source), callback_(callback) {
}

void stop_callback_base::do_callback() noexcept {
  (this->callback_)(this);
}

void stop_callback_base::register_callback() noexcept {
  if (source_ != nullptr) {
    if (!source_->try_add_callback(this)) {
      source_ = nullptr;

      // stop has been requested
      // callback should be invoked immediately
      do_callback();
    }
  }
}

}  // namespace ce
