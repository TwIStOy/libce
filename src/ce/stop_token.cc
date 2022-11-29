// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#include <ce/impls/stop_token.hh>

#include <cstdio>

#include <ce/base/assert.hh>
#include <thread>

namespace ce {

stop_source::~stop_source() {
  CE_ASSERT((state_.load(std::memory_order_relaxed) & kLockedFlag) == 0);

  for (auto *cbv = callbacks_; cbv != nullptr; cbv = cbv->next_) {
    printf("dangling stop_callback: %p\n", cbv);
    fflush(stdout);
  }

  CE_ASSERT(callbacks_ == nullptr);
}

bool stop_source::request_stop() noexcept {
  if (!try_lock_unless_stop_requested(true)) {
    return true;
  }

  notifying_thread_id_ = std::this_thread::get_id();

  while (callbacks_ != nullptr) {
    auto *current = callbacks_;

    current->prev_ptr_ = nullptr;

    callbacks_ = current->next_;

    if (callbacks_ != nullptr) {
      callbacks_->prev_ptr_ = &callbacks_;
    }

    unlock(kStopRequestedFlag);

    bool removed_during_callback      = false;
    current->removed_during_callback_ = &removed_during_callback;

    current->execute();

    if (!removed_during_callback) {
      current->removed_during_callback_ = nullptr;
      current->callback_completed_.store(true, std::memory_order_release);
    }

    lock();
  }

  unlock(kStopRequestedFlag);

  return false;
}

bool stop_source::try_lock_unless_stop_requested(bool stop) noexcept {
  auto old_value = state_.load(std::memory_order_relaxed);
  auto new_value = stop ? (kLockedFlag | kStopRequestedFlag) : kLockedFlag;

  do {
    while (true) {
      if ((old_value & kStopRequestedFlag) != 0) {
        // Stop already requested.
        return false;
      } else if (old_value == 0) {
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
    return false;
  }

  callback->next_     = callbacks_;
  callback->prev_ptr_ = &callbacks_;
  if (callbacks_ != nullptr) {
    callbacks_->prev_ptr_ = &callback->next_;
  }
  callbacks_ = callback;

  unlock(0);

  return true;
}

void stop_source::remove_callback(stop_callback_base *callback) noexcept {
  auto oldState = lock();

  if (callback->prev_ptr_ != nullptr) {
    // Callback has not been executed yet.
    // Remove from the list.
    *callback->prev_ptr_ = callback->next_;
    if (callback->next_ != nullptr) {
      callback->next_->prev_ptr_ = callback->prev_ptr_;
    }
    unlock(oldState);
  } else {
    auto notifyingThreadId = notifying_thread_id_;
    unlock(oldState);

    if (std::this_thread::get_id() == notifyingThreadId) {
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

uint8_t stop_source::lock() noexcept {
  auto oldState = state_.load(std::memory_order_relaxed);
  do {
    while ((oldState & kLockedFlag) != 0) {
      std::this_thread::yield();
      oldState = state_.load(std::memory_order_relaxed);
    }
  } while (!state_.compare_exchange_weak(oldState, oldState | kLockedFlag,
                                         std::memory_order_acquire,
                                         std::memory_order_relaxed));

  return oldState;
}

void stop_source::unlock(uint8_t v) noexcept {
  state_.store(v, std::memory_order_release);
}

}  // namespace ce
