#pragma once

#include <cstddef> // std::size_t, std::ptrdiff_t
#include <utility> // std::forward, std::move
#include <type_traits> // std::is_same_v

#include "./default_delete.hpp"
#include "./unique_ptr.hpp"
#include "../utility/private_tag.hpp"

namespace xlib::memory {
  template <typename T>
  class shared_ptr {
    public:
      struct ControlBlock {
        std::size_t* shared_count;
      // std::size_t* weak_count; TODO

        T* ptr = nullptr;

        ControlBlock(T* ptr, std::size_t* shared_count)
              : shared_count(shared_count), ptr(ptr) {}

        virtual ~ControlBlock() = default;
      };

      class ControlBlockWithCount : public ControlBlock {
      private:
        size_t data = 1;

      public:
        ControlBlockWithCount(T* ptr) : ControlBlock(ptr, &data) {}
        ~ControlBlockWithCount() override = default;
      };

      template <typename Deleter = default_delete<T>>
      struct ControlBlockWithPtr : public ControlBlockWithCount {
        Deleter deleter;

        ControlBlockWithPtr(T* ptr) : ControlBlockWithCount(ptr) {}

        ControlBlockWithPtr(T* ptr, const Deleter& deleter)
              : ControlBlock(ptr), deleter(deleter) {}

        ControlBlockWithPtr(T* ptr, Deleter&& deleter)
              : ControlBlock(ptr), deleter(std::move(deleter)) {}

        ~ControlBlockWithPtr() override {
          deleter(this->ptr);
        }
      };

      struct ControlBlockWithObj : public ControlBlockWithCount {
        alignas(T) char data[sizeof(T)];

        template <typename... Args>
        ControlBlockWithObj(Args&&... args)
            : ControlBlockWithCount(reinterpret_cast<T*>(data)) { // Undefine behavour (align!) TODO
          new (this->ptr) T(std::forward<Args>(args)...);
        }

        ~ControlBlockWithObj() override {
          this->ptr->~T();
        }
      };

      ControlBlock* block = nullptr;

      template <typename... Args>
      shared_ptr(private_tag_t, Args&&... args)
            : block(new ControlBlockWithObj(std::forward<Args>(args)...)) {}

      shared_ptr(private_tag_t, ControlBlock* block)
            : block(block) {}

    public:
      template <typename _T, typename U>
      friend shared_ptr<_T> static_pointer_cast(const shared_ptr<U>& other);

      template <typename U, typename... Args>
      friend shared_ptr<U> make_shared(Args&&... args);

      shared_ptr(T* ptr) : block(new ControlBlockWithPtr(ptr)) {}

      template <typename Deleter>
      shared_ptr(T* ptr, Deleter deleter) : block(new ControlBlockWithPtr<Deleter>(ptr, std::move(deleter))) {}

      shared_ptr(const shared_ptr& other) : block(other.block) {
        ++*block->shared_count;
      }

      shared_ptr(shared_ptr&& other) : block(other.block) {
        other.block = nullptr;
      }

      template <typename Deleter>
      shared_ptr(unique_ptr<T, Deleter>&& other)
            : block(new ControlBlockWithPtr<Deleter>(other.release())) {}

      ~shared_ptr() {
        if (--*block->shared_count == 0)
          delete block;
      }

      void swap(shared_ptr& other) {
        std::swap(block, other.block);
      }

      T* get() {
        return block->ptr;
      }

      T& operator*() {
        return *get();
      }

      T* operator->() {
        return get();
      }

      explicit operator bool() const noexcept {
        return get() == nullptr;
      }

      T& operator[](std::ptrdiff_t index) {
        return *(get()[index]);
      }

      size_t use_count() const {
        return *block->shared_count;
      }

      bool unique() const {
        return use_count() == 1;
      }

      void reset() {
        shared_ptr().swap(*this);
      }

      void reset(T* ptr) {
        shared_ptr(ptr).swap(*this);
      }

      template <typename Deleter>
      void reset(T* ptr, Deleter deleter) {
        shared_ptr(ptr, std::move(deleter)).swap(*this);
      }
  };

  template <typename U, typename... Args>
  shared_ptr<U> make_shared(Args&&... args) {
    return { private_tag, std::forward<Args>(args)... };
  }

  template <typename T, typename U>
  shared_ptr<T> static_pointer_cast(const shared_ptr<U>& other) {
    ++*other.block->shared_count;
    return {
      private_tag,
      new typename shared_ptr<T>::ControlBlock(static_cast<U*>(other.block->ptr), other.block->shared_count)
    };
  }

  template <typename T, typename U>
  shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& other) {
    ++*other.block->shared_count;
    return {
      private_tag,
      new typename shared_ptr<T>::ControlBlock(dynamic_cast<U*>(other.block->ptr), other.block->shared_count)
    };
  }

  template <typename T, typename U>
  shared_ptr<T> const_pointer_cast(const shared_ptr<U>& other) {
    ++*other.block->shared_count;
    return {
      private_tag,
      new typename shared_ptr<T>::ControlBlock(const_cast<U*>(other.block->ptr), other.block->shared_count)
    };
  }
}
