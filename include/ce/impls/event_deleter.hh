// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <event.h>

#include <memory>

namespace ce::event_util {

struct EventDeleter {
  void operator()(void *p) const noexcept {
    auto e = reinterpret_cast<struct ::event *>(p);
    ::event_del(e);
    ::event_free(e);
  }
};

using EventPtr = std::unique_ptr<::event, EventDeleter>;

}  // namespace ce::event_util
