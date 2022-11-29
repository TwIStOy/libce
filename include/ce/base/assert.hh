// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <iostream>
#include <source_location>

#include <ce/config.h>
#include <ce/base/cppfeature.hh>

#ifdef CE_RUNTIME_ASSERTION

namespace ce::_assert_impl {

CE_COLD inline auto runtime_assert_failed_cold(auto&& failed_fn) -> auto&& {
  return CE_FWD(failed_fn);
}

inline void report_error_and_terminate(const char *prefix, const char *msg,
                                       std::source_location where) {
  std::cerr << "<" << prefix << "> " << where.file_name() << ":" << where.line()
            << "] " << where.function_name() << ": " << msg << std::endl;
  std::terminate();
}

inline decltype(auto) expect(const char *prefix, const char *statement,
                             auto&& value, std::source_location where) {
  if (!value) [[unlikely]] {
    report_error_and_terminate(prefix, statement, where);
  }
  return CE_FWD(value);
}

#define CE_ASSERT(...)                                                  \
  ::ce::_assert_impl::expect("CE-ASSERT", "" #__VA_ARGS__, __VA_ARGS__, \
                             std::source_location::current())

}  // namespace ce::_assert_impl

#else

#define CE_ASSERT(...)

#endif
