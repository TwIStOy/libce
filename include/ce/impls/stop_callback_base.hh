// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <atomic>

#include <ce/fwd/stop_token_fwd.hh>

namespace ce {

class stop_callback_base {
 public:
  using callback_fn = void(stop_callback_base *) noexcept;

  void do_callback() noexcept;

 protected:
  friend class stop_source;

  stop_callback_base(stop_source *source, callback_fn *callback) noexcept;

  void register_callback() noexcept;

  stop_source *source_;
  callback_fn *callback_;

  // pointer for linked list
  stop_callback_base *next_ = nullptr;
  stop_callback_base *prev_ = nullptr;

  bool in_linked_list_ = false;

  bool *removed_during_callback_ = nullptr;
  std::atomic<bool> callback_completed_{false};
};

}  // namespace ce
