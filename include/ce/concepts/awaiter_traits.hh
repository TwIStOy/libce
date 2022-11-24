// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <concepts>
#include <type_traits>

#include <ce/concepts/has_operator_co_await.hh>

namespace ce {

template<typename T>
  requires has_operator_co_await<T> && requires(T obj) {
                                         {
                                           obj.await_ready()
                                           } -> std::convertible_to<bool>;
                                         { obj.await_resume() };
                                       }
using await_result_t = decltype(std::declval<T>().await_resume());

}  // namespace ce
