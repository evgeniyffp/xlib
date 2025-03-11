#pragma once

#include "./default_delete.hpp"

namespace xlib::memory {
  template <typename T, class Deleter = default_delete<T>>
  using unique_ptr = std::unique_ptr<T, Deleter>;
//  class unique_ptr {};
}

