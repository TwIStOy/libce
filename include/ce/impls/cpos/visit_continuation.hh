// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <ce/base/tag_invoke.hh>

namespace ce {

namespace _visit_continuation_cpo {

struct _fn {
  template<typename Continuation, typename Func>
    requires tag_invocable<_fn, const Continuation&, Func&&>
  void operator()(const Continuation& c, Func&& func) const
      noexcept(tag_nothrow_invocable_v<_fn, const Continuation&, Func&&>) {
    return tag_invoke(_fn{}, c, CE_FWD(func));
  }

  template<typename Continuation, typename Func>
    requires(!tag_invocable<_fn, const Continuation&, Func &&>)
  void operator()(const Continuation& c, Func&& func) const noexcept {
    // do nothing
  }
};

}  // namespace _visit_continuation_cpo

inline constexpr _visit_continuation_cpo::_fn visit_continuation;

}  // namespace ce
