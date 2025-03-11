#pragma once

#include <utility> // std::move
#include <fstream>
#include <string>
#include <vector>
#include <type_traits> // std::is_fundamental_v<>
#include <exception>

#include <boost/type_index.hpp>

namespace xlib::serialization {

  // Serialization's model:
  //  Name of type('\0' is end) - Size(16 bits) - Data(size is Size)

  class serializator {
  public:
    template <typename CharT>
    struct basic_node_type {
      std::basic_string<CharT> name;
      std::vector<uint8_t> array;

      bool is_default() {
        return name.empty() && array.empty();
      }
    };

    using node_type = basic_node_type<char>;

  private:
    std::fstream file;

    bool try_write(const node_type& node) {
      file << node.name;
      file << '\n';
      file << static_cast<uint8_t>(node.array.size() >> 8);
      file << static_cast<uint8_t>(node.array.size() >> 0);
      for (auto x : node.array)
        file << x;

      file.flush();

      return static_cast<bool>(file);
    }

    bool try_read(node_type& node) {
      file >> node.name;

      uint8_t tmp;
      uint16_t size = 0;
      file >> tmp;
      size += tmp << 8;
      file >> tmp;
      size += tmp;
      
      std::cout << size << std::endl;

      node.array.resize(size);
      for (auto& x : node.array)
        x = file.get();

      return static_cast<bool>(file);
    }


  public:
    serializator(const std::string& name,
        std::ios_base::openmode mode = 
          std::ios_base::out |  std::ios_base::in | std::ios_base::app | std::ios_base::binary)
      : file(name, mode) {}

    template <typename T>
    friend serializator& operator<<(serializator& s, const T& value);

    template <typename T>
    friend serializator& operator>>(serializator& s, T& value);
  };

  struct failed_write : std::exception {
    failed_write() = default;
  };

  struct failed_read : std::exception {
    failed_read() = default;
  };

  struct failed_encode : std::exception {
    failed_encode() = default;
  };

  struct failed_decode : std::exception {
    failed_decode() = default;
  };

  namespace fundamental {
    void encode(std::vector<uint8_t>& array, float value) {
      auto size = array.size();
      auto str = std::to_string(value);
      array.resize(size + str.size());
      std::copy(str.begin(), str.end(), array.begin() + size);
    }

    void encode(std::vector<uint8_t>& array, int8_t value) {
      auto size = array.size();
      array.resize(size + 1);
      array[size + 0] = value >> 0;
    }

    void encode(std::vector<uint8_t>& array, int16_t value) {
      auto size = array.size();
      array.resize(size + 2);
      array[size + 0] = value >> 8;
      array[size + 1] = value >> 0;
    }

    void encode(std::vector<uint8_t>& array, int32_t value) {
      auto size = array.size();
      array.resize(size + 4);
      array[size + 0] = value >> 24;
      array[size + 1] = value >> 16;
      array[size + 2] = value >> 8;
      array[size + 3] = value >> 0;
    }

    void encode(std::vector<uint8_t>& array, int64_t value) {
      auto size = array.size();
      array.resize(size + 8);
      array[size + 0] = value >> 56;
      array[size + 1] = value >> 48;
      array[size + 2] = value >> 40;
      array[size + 3] = value >> 32;
      array[size + 4] = value >> 24;
      array[size + 5] = value >> 16;
      array[size + 6] = value >> 8;
      array[size + 7] = value >> 0;
    }

#define _f(x) (static_cast<int64_t>(x))

    void decode(uint8_t& value, std::vector<int8_t>::const_iterator& it) {
      value = (*(it++) << 0);
    }

    void decode(int16_t& value, std::vector<uint8_t>::const_iterator& it) {
      value = (*(it++) << 8)
            + (*(it++) << 0);
    }

    void decode(int32_t& value, std::vector<uint8_t>::const_iterator& it) {
      value = (_f(*(it++)) << 24)
            + (_f(*(it++)) << 16)
            + (_f(*(it++)) << 8)
            + (_f(*(it++)) >> 0);
    }
/*
    void decode(int64_t& value, std::vector<uint8_t>::const_iterator& it) {
      value = (*(it++) >> 56)
            + (*(it++) >> 48)
            + (*(it++) >> 40)
            + (*(it++) >> 32)
            + (*(it++) >> 24)
            + (*(it++) >> 16)
            + (*(it++) >> 8)
            + (*(it++) >> 0);
    }*/
  }

  template <typename T>
  requires (!std::is_fundamental_v<T>)
  void encode(std::vector<uint8_t>& array, const T& value) {
    serializator::node temp_node;

    if (!value.try_encode(temp_node)) {
      throw failed_encode{};
    }

    auto old_size = node_type.array.size();
    array.resize(array.size() + old_size);
    array.insert(array.begin() + old_size, node.array.begin(), node.array.end());

    return true;
  }

  template <typename T>
  serializator& operator<<(serializator& s, const T& value) {
    serializator::node_type temp_node;
    temp_node.name = boost::typeindex::type_id_with_cvr<T>().pretty_name();

    if constexpr (std::is_fundamental_v<T>) {
      fundamental::encode(temp_node.array, value);
    }
    else {
      if (!value.try_encode(temp_node)) {
        throw failed_encode{};
      }
    }

    if (!s.try_write(temp_node)) {
        throw failed_write{};
    }

    return s;
  }

  template <typename T>
  serializator& operator>>(serializator& s, T& value) {
    serializator::node_type temp_node;

    if (!s.try_read(temp_node)) {
        throw failed_read{};
    }

    if (!value.try_decode(temp_node)) {
        throw failed_decode{};
    }

    return s;
  }

}

namespace xlib {
  using serialization::serializator;
}
