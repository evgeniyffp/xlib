#pragma once

#include <algorithm>
#include <type_traits>
#include <cstddef>

namespace xlib {

template <typename...>
class variant;



template <typename, typename, size_t = 0>
struct index_at;

template <typename Variant, typename Type, size_t StartIndex = 0>
constexpr size_t index_at_v = index_at<Variant, Type, StartIndex>::value;

template <typename Type, size_t StartIndex>
struct index_at<variant<>, Type, StartIndex>
    : std::integral_constant<size_t, static_cast<size_t>(-1)> {};

template <typename Head, typename... Tail, typename Type, size_t StartIndex>
struct index_at<variant<Head, Tail...>, Type, StartIndex>
    : std::integral_constant<size_t,
      std::conditional_t<
        std::is_same_v<Head, Type>,
        std::integral_constant<size_t, StartIndex>,
        index_at<variant<Tail...>, Type, StartIndex + 1>
      >::value
    > {};


template <typename Variant, typename Type>
class variant_base {
private:

public:
  variant_base() {}

  variant_base(const Type& value) {
    this->operator=(value);
  }

  variant_base(Type&& value) {
    this->operator=(std::move(value));
  }

  variant_base& operator=(const Type& value) {
    auto var = static_cast<Variant*>(this);

    new (var->buffer) Type(value);
    var->current_type = index_at_v<Variant, Type>;

    return *this;
  }

  variant_base& operator=(Type&& value) {
    auto var = static_cast<Variant*>(this);

    new (var->buffer) Type(std::move(value));
    var->current_type = index_at_v<Variant, Type>;

    return *this;
  }

  bool clear() {
    auto var = static_cast<Variant*>(this);

    if (var->current_type != index_at_v<Variant, Type>)
      return false;

    reinterpret_cast<Type*>(var->buffer)->~Type();
    var->current_type = -1;

    return true;
  }
};



template <typename...>
struct max_sizeof;

template <typename... Ts>
constexpr size_t max_sizeof_v = max_sizeof<Ts...>::value;

template <typename Head>
struct max_sizeof<Head>
    : std::integral_constant<std::size_t, sizeof(Head)> {};

template <typename Head, typename... Tail>
struct max_sizeof<Head, Tail...>
    : std::integral_constant<std::size_t, std::max(sizeof(Head), max_sizeof_v<Tail...>)> {};



template <typename... Ts>
class variant : private variant_base<variant<Ts...>, Ts>... {
private:
  alignas(std::max_align_t) char buffer[max_sizeof_v<Ts...>];

  size_t current_type;

  void clear() {
    (this->variant_base<variant<Ts...>, Ts>::clear(), ...);
  }

public:
  using variant_base<variant<Ts...>, Ts>::variant_base...;

  template <typename, typename>
  friend class variant_base;

  ~variant() {
    clear();
  }
};

}
