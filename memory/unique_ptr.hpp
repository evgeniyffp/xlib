#pragma once

#include <utility> // std::forward

#include "./default_delete.hpp"

#include <memory> // TODO

namespace xlib::memory {
  template <typename T, class Deleter = default_delete<T>>
  using unique_ptr = std::unique_ptr<T, Deleter>; // TODO
//  class unique_ptr {};
  
  template <typename T, typename... Args>
  unique_ptr<T> make_unique(Args&&... args) {
    return { std::forward<Args>(args)... };
  }
}

