// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <ce/concepts/has_operator_co_await.hh>
#include <ce/impls/cppfeature.hh>

namespace ce {

namespace _get_awaiter_cpo {

struct _fn {
  template<typename T>
  decltype(auto) operator()(T&& arg) const noexcept {
    if constexpr (_has_operator_co_await_impl::has_member_operaor_co_await<T>) {
      return CE_FWD(arg).operator co_await();
    } else if constexpr (_has_operator_co_await_impl::has_free_operaor_co_await<
                             T>) {
      return operator co_await(CE_FWD(arg));
    } else {
      return CE_FWD(arg);
    }
  }
};

}  // namespace _get_awaiter_cpo

inline constexpr _get_awaiter_cpo::_fn get_awaiter;

}  // namespace ce
