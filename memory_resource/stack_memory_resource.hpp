#pragma once

#include <memory_resource>

namespace xlib {
  template <std::size_t N = 1024 * 1034>
  class stack_memory_resource : public std::pmr::memory_resource {
  private:
    char buffer[N];
    char* offset = buffer;

    void* do_allocate(std::size_t bytes, std::size_t aligment) override {
      std::cout << "stack_memory_resource::do_allocate(): bytes = " << bytes << std::endl;

      if (reinterpret_cast<std::size_t>(offset) % aligment != 0) {
        offset += aligment - reinterpret_cast<std::size_t>(offset) % aligment;
      }

      char* ptr = offset;

      offset += bytes;

      if (offset >= buffer + N)
        throw std::bad_alloc();

      return ptr;
    }

    void do_deallocate(void*, std::size_t, std::size_t) override {}

    bool do_is_equal(const std::pmr::memory_resource&) const noexcept override {
      return true;
    }

  public:

  };
}
