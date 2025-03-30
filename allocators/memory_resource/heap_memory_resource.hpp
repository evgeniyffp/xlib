#pragma once

#include <memory_resource>

namespace xlib {
  class heap_memory_resource : public std::pmr::memory_resource {
    void* do_allocate(std::size_t bytes, std::size_t aligment) override {
      return ::operator new(bytes, static_cast<std::align_val_t>(aligment));
    }

    void do_deallocate(void* ptr, std::size_t, std::size_t aligment) override {
      ::operator delete(ptr, static_cast<std::align_val_t>(aligment));
    }

    bool do_is_equal(const std::pmr::memory_resource&) const noexcept override {
      return true;
    }
  };
}
