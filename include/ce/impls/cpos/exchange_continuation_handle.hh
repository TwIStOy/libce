// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <ce/base/tag_invoke.hh>
#include <ce/impls/continuations.hh>

namespace ce {

namespace _exchange_continuation_cpo {

struct _fn {
  template<typename P>
  continuation_handle<> operator()(
      P& p, continuation_handle<> handle) const noexcept {
    return tag_invoke(_fn{}, p, std::move(handle));
  }
};

}  // namespace _exchange_continuation_cpo

inline constexpr _exchange_continuation_cpo::_fn exchange_continuation;

}  // namespace ce
