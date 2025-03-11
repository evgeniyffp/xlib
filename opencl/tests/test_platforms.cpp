#include <iostream>

#include "cl_wrapper.h"

using namespace xlib;

int main() {
  auto platforms = cl::get_platforms();
  std::cout << "Number of platforms: " << cl::get_platform_count() << "\n";

  for (cl_uint i = 0; i < cl::get_platform_count(); i++) {
    std::cout << "Platform " << i + 1 << ": " << platforms[i].get_name() << "\n";

    auto device_count = platforms[i].get_device_count();

    auto devices = platforms[i].get_devices();

    for (cl_uint j = 0; j < device_count; j++) {
      std::cout << "    Device " << j + 1 << ": " << devices[j].get_name() << "\n";
    }
  }

  return 0;
}

