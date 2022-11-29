// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#include <ce/impls/stop_token.hh>

#include <ce/impls/stop_source.hh>

namespace ce {

bool stop_token::stop_requested() const noexcept {
  return source_ != nullptr && source_->stop_requested();
}

inline bool stop_token::stop_possible() const noexcept {
  return source_ != nullptr;
}

auto stop_token::swap(stop_token& other) noexcept -> void {
  std::swap(source_, other.source_);
}

}  // namespace ce
