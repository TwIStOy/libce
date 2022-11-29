// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <utility>

#define CE_FWD(v)    std::forward<decltype(v)>(v)
#define CE_TYPEOF(v) std::remove_cvref_t<decltype(v)>

// TODO(hawtian): msvc
#define CE_COLD [[gnu::cold]]

namespace ce {

// a RAII scope guard struct
template<typename Func>
struct scope_guard {
  scope_guard(Func&& fn) : fn_(CE_FWD(fn)) {
  }

  ~scope_guard() {
    fn_();
  }

  [[no_unique_address]] Func fn_;
};

}  // namespace ce
