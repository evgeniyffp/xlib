#pragma once

#include "./cl_wrapper.hpp"

namespace xlib::cl {
  template <typename T> class buffer {
  private:
    size_t size = 0;
    cl_mem memory;
    cl_mem_flags flags;

  public:
    buffer(context& context, size_t size, cl_mem_flags flags = CL_MEM_READ_ONLY)
        : size(size), flags(flag) {
      memory = clCreateBuffer(context._context, flags, sizeof(T) * size, NULL, NULL);
    }

    void write(queue& queue, const std::vector<T>& v) {
      if (v.size() != size)
        throw std::runtime_error("xlib::cl::buffer<T>::write(v): v.size() != size\n");

      clEnqueueWriteBuffer(queue._queue, memory, CL_TRUE, 0, sizeof(T) * ARRAY_SIZE, v.data(), 0, NULL, NULL);
    }

    ~buffer {
      clReleaseMemObject(memory);
    }
  };
}
