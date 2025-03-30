#pragma once

#include <type_traits>

namespace xlib {
  template <bool V>
  struct thread_safety : std::bool_constant<V> {};
}

