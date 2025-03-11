#pragma once

#include "cl_wrapper.hpp"

namespace xlib::cl {
  class command_queue {
  private:
  public: // FOR DEBUG
    cl_command_queue _queue;

  public:
    command_queue(context& context, device& device) {
      _queue = clCreateCommandQueue(context._context, device.id, 0, NULL);
    }

    ~command_queue() {
      clReleaseCommandQueue(_queue);
    }
  };
}
