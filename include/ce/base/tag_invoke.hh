// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <concepts>
#include <type_traits>
#include <utility>

#include <ce/config.h>
#include <ce/base/cppfeature.hh>

namespace ce::_tag_invoke_cpo {

void tag_invoke();

struct _fn {
  template<typename CPO, typename... Args,
           bool noexcept_ = noexcept(tag_invoke(std::declval<CPO>(),
                                                std::declval<Args>()...))>
  constexpr auto operator()(CPO cpo, Args&&...args) const noexcept(noexcept_) {
    return tag_invoke(std::move(cpo), CE_FWD(args)...);
  }
};

};  // namespace ce::_tag_invoke_cpo

namespace ce {
namespace __tag_invoke_cpo {
inline constexpr _tag_invoke_cpo::_fn tag_invoke;
}  // namespace __tag_invoke_cpo

using namespace __tag_invoke_cpo;

template<auto& CPO>
using tag_t = CE_TYPEOF(CPO);

template<typename CPO, typename... Args>
concept tag_invocable = std::invocable<decltype(tag_invoke), CPO, Args...>;

template<typename CPO, typename... Args>
concept tag_nothrow_invocable =
    std::is_nothrow_invocable_v<decltype(tag_invoke), CPO, Args...>;

template<typename CPO, typename... Args>
inline constexpr bool tag_invocable_v = tag_invocable<CPO, Args...>;

template<typename CPO, typename... Args>
inline constexpr bool tag_nothrow_invocable_v =
    tag_nothrow_invocable<CPO, Args...>;

template<typename CPO, typename... Args>
using tag_invoke_result_t =
    std::invoke_result_t<decltype(tag_invoke), CPO, Args...>;

}  // namespace ce
