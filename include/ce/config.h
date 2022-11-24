// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <cstdint>
#include <type_traits>

// TODO(hawtian): impl

#define CE_RUNTIME_ASSERTION 1
#define CE_ALWAYS_INLINE
#define CE_FRAME_INFO

#ifdef CE_FRAME_INFO
#include <source_location>
#define CE_LOC_PARAM , std::source_location _source_loc
#define CE_LOC_ARG   _source_loc
#define CE_LOC_INIT  , source_loc_(_source_loc)
#define CE_LOC_GET_FRAME                                                \
  inline CE_ALWAYS_INLINE std::source_location get_frame_info() const { \
    return source_loc_;                                                 \
  }
#define CE_LOC_FIELD std::source_location source_loc_;
#else
#define CE_LOC_PARAM
#define CE_LOC_ARG
#define CE_LOC_GET_FRAME
#define CE_LOC_FIELD
#define CE_LOC_INIT
#endif

namespace ce {

using handle_id_t = uint64_t;

}  // namespace ce
