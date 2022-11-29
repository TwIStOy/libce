// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <concepts>
#include <coroutine>
#include <type_traits>

#include <ce/concepts/has_operator_co_await.hh>

namespace ce::concepts {

template<typename T>
concept awaitable = has_operator_co_await<T>;

template<typename T>
concept awaiter = requires(T t) {
                    { t.await_ready() } -> std::convertible_to<bool>;
                    { t.await_suspend(std::coroutine_handle<>{}) };
                    { t.await_resume() };
                  };

template<awaiter T>
using awaiter_result_t = decltype(std::declval<T>().await_resume());

}  // namespace ce::concepts
