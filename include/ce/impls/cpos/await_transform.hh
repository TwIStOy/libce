// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <ce/base/cppfeature.hh>
#include <ce/base/tag_invoke.hh>

namespace ce {

namespace _await_transform_cpo {

struct _fn {
  // proxy impl
  decltype(auto) operator()(auto& promise, auto&& value) const
      noexcept(tag_nothrow_invocable_v<_fn, decltype(promise), decltype(value)>)
    requires tag_invocable<_fn, decltype(promise), decltype(value)>
  {
    return tag_invoke(_fn{}, promise, CE_FWD(value));
  }

  // default impl
  decltype(auto) operator()(auto& promise, auto&& value) const
      noexcept(noexcept(promise.await_transform(CE_FWD(value))))
    requires(!tag_invocable<_fn, decltype(promise), decltype(value)>)
  {
    // FIXME(hawtian): maybe `promise.await_transform(CE_FWD(value))`?
    return CE_FWD(value);
  }
};

}  // namespace _await_transform_cpo

inline constexpr _await_transform_cpo::_fn await_transform;

}  // namespace ce
