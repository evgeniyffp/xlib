#pragma once

#include <type_traits>
#include <cstddef>

namespace xlib {
  template <bool b, int i = 16>
  struct function_settings {
    static const bool is_enable_SOO = b;
    static const int max_bytes_to_SOO = i;
  };

  template <>
  struct function_settings<false> {
    static const bool is_enable_SOO = false;
    static const int max_bytes_to_SOO = 0;
  };

  template <typename, typename = function_settings<true>>
  class function;

  template <typename R, typename... Args, typename S>
  class function<R(Args...), S> {
  private:
    enum class types_of_actions {
      invoke, clear, copy, move
    };

    union maybe_value {
      char buffer[sizeof(R)];
      R value;

      template <typename _R = R>
      maybe_value(_R&& value) : value(std::forward<_R>(value)) {}

      maybe_value() = default;
    };

    template <typename F>
    static maybe_value vtable(types_of_actions type, F* f, F* other, Args... args) {
      switch (type) {
        case types_of_actions::invoke:
          return maybe_value(std::invoke(*f, std::forward<Args>(args)...));

        case types_of_actions::clear:
          if constexpr (sizeof(F) <= S::max_bytes_to_SOO) {
            f->~F();
          }
          else {
            delete f;
          }
          return {};

        case types_of_actions::copy:
          new (other) F(*f);
          return {};

        case types_of_actions::move:
          new (other) F(std::move(*f));
          return {};
      }
    }
    using vtable_t = maybe_value (*)(types_of_actions, void*, void*, Args... args);
    vtable_t vtable_f = nullptr;
    using vtable_without_args_t = maybe_value (*)(types_of_actions, void*, void*);
    #define vtable_without_args_f (reinterpret_cast<vtable_without_args_t>(vtable_f))

    void* ptr;
    alignas(std::max_align_t) char buffer[S::max_bytes_to_SOO];

  public:
    function() = default;

    template <typename F>
    function(F&& f) {
      this->operator=(std::forward<F>(f));
    }

    function(const function& other) {
      this->operator=(other);
    }

    function(function&& other) {
      vtable_f = other.vtable_f;
      vtable_without_args_f(types_of_actions::move, ptr, &other);
    }

    ~function() {
      clear();
    }

    void clear() {
      if (vtable_f != nullptr)
        vtable_without_args_f(types_of_actions::clear, ptr, nullptr);
    }

    template <typename F>
    requires (!std::is_same_v<function, std::decay_t<F>>)
    function& operator=(F&& f) {
      clear();

      if constexpr (sizeof(F) <= S::max_bytes_to_SOO) {
        ptr = buffer;
        new (ptr) F(std::forward<F>(f));
      }
      else {
        ptr = new F(std::forward<F>(f));
      }

      vtable_f = reinterpret_cast<vtable_t>(&vtable<F>);

      return *this;
    }

    function& operator=(const function& other) {
      clear();

      vtable_f = other.vtable_f;
      vtable_without_args_f(types_of_actions::copy, ptr, other.ptr);

      return *this;
    }

    R operator()(Args... args) const {
#define call() (vtable_f(types_of_actions::invoke, ptr, nullptr, std::forward<Args>(args)...))
      if constexpr (std::is_same_v<R, void>)
        call();
      else
        return call().value;
    }
  };
}

