// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

namespace ce {

namespace _has_operator_co_await_impl {

template<typename T>
concept has_member_operaor_co_await = requires(T obj) {
                                        { obj.operator co_await() };
                                      };

template<typename T>
concept has_free_operaor_co_await = requires(T obj) {
                                      { operator co_await(obj) };
                                    };

}  // namespace _has_operator_co_await_impl

template<typename T>
concept has_operator_co_await =
    _has_operator_co_await_impl::has_member_operaor_co_await<T> ||
    _has_operator_co_await_impl::has_free_operaor_co_await<T>;

}  // namespace ce
