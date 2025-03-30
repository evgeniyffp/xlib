### **xlib**
## The description
  *xlib* is my big project, which include a lot of library(headers).
## **The components**
  Project is still being improved by me.
  - *serialization* 99%. There are a few things that need to be fixed.
  - *function* 99%. It have 2 realization(*function.hpp* and *function_may_be_with_virtual.hpp*. And I think *function.hpp* have UB, but I'm not sure about that.)
  - memory 70%. *shared_ptr* and *weak_ptr* is almost ready. There will *unique_ptr*, maybe *hazard_ptr* in future.
  - variant 50%.
  - allocators 30%.
  - *containers* 20%. Now there're *unordered_map*. *avl_tree* isn't ready now. In future there will a lot of containers(such as *hash_map*, *stable_vector*, *devector*, maybe *vector* and *inplace_vector*, *list* and etc.)
  - utility 10%. Now it have *timer*, *private_tag*.
### Comming soon:
  - update *multithreading*: add *thread*, *async* and etc.
  - update *memory*
  - add *tuple*

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
  This is one of many examples(see directory *./examples* or *./tests*).


