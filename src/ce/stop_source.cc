// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#include <ce/impls/stop_source.hh>

#include <limits>
#include <thread>

#include <ce/base/assert.hh>
#include <ce/impls/stop_token.hh>

namespace ce {

static constexpr uint8_t kStopRequestedMask = 1 << 0;
static constexpr uint8_t kLockedMask        = 1 << 1;

bool stop_source::stop_requested() const noexcept {
  return state_.load(std::memory_order_acquire) & kStopRequestedMask;
}

uint8_t stop_source::lock() noexcept {
  auto old_value = state_.load(std::memory_order_relaxed);

  do {
    while ((old_value & kLockedMask) != 0) {
      std::this_thread::yield();
      old_value = state_.load(std::memory_order_relaxed);
    }
  } while (!state_.compare_exchange_weak(old_value, old_value | kLockedMask,
                                         std::memory_order_acquire,
                                         std::memory_order_relaxed));

  return old_value;
}

void stop_source::unlock(uint8_t v) noexcept {
  CE_ASSERT((v & kLockedMask) == 0);
  state_.store(v, std::memory_order_release);
}

bool stop_source::try_lock_unless_stop_requested(bool _request_stop) noexcept {
  uint8_t old_value = state_.load(std::memory_order_relaxed);
  uint8_t new_value =
      _request_stop ? (kLockedMask | kStopRequestedMask) : kLockedMask;

  do {
    while (true) {
      if ((old_value & kStopRequestedMask) != 0) {
        // Stop already requested.
        return false;
      } else if (old_value == 0) {  // not locked and not stop_requested
        break;
      } else {
        std::this_thread::yield();
        old_value = state_.load(std::memory_order_relaxed);
      }
    }
  } while (!state_.compare_exchange_weak(old_value, new_value,
                                         std::memory_order_acq_rel,
                                         std::memory_order_relaxed));

  return true;
}

bool stop_source::try_add_callback(stop_callback_base *callback) noexcept {
  if (!try_lock_unless_stop_requested(false)) {
    // already stop requested, add callback failed
    return false;
  }

  callback->next_           = head_;
  head_->prev_              = callback;
  head_                     = callback;
  callback->in_linked_list_ = true;

  unlock(0);  // old_value must be 0

  return true;
}

bool stop_source::request_stop() noexcept {
  if (!try_lock_unless_stop_requested(true)) {
    // already stop requested
    return true;
  }

  // record current thread
  invoking_request_stop_thread_id_ = std::this_thread::get_id();

  // invoke all callbacks
  while (head_ != nullptr) {
    auto current = head_;

    // already executed
    current->in_linked_list_ = false;

    // fix head
    head_        = head_->next_;
    head_->prev_ = nullptr;

    unlock(kStopRequestedMask);

    bool removed_during_callback      = false;
    current->removed_during_callback_ = &removed_during_callback;

    current->do_callback();

    if (!removed_during_callback) {
      current->removed_during_callback_ = nullptr;
      current->callback_completed_.store(true, std::memory_order_release);
    }

    lock();
  }

  unlock(kStopRequestedMask);

  return false;
}

void stop_source::remove_callback(stop_callback_base *callback) noexcept {
  auto old_value = lock();

  if (callback->in_linked_list_) {
    // *callback->prev_ptr_ = callback->next_;
    if (callback->next_ != nullptr) {
      callback->next_->prev_ = callback->prev_;
    }
    if (callback->prev_ != nullptr) {
      callback->prev_->next_ = callback->next_;
    }

    callback->next_ = callback->prev_ = nullptr;
    callback->in_linked_list_         = false;

    unlock(old_value);
  } else {
    auto tid = invoking_request_stop_thread_id_;
    unlock(old_value);

    if (std::this_thread::get_id() == invoking_request_stop_thread_id_) {
      // callback is executing on this thread.
      if (callback->removed_during_callback_ != nullptr) {
        *callback->removed_during_callback_ = true;
      }
    } else {
      // callback is executing on another thread.
      // Wait until the other thread finishes executing the callback.
      while (!callback->callback_completed_.load(std::memory_order_acquire)) {
        std::this_thread::yield();
      }
    }
  }
}

stop_token stop_source::get_token() noexcept {
  return stop_token{this};
}

stop_source::~stop_source() {
  CE_ASSERT((state_.load(std::memory_order_relaxed) & kLockedMask) == 0);

  for (auto *cbv = head_; cbv != nullptr; cbv = cbv->next_) {
    fprintf(stderr, "dangling stop_callback: %p\n", cbv);
    fflush(stderr);
  }

  CE_ASSERT(head_ == nullptr);
}

}  // namespace ce
