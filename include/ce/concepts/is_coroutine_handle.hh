// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <coroutine>
#include <type_traits>

namespace ce::concepts {

template<typename T>
struct is_coroutine_handle : std::false_type {};

template<typename T>
struct is_coroutine_handle<std::coroutine_handle<T>> : std::true_type {};

template<typename T>
inline constexpr bool is_coroutine_handle_v = is_coroutine_handle<T>::value;

}  // namespace ce::concepts
