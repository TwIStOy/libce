// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <ce/impls/cppfeature.hh>
#include <ce/impls/tag_invoke.hh>

namespace ce {

namespace _get_frame_info_cpo {

struct _fn {
  constexpr auto operator()(auto&&...args) const noexcept {
    return tag_invoke(_fn{}, CE_FWD(args)...);
  }
};

}  // namespace _get_frame_info_cpo

inline constexpr _get_frame_info_cpo::_fn get_frame_info;

}  // namespace ce
