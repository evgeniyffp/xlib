#pragma once

#include <cstddef> // std::size_t, std::ptrdiff_t
#include <utility> // std::forward, std::move

namespace xlib::memory {
  template <typename T>
  class shared_ptr {
    public:
      struct ControlBlock {
        std::size_t shared_count;
      // std::size_t weak_count; TODO

        T* ptr = nullptr;

        ControlBlock(T* ptr) : shared_count(1), ptr(ptr) {}
        virtual ~ControlBlock() = default;
      };

      template <typename Deleter = default_delete<T>>
      struct ControlBlockWithPtr : public ControlBlock {
        Deleter deleter;

        ControlBlockWithPtr(T* ptr) : ControlBlock(ptr) {}

        ControlBlockWithPtr(T* ptr, const Deleter& deleter)
              : ControlBlockWithPtr(ptr), deleter(deleter) {}

        ControlBlockWithPtr(T* ptr, Deleter&& deleter)
              : ControlBlockWithPtr(ptr), deleter(std::move(deleter)) {}

        ~ControlBlockWithPtr() override {
          deleter(this->ptr);
        }
      };

      struct ControlBlockWithObj : public ControlBlock {
        alignas(T) char data[sizeof(T)];

        template <typename... Args>
        ControlBlockWithObj(Args&&... args)
            : ControlBlock(reinterpret_cast<T*>(data)) { // Undefine behavour (align!) TODO
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

    public:
      shared_ptr(T* ptr) : block(new ControlBlockWithPtr(ptr)) {}

      template <typename Deleter>
      shared_ptr(T* ptr, Deleter deleter) : block(new ControlBlockWithPtr<Deleter>(ptr, std::move(deleter))) {}

      template <typename U, typename... Args>
      friend shared_ptr<U> make_shared(Args&&... args);

      shared_ptr(const shared_ptr& other) : block(other.block) {
        ++block->shared_count;
      }

      shared_ptr(shared_ptr&& other) : block(other.block) {
        other.block = nullptr;
      }

      ~shared_ptr() {
        if (--block->shared_count == 0)
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
        return block->shared_count;
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
        shared_ptr<T, Deleter>(ptr, std::move(deleter)).swap(*this);
      }
  };

  template <typename U, typename... Args>
  shared_ptr<U> make_shared(Args&&... args) {
    return { private_tag, std::forward<Args>(args)... };
  }
}
