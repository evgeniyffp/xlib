#pragma once

namespace xlib {
  template <bool b, int i = 32>
  struct function_settings {
    static const bool is_enable_SO = b;
    static const int max_bytes_to_SO = i;
  };

  template <>
  struct function_settings<false> {
    static const bool is_enable_SO = false;
    static const int max_bytes_to_SO = 0;
  };

  template <typename, typename = function_settings<true>>
  class function;

  template <typename R, typename... Args, typename S>
  class function<R(Args...), S> {
  private:
    struct Base {
      int size;

      Base(int size) : size(size) {}
      virtual ~Base() = default;
      virtual R invoke(Args... args) = 0;
    };

    template <typename F>
    struct Derived : Base {
      F f;

      R invoke(Args... args) override {
        return std::invoke(f, std::forward<Args>(args)...);
      }

      template <typename _F = F>
      Derived(_F&& f) : Base(sizeof(_F)), f(std::forward<_F>(f)) {}

      ~Derived() override = default;
    };

/*
    // with virtual functions
    Base* ptr = nullptr;
    char buffer[S::max_bytes_to_SO];
*/
    // with static functions
    void* ptr;

    template <typename F>
    static R invoker(F* f, Args... args) {
      return std::invoke(*f, std::forward<Args>(args)...);
    }
    using invoker_t = R (*)(void*, Args...);
    invoker_t invoker_f = nullptr;

    template <typename F>
    static void deleter(F* f) {
      delete f;
    }
    using deleter_t = void (*)(void*);
    deleter_t deleter_f = nullptr;

  public:
    function() = default;

    template <typename F>
    function(F&& f) {
      this->operator=(std::forward<F>(f));
    }

    ~function() {
      clear();
    }

    void clear() {
    /*
      if (ptr != nullptr) {
        if (reinterpret_cast<char*>(ptr) == buffer)
          ptr->~Base();
        else
          delete ptr;

        ptr = nullptr;
      }
    */
      if (deleter_f != nullptr)
        deleter_f(ptr);
    }

    template <typename F>
    function& operator=(F&& f) {
      clear();
/*
      if (sizeof(F) <= S::max_bytes_to_SO) {
        ptr = reinterpret_cast<Derived<F>*>(buffer); // ?
        new (ptr) Derived<F>(std::forward<F>(f));
      }
      else {
        ptr = new Derived<F>(std::forward<F>(f));
      }
*/
      ptr = new F(std::forward<F>(f));
      invoker_f = reinterpret_cast<invoker_t>(&invoker<F>);
      deleter_f = reinterpret_cast<deleter_t>(&deleter<F>);

      return *this;
    }

    R operator()(Args... args) {
    /*
      return ptr->invoke(std::forward<Args>(args)...);
    */
      return invoker_f(ptr, std::forward<Args>(args)...);
    }
  };
}

