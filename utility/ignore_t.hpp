#pragma once

namespace xlib {
  struct ignore_t {
    template <typename... Args> ignore_t(Args&&...) {}
    template <typename... Args> ignore_t& operator=(Args&&...) { return *this; }
  };
}

