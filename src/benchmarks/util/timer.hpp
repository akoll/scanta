#pragma once

#include <chrono>
#include <string>
#include <iostream>

/// Namespace for anything related to gathering non-functional metrics.
namespace benchmark {

class Timer {
public:
  Timer() : _start(std::chrono::high_resolution_clock::now()) {}

  ~Timer() {
    auto end = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    auto start = std::chrono::time_point_cast<std::chrono::nanoseconds>(_start).time_since_epoch().count();
    auto elapsed = end - start;
    std::cout << elapsed * 0.000000001 << std::endl;
  }
private:
  std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};

}
