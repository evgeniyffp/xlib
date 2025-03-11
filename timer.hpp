#pragma once

#include <chrono>

namespace xlib {
  class timer {
  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

  public:
    timer() {
      restart();
    }

    void restart() {
      start_time = std::chrono::high_resolution_clock::now();
    }

    double elapsedMilliseconds() const {
      auto end_time = std::chrono::high_resolution_clock::now();
      return std::chrono::duration<double, std::milli>(end_time - start_time).count();
    }

    double elapsedSeconds() const {
      auto end_time = std::chrono::high_resolution_clock::now();
      return std::chrono::duration<double>(end_time - start_time).count();
    }
  };
}
