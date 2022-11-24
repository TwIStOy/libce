// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <event.h>

#include <cstdint>
#include <sstream>
#include <string>

#include <ce/config.h>
#include <ce/impls/cppfeature.hh>
#include <ce/impls/event_loop.hh>
#include "ce/impls/frame_info.hh"
#include "ce/impls/tag_invoke.hh"

namespace ce {

struct Handle {
 public:
  virtual ~Handle() = default;

  Handle(EventLoop *loop CE_LOC_PARAM);

  handle_id_t get_handle_id() const;

#ifdef CE_FRAME_INFO
  inline CE_ALWAYS_INLINE std::string frame_name() const {
    std::ostringstream oss;
    oss << source_loc_.function_name() << " at " << source_loc_.file_name()
        << ":" << source_loc_.line() << ":" << source_loc_.column();
    return oss.str();
  }

  CE_LOC_GET_FRAME;

  friend std::source_location tag_invoke(tag_t<::ce::get_frame_info>,
                                         const Handle& handle) {
    return handle.source_loc_;
  }
#endif

  virtual void schedule() = 0;
  virtual void cancel()   = 0;

 private:
  EventLoop *loop_;
  handle_id_t handle_id_;

 protected:
  CE_LOC_FIELD;
};

inline CE_ALWAYS_INLINE Handle::Handle(EventLoop *loop CE_LOC_PARAM)
    : loop_(loop) CE_LOC_INIT {
  handle_id_ = loop->get_next_handle_id();
}

inline CE_ALWAYS_INLINE handle_id_t Handle::get_handle_id() const {
  return handle_id_;
}

}  // namespace ce
