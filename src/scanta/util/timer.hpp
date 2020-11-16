#pragma once

#include <chrono>
#include <string>
#include <iostream>

/// Namespace for anything related to gathering time-behavioral metrics.
namespace timing {

class Timer {
public:
  Timer() : _last(std::chrono::high_resolution_clock::now()) {}

  double reset() {
    auto last = std::chrono::time_point_cast<std::chrono::nanoseconds>(_last).time_since_epoch().count();
    _last = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::time_point_cast<std::chrono::nanoseconds>(_last).time_since_epoch().count() - last;
    return elapsed * 0.000000001;
  }
private:
  // TODO: benchmark usage of nanos since epoch here (to avoid time_point_cast)
  std::chrono::time_point<std::chrono::high_resolution_clock> _last;
};

}
