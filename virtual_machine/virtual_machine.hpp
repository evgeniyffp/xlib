#pragma once

#include <iostream>
#include <utility>
#include <memory>
#include <functional>

namespace xlib {

#define STACK_MAX_SIZE 256
#define IGCT 8

enum class states {
	basic, used, deleted
};

struct IObject {
	virtual states get_state() const = 0;
	virtual void set_state(states) = 0;

	virtual IObject*& get_next() = 0;

  bool is_valid() { return get_state() != states::deleted; }

  virtual void destroy() = 0;

	virtual ~IObject() = default;
};

template <typename T>
class Object : public IObject {
private:
	states state;

	IObject* next;

  alignas(T) char data[sizeof(T)];

public:
	void set_state(states _state) override {
		state = _state;
	}

	states get_state() const override {
		return state;
	}

	IObject*& get_next() override {
		return next;
	}

	operator T&() {
		return *reinterpret_cast<T*>(data);
	}

	operator const T&() const {
		return *reinterpret_cast<const T*>(data);
	}

  void destroy() override {
    if (is_valid()) {
      state = states::deleted;
      reinterpret_cast<T*>(data)->~T();
    }
  }

  template <typename... Args>
	Object(states state, IObject* next, Args&&... args)
			: state(state)
			, next(next) {
        new (data) T(std::forward<Args>(args)...);
      }
	~Object() override {
    destroy();
  }
};

class virtual_machine {
public:
  template <typename T, typename... Args>
  friend auto make_unique(virtual_machine& vm, Args&&... args);

	virtual_machine() = default;
	~virtual_machine() {
		stackSize = 0;
		collect_garbage();
	}

	void collect_garbage() {
		size_t old_count_objects = count_objects;

		mark_all();
		markspeep();

		max_objects = count_objects * 2;

		std::cout << "Collected " << old_count_objects - count_objects << " objects, " << count_objects << " left.\n";
	}

private:
	IObject* stack[STACK_MAX_SIZE]{};

	size_t stackSize = 0;
	IObject* begin = nullptr;

	size_t count_objects = 0;
	size_t max_objects = IGCT;

	void push(IObject* value) {
    if (stackSize >= STACK_MAX_SIZE) {
      throw std::overflow_error("xlib::virtual_machine::push(): stack overflow");
    }
		stack[stackSize++] = value;
	}

	IObject* pop() {
		return stack[--stackSize];
	}

	void mark_all() {
		for (size_t i = 0; i < stackSize; i++) {
			if (stack[i]->is_valid())
        stack[i]->set_state(states::used);
		}
	}

  template <typename T, typename... Args>
	Object<T>* create(Args&&... args) {
    Object<T>* object = new Object<T>(states::basic, begin, std::forward<Args>(args)...);
		begin = object;

		count_objects++;

		push(object);
		return object;
	}

	void markspeep() {
		IObject** object = &begin;
		while (*object != nullptr) {
			if ((*object)->get_state() == states::used) {
				(*object)->set_state(states::basic);
				object = &(*object)->get_next();
			} else {
				IObject* unreached = *object;

				*object = unreached->get_next();
				unreached->~IObject();

				count_objects--;
			}
		}
	}
};

template <typename T, typename... Args>
auto make_unique(virtual_machine& vm, Args&&... args) {
  return std::unique_ptr<Object<T>, std::function<void(Object<T>*)>>(
    vm.create<T>(std::forward<Args>(args)...),
    [](Object<T>* object) { object->destroy(); }
  );
}

}
