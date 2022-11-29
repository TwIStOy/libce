// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <stop_token>

#include <ce/base/tag_invoke.hh>

namespace ce {

namespace _get_stop_token_cpo {

struct _fn {
  template<typename P>
  std::stop_token operator()(const P& p) const noexcept {
    return tag_invoke(_fn{}, p);
  }
};

}  // namespace _get_stop_token_cpo

inline constexpr _get_stop_token_cpo::_fn get_stop_token;

}  // namespace ce
