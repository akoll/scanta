#pragma once

#include <iostream>

#include "memory_tracker.hpp"

namespace benchmark {

/// ECS System to output memory usage to stdout.
///
/// @tparam N Number of frames to measure.
template<size_t N>
class MemoryUsageSystem {
public:
  void operator()() {
    // Don't measure the first frame to stay aligned with the frametime measuring system.
    if (_first_frame) _first_frame = false;
    else if (_frame_number++ < N)
      std::cout << benchmark::memory::get_usage() << std::endl;
  }

private:
  bool _first_frame = true;
  size_t _frame_number = 0;
};

}
