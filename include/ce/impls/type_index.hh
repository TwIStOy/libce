// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <typeindex>

namespace ce {

struct type_index_helper {
  virtual std::type_index get_type_index() = 0;
};

template<typename T>
struct type_index_helper_impl : public type_index_helper {
  inline std::type_index get_type_index() final {
    return std::type_index(typeid(T));
  }
};

}  // namespace ce
