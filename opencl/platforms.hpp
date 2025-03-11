#pragma once

#include <CL/cl.h>

#include <stdexcept> // std::out_of_range

namespace xlib::cl {
  // ---------- DEVICE ----------
  class device {
  private:
  public: // FOR DEBUG
    cl_device_id id;

    friend class devices;
    friend class platform;
    friend class command_queue;

    device(cl_device_id id) : id(id) {}

  public:
    std::string get_name() {
      std::string name(128, '\0');
      clGetDeviceInfo(id, CL_DEVICE_NAME, 128, name.data(), NULL);
      return name;
    }
  };

  // ---------- DEVICES ----------
  class devices {
  private:
    cl_uint device_count = -1;
    cl_device_id* array = nullptr;

    friend class platform;
    friend class context;

    devices(cl_uint device_count, cl_device_id* array)
      : device_count(device_count), array(array) {}

  public:
    device operator[](size_t index) {
      if (index > device_count)
        throw std::out_of_range("xlib::cl::platforms::operator[](index): index > device_count\n");

      return { array[index] };
    }

    ~devices() {
      delete array;
      array = nullptr;
      device_count = -1;
    }
  };

  // ---------- PLATFORM ----------
  class platform {
  private:
    cl_platform_id id;

    friend class platforms;
    friend platform get_platform();

    platform(cl_platform_id id) : id(id) {}

  public:
    cl_uint get_device_count() {
      cl_uint device_count;

      clGetDeviceIDs(id, CL_DEVICE_TYPE_ALL, 0, NULL, &device_count);

      return device_count;
    }

    devices get_devices() {
      auto device_count = get_device_count();

      auto* array = new cl_device_id[device_count];

      clGetDeviceIDs(id, CL_DEVICE_TYPE_ALL, device_count, array, NULL);

      return {device_count, array};
    }

    device get_gpu_device() {
      cl_device_id gpu_device;

      clGetDeviceIDs(id, CL_DEVICE_TYPE_GPU, 1, &gpu_device, NULL);

      return { gpu_device };
    }

    std::string get_name() {
      std::string name(128, '\0');
      clGetPlatformInfo(id, CL_PLATFORM_NAME, 128, name.data(), NULL);
      return name;
    }
  };

  // ---------- PLATFORMS ----------
  class platforms {
  private:
    cl_uint platform_count = -1;
    cl_platform_id* array = nullptr;

    friend platforms get_platforms();

    platforms(cl_uint platform_count, cl_platform_id* array)
      : platform_count(platform_count), array(array) {}

  public:
    platform operator[](size_t index) {
      if (index > platform_count)
        throw std::out_of_range("xlib::cl::platforms::operator[](index): index > platform_count\n");

      return { array[index] };
    }

    ~platforms() {
      delete array;
      array = nullptr;
      platform_count = -1;
    }
  };

  cl_uint get_platform_count() {
    cl_uint platform_count;
    clGetPlatformIDs(0, NULL, &platform_count);
    return platform_count;
  }

  platforms get_platforms() {
    auto platform_count = get_platform_count();

    auto* array = new cl_platform_id[platform_count];

    clGetPlatformIDs(platform_count, array, NULL);

    return {platform_count, array};
  }

  platform get_platform() {
    cl_platform_id data;

    clGetPlatformIDs(1, &data, NULL);

    return {data};
  }
}

