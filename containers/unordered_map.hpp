#pragma once

#include <functional>
#include <memory>

namespace xlib::container {

template<
    class Key,
    class T,
    class Hash = std::hash<Key>,
    class KeyEqual = std::equal_to<Key>,
    class Allocator = std::allocator<std::pair<const Key, T>>
>
class unordered_map {
public:
    using key_type = Key;
    using mapped_type = T;

    using value_type = std::pair<const key_type, mapped_type>;

private:
    using size_type = size_t;
    using __hash_t = size_t;

    struct __base_value_list_t {
        __hash_t hash;
        __base_value_list_t* next;

        __base_value_list_t(__hash_t hash, __base_value_list_t* next) : hash(hash), next(next) {}
        virtual ~__base_value_list_t() = default;
    };

    struct __value_list_t : __base_value_list_t {
        value_type value;

        __value_list_t(__hash_t hash, __base_value_list_t* next, const value_type& value) : __base_value_list_t(hash, next), value(value) {}
        ~__value_list_t() override = default;
    };

    struct Bucket {
        __base_value_list_t *value_list = nullptr;
    };
    using __bucket_t = Bucket;

    __bucket_t* buckets = nullptr;       // is array
    size_type bucket_count_ = 0;         // is count of buckets
    __base_value_list_t* root = nullptr; // is pointer to list

    [[no_unique_address]] Hash hash_function;
    [[no_unique_address]] KeyEqual key_equal;
    [[no_unique_address]] Allocator allocator;

    size_type size_ = 0;
    float max_load_factor_ = 3.0;

private:
  template <bool is_const>
  class __base_iterator_t {
    friend class unordered_map<Key, T, Hash, KeyEqual, Allocator>;
  private:
    __base_value_list_t* ptr;

    __base_iterator_t(__base_value_list_t* ptr)
        : ptr(ptr) {} 

  public:
    __base_iterator_t& operator++() {
      ptr = ptr->next;
      return *this;
    }

    std::conditional_t<is_const, const value_type&, value_type&> operator*() {
      return reinterpret_cast<__value_list_t*>(ptr)->value;
    }

    bool operator==(__base_iterator_t& other) {
      return ptr == other.ptr;
    }

    bool operator!=(__base_iterator_t& other) {
      return !(*this == other);
    }
  };

public:
  using iterator = __base_iterator_t<false>;
  using const_iterator = __base_iterator_t<true>;

  iterator begin() { return { root->next }; }
  const_iterator begin() const { return { root->next }; }
  const_iterator cbegin() const { return { root->next }; }

  iterator end() { return { root }; }
  const_iterator end() const { return { root }; }
  const_iterator cend() const { return { root }; }

public:
  explicit unordered_map(
          size_type bucket_count_ = 128,
          const Hash &hash_function = {},
          const KeyEqual &key_equal = {},
          const Allocator &allocator = {})
          : buckets(new __bucket_t[bucket_count_])
          , bucket_count_(bucket_count_)
          , root(new __base_value_list_t{ 0, nullptr })
          , hash_function(hash_function)
          , key_equal(key_equal)
          , allocator(allocator) {
    root->next = root;
  }

  ~unordered_map() {
    delete[] buckets;
    auto current = root->next;
    while (current != root) {
      auto tmp = current;
      std::cout << tmp << std::endl;
      current = current->next;
      delete tmp;
    }
    delete root;
  }

  T& insert(const value_type &value) {
    /*
    auto hash = hash_function(value.first);
    auto index = hash % bucket_count_;

    __value_list_t* empty_piece = buckets[index].value_list;

    if (empty_piece != nullptr) {
      while (true) {
        if (key_equal(empty_piece->value.first, key)) {
          break;
        }
        current = current->next;
      }
    }
    
    if (buckets[index].value_list == nullptr) {
      auto* list = new __value_list_t{ hash, root->next, value };
      root->next = buckets[index].value_list = list;
      return reinterpret_cast<__value_list_t*>(root)->value.second;
    }
    else {
      
    }*/
  }

  T& at(const Key &key) {
    auto hash = hash_function(key);
    auto index = hash % bucket_count_;

    if (buckets[index].value_list != nullptr) {
      return reinterpret_cast<__value_list_t*>(buckets[index].value_list)->value.second;
    } else {
      throw std::out_of_range("container doesn\'t have element with this key");
    }
  }

  T& operator[](const Key &key) {
    auto hash = hash_function(key);
    auto index = hash % bucket_count_;

    if (buckets[index].value_list == nullptr) {
      auto* list = new __value_list_t( hash, root->next, { key, {} } );
      root->next = buckets[index].value_list = list;
      // __maybe_rehash();
      return reinterpret_cast<__value_list_t*>(root->next)->value.second;
    }
    else {
      __value_list_t * prev, * current;
      for (
            current = reinterpret_cast<__value_list_t*>(buckets[index].value_list);
            current->hash == hash;
            prev = current, current = reinterpret_cast<__value_list_t*>(current->next))
      {
        if (key_equal(key, current->value.first)) {
          return current->value.second;
        }
      }
//  TODO:
    }
    ++size_;
  }

  void rehash(size_type count) {
    if (bucket_count_ >= count) {
      return;
    }

    delete[] buckets;
    buckets = new __bucket_t[count];

    __value_list_t* current = reinterpret_cast<__value_list_t*>(root->next);
    for (; current != root; current = reinterpret_cast<__value_list_t*>(current->next)) {
      auto tmp_hash = current->hash;
      buckets[tmp_hash % count].value_list = current;

      for (;
            current->next != root && current->hash == tmp_hash;
            current = reinterpret_cast<__value_list_t*>(current->next)
      );
    }

    bucket_count_ = count;
  }

  size_type size() const {
    return size_;
  }

  float load_factor() const {
    return static_cast<float>(static_cast<long double>(size_) / static_cast<long double>(bucket_count_));
  }

  void max_load_factor(float max_lf) {
    if (max_lf > max_load_factor_)
      max_load_factor_ = max_lf;
  }

  float max_load_factor() {
    return max_load_factor_;
  }

  void __maybe_rehash() {
    __maybe_rehash(bucket_count_ * 2);
  }

  void __maybe_rehash(size_type count) {
    if (size_ > max_load_factor_ * bucket_count_) {
      rehash(count);
    }
  }

  void __test() { 
    __value_list_t* current = reinterpret_cast<__value_list_t*>(root->next);

    while (current != root) {
//      std::cout << current->hash << ' ' << current->value.first << ' ' << &current->value.second << '\n';
      current = reinterpret_cast<__value_list_t*>(current->next);
    }
  }

};

}
