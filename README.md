### **xlib**
## The description
  *xlib* is my big project, which include a lot of library(headers).
## **The components**
  Project is still being improved by me.
  - *containers* 0%. Now it have nothing. In future there will a lot of containers(such as hash_map, avl_tree, stable_vector, devector, maybe simpe vector, list and etc.)
  - *function* 99%. It have 2 realization(*function.hpp* and *function_may_be_with_virtual.hpp*. And I think *function.hpp* have UB, but I'm not sure about that.)
  - *serialization* 50%. There are a few things that need to be fixed.
  - memory 70%. *shared_ptr* and *weak_ptr* is almost ready. There will *unique_ptr.hpp*, maybe *hazard_ptr.hpp* in future.
## **How to use**
  You can any c++ compiler(I was testing it by clang) with C++20.
  ```C++
  #include <iostream>
  #include "./memory/shared_ptr.hpp"

  int main() {
    xlib::memory::shared_ptr<const char> p("Hello, World", [](const char*){});

    std::cout << p.get();

    return 0;
  }
  ```
  This is one of many examples(see directory *./examples*).


