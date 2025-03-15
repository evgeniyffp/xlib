#pragma once

#include <cstddef> // std::size_t, std::ptrdiff_t
#include <utility> // std::forward, std::move
#include <type_traits> // std::is_same_v

#include "./default_delete.hpp"
#include "./unique_ptr.hpp"
#include "../utility/private_tag.hpp"

namespace xlib::memory {
  template <typename T> class weak_ptr;

  template <typename T>
  class shared_ptr {
    public:
      struct ControlBlock {
        std::size_t* shared_count;
        std::size_t* weak_count;

        T* ptr = nullptr;

        ControlBlock(T* ptr, std::size_t* shared_count, std::size_t* weak_count)
              : shared_count(shared_count), weak_count(weak_count), ptr(ptr) {}

        void try_clear() {
          if (*shared_count == 0) {
            if (ptr != nullptr) {
              use_deleter();
              ptr = nullptr;
            }

            if (*weak_count == 0)
              delete this;
          }

        }

        virtual void use_deleter() {
          default_delete<T>{}(ptr);
        }

        virtual ~ControlBlock() = default; // TODO: maybe not virtual?
      };

      class ControlBlockWithCount : public ControlBlock {
      private:
        size_t shared_data = 1;
        size_t weak_data = 0;

      public:
        ControlBlockWithCount(T* ptr) : ControlBlock(ptr, &shared_data, &weak_data) {}
        ~ControlBlockWithCount() override = default;
      };

      template <typename Deleter = default_delete<T>>
      struct ControlBlockWithPtr : public ControlBlockWithCount {
        Deleter deleter;

        void use_deleter() override {
          deleter(this->ptr);
        }

        ControlBlockWithPtr(T* ptr) : ControlBlockWithCount(ptr) {}


        ControlBlockWithPtr(T* ptr, const Deleter& deleter)
              : ControlBlock(ptr), deleter(deleter) {}

        ControlBlockWithPtr(T* ptr, Deleter&& deleter)
              : ControlBlock(ptr), deleter(std::move(deleter)) {}

        ~ControlBlockWithPtr() override = default;
      };

      struct ControlBlockWithObj : public ControlBlockWithCount {
        alignas(T) char data[sizeof(T)];

        void use_deleter() override {
          this->ptr->~T();
        }

        template <typename... Args>
        ControlBlockWithObj(Args&&... args)
            : ControlBlockWithCount(reinterpret_cast<T*>(data)) {
          new (this->ptr) T(std::forward<Args>(args)...);
        }

        ~ControlBlockWithObj() override = default;
      };
public:
      ControlBlock* block = nullptr;

      template <typename... Args>
      shared_ptr(private_tag_t, Args&&... args)
            : block(new ControlBlockWithObj(std::forward<Args>(args)...)) {}

      shared_ptr(private_tag_t, ControlBlock* block)
            : block(block) {}

    public:
      template <typename U>
      friend class weak_ptr;

      template <typename _T, typename U>
      friend shared_ptr<_T> static_pointer_cast(const shared_ptr<U>& other);

      template <typename _T, typename U>
      friend shared_ptr<_T> dynamic_pointer_cast(const shared_ptr<U>& other);

      template <typename _T, typename U>
      friend shared_ptr<_T> const_pointer_cast(const shared_ptr<U>& other);

      template <typename U, typename... Args>
      friend shared_ptr<U> make_shared(Args&&... args);


      shared_ptr(T* ptr = nullptr) : block(new ControlBlockWithPtr(ptr)) {}

      template <typename Deleter>
      shared_ptr(T* ptr, Deleter deleter) : block(new ControlBlockWithPtr<Deleter>(ptr, std::move(deleter))) {}

      shared_ptr(const shared_ptr& other) : block(other.block) {
        ++*block->shared_count;
      }

      shared_ptr(shared_ptr&& other) : block(other.block) {
        other.block = nullptr;
      }

      shared_ptr(const weak_ptr<T>& other) : block(other.block) {
        ++*block->shared_count;
      }

      template <typename Deleter>
      shared_ptr(unique_ptr<T, Deleter>&& other)
            : block(new ControlBlockWithPtr<Deleter>(other.release())) {}

      ~shared_ptr() {
        --*block->shared_count;
        block->try_clear();
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
      new typename shared_ptr<T>::ControlBlock(static_cast<U*>(other.block->ptr), other.block->shared_count, other.block->weak_count)
    };
  }

  template <typename T, typename U>
  shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& other) {
    ++*other.block->shared_count;
    return {
      private_tag,
      new typename shared_ptr<T>::ControlBlock(dynamic_cast<U*>(other.block->ptr), other.block->shared_count, other.block->weak_count)
    };
  }

  template <typename T, typename U>
  shared_ptr<T> const_pointer_cast(const shared_ptr<U>& other) {
    ++*other.block->shared_count;
    return {
      private_tag,
      new typename shared_ptr<T>::ControlBlock(const_cast<U*>(other.block->ptr), other.block->shared_count, other.block->weak_count)
    };
  }
}
