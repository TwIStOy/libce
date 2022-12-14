// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#pragma once  // NOLINT(build/header_guard)

#include <coroutine>

#include <ce/base/tag_invoke.hh>
#include <ce/fwd/continuations_fwd.hh>
#include <functional>
#include <typeindex>
#include <utility>

namespace ce {

namespace _continuation_handle_impl {

struct continuation_handle_vtable {
  using DoneCallback_t     = std::function<std::coroutine_handle<>(void *)>;
  using PromiseTypeIndex_t = std::function<std::type_index()>;

  DoneCallback_t done_callback_;
  PromiseTypeIndex_t promise_type_index;

  template<typename Promise>
  static continuation_handle_vtable *vtable_for() {
    static continuation_handle_vtable _vtable{
        .done_callback_ = [](void *ptr) -> std::coroutine_handle<> {
          return std::coroutine_handle<Promise>::from_address(ptr)
              .promise()
              .unhandle_done();
        },
        .promise_type_index =
            []() {
              return std::type_index(typeid(Promise));
            },
    };
    return &_vtable;
  }
};

}  // namespace _continuation_handle_impl

template<>
class continuation_handle<void> {
 public:
  continuation_handle() = default;

  template<typename Promise>
    requires(!std::same_as<Promise, void>)
  continuation_handle(std::coroutine_handle<Promise> coro) noexcept
      : handle_(std::move(coro)),
        vtable_(
            _continuation_handle_impl::continuation_handle_vtable::vtable_for<
                Promise>()) {
  }

  explicit operator bool() const noexcept {
    return handle_ != nullptr;
  }

  std::coroutine_handle<> handle() const noexcept {
    return handle_;
  }

  void resume() {
    handle_.resume();
  }

  auto done() const noexcept {
    return vtable_->done_callback_(handle_.address());
  }

 private:
  std::coroutine_handle<> handle_;
  _continuation_handle_impl::continuation_handle_vtable *vtable_;
};

template<typename Promise>
class continuation_handle {
 public:
  continuation_handle() = default;

  continuation_handle(std::coroutine_handle<Promise> coro) noexcept
      : impl_(std::move(coro)) {
  }

  explicit operator bool() const noexcept {
    return !!impl_;
  }

  /*implicit*/ operator continuation_handle<>() const noexcept {
    return impl_;
  }

  std::coroutine_handle<Promise> handle() const noexcept {
    return std::coroutine_handle<Promise>::from_address(
        impl_.handle().address());
  }

  void resume() {
    impl_.resume();
  }

  Promise& promise() const noexcept {
    return handle().promise();
  }

  auto done() const noexcept {
    return impl_.done();
  }

 private:
  continuation_handle<> impl_;
};

}  // namespace ce
