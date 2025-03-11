#pragma once

#include "cl_wrapper.hpp"

namespace xlib::cl {
  class context {
  private:
  public: // FOR DEBUG
    cl_context _context;

    friend class command_queue;

  public:
    context(device& d) {
      _context = clCreateContext(NULL, 1, &d.id, NULL, NULL, NULL);
    }

    ~context() {
      clReleaseContext(_context);
    }
  };
}
