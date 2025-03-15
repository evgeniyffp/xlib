#pragma once

#include "./shared_ptr.hpp"
#include "../utility/private_tag.hpp"

namespace xlib::memory {
  template <typename T>
  class weak_ptr {
  private:
    typename shared_ptr<T>::ControlBlock* block;

    template <typename U>
    friend class shared_ptr;

    void clear() {
      if (block == nullptr)
        return;

      --*block->weak_count;
      block->try_clear();
    }

  public:
    weak_ptr() : block(nullptr) {}

    weak_ptr(const shared_ptr<T>& other) : block(other.block) {
      ++*block->weak_count;
    }

    ~weak_ptr() {
      clear();
    }

    weak_ptr& operator=(const shared_ptr<T>& other) {
      clear();
      block = other.block;
      ++*block->weak_count;
      return *this;
    }

    shared_ptr<T> lock() {
      return expired() ? shared_ptr<T>() : shared_ptr<T>(*this);
    }

    std::size_t use_count() {
      return *block->shared_count;
    }

    bool expired() {
      return use_count() == 0;
    }
  };
}

