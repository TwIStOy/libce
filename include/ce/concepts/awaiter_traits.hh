// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <concepts>
#include <coroutine>
#include <type_traits>

#include <ce/base/cppfeature.hh>
#include <ce/concepts/has_operator_co_await.hh>

namespace ce::concepts {

namespace _get_awaiter_cpo {

struct _fn {
  decltype(auto) operator()(auto&& t) const {
    if constexpr (_has_operator_co_await_impl::has_member_operaor_co_await<
                      CE_TYPEOF(t)>) {
      return CE_FWD(t).operator co_await();
    } else if constexpr (_has_operator_co_await_impl::has_free_operaor_co_await<
                             CE_TYPEOF(t)>) {
      return operator co_await(CE_FWD(t));
    } else {
      return CE_FWD(t);
    }
  }
};

}  // namespace _get_awaiter_cpo

inline constexpr _get_awaiter_cpo::_fn get_awaiter;

template<typename T>
concept awaitable = has_operator_co_await<T>;

template<typename T>
concept awaiter = requires(T t) {
                    { t.await_ready() } -> std::convertible_to<bool>;
                    { t.await_resume() };
                  };

namespace _get_awaitable_cpo {

struct _fn {
  decltype(auto) operator()(auto&& expr) const noexcept {
    if constexpr (awaiter<CE_TYPEOF(expr)>) {
      return CE_FWD(expr);
    } else {
      using task_t    = CE_TYPEOF(get_awaiter(CE_FWD(expr)));
      using promise_t = typename task_t::promise_type;

      return (*reinterpret_cast<promise_t *>(1))
          .await_transform(std::move(expr));
    }
  }
};

}  // namespace _get_awaitable_cpo

inline constexpr _get_awaitable_cpo::_fn get_awaitable;

template<typename T>
struct awaiter_traits {
  using awaitable_type =
      std::remove_cvref_t<decltype(get_awaitable(std::declval<T>()))>;

  using awaiter_type = std::remove_cvref_t<decltype(get_awaiter(
      std::declval<awaitable_type>()))>;

  using awaiter_result_t =
      decltype(std::declval<awaiter_type>().await_resume());
};

}  // namespace ce::concepts
