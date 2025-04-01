#pragma once

#include <memory>
#include <mutex>

#include "../utility/thread_safety.hpp"
#include "../utility/ignore_t.hpp"

namespace xlib {
  template <typename, typename = thread_safety<false>>
  class pool_allocator;

  template <typename T, bool is_thread_safety>
  class pool_allocator<T, thread_safety<is_thread_safety>> {
  private:
    struct pair {
      bool is_used = false;
      T data;
    };

    struct impl_data_t {
      std::vector<pair> pool;
      std::mutex mtx;

      using lock_guard = std::conditional_t<is_thread_safety, std::lock_guard<std::mutex>, ignore_t>;

      impl_data_t(std::size_t size) : pool(size) {}
    };

    using lock_guard = typename impl_data_t::lock_guard;

    std::shared_ptr<impl_data_t> impl_data;

  public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using diffrent_type = std::ptrdiff_t;

    pool_allocator(size_type count) : impl_data(std::make_shared<impl_data_t>(count)) {}
    ~pool_allocator() = default;

    pool_allocator(const pool_allocator&) = default;
    pool_allocator(pool_allocator&&) = default;

    pool_allocator& operator=(const pool_allocator&) = delete;
    pool_allocator& operator=(pool_allocator&&) = delete;

    pointer allocate() {
      lock_guard l(impl_data->mtx);

      for (size_t i = 0; i < impl_data->pool.size(); ++i) {
        if (!impl_data->pool[i].is_used) {
          impl_data->pool[i].is_used = true;
          return &impl_data->pool[i].data;
        }
      }

      return nullptr;
    }

    void deallocate(pointer ptr) {
      lock_guard l(impl_data->mtx);

      diffrent_type index =
          reinterpret_cast<char*>(ptr) - reinterpret_cast<char*>(&(impl_data->pool.data()->data));
      index /= sizeof(value_type);

      impl_data->pool[index].is_used = false;
    }

    template <typename U>
    struct rebind {
      using type = pool_allocator<U, thread_safety<is_thread_safety>>;
    };
  };
}
