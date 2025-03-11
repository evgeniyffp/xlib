#pragma once

namespace xlib::memory {
  template <typename T>
  struct default_delete {
    constexpr default_delete() = default;

    template <typename U>
    constexpr default_delete(const default_delete<U>&) {}

    void operator()(T* ptr) {
      delete ptr;
    }
  };

  template <typename T>
  struct default_delete<T[]> {
    constexpr default_delete() = default;

    template <typename U>
    constexpr default_delete(const default_delete<U[]>&) {}

    template <typename U>
    void operator()(U* ptr) {
      delete[] ptr;
    }
  };
}
