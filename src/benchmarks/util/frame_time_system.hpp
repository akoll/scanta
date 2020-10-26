#pragma once

#include <iostream>
#include <cstddef>
#include <functional>

namespace benchmark {

/// ECS System to measure frame delta times and output them to stdout.
///
/// @tparam N Number of frames to measure.
template<size_t N>
class FrameTimeSystem {
public:
  float get_seconds_total() const {
    return _seconds_total;
  }

  void operator()(double delta_time) {
    if (_first_frame) _first_frame = false;
    else {
      _seconds_total += delta_time;
      if (_current_frame < _frame_times.size())
        _frame_times[_current_frame++] = delta_time;
      if (_current_frame >= N) {
        for (auto& time : _frame_times)
          std::cout << time << std::endl;
      }
    }
  }

  /// Resets counters and discard all unprinted measurements.
  void reset() {
    _seconds_total = 0.0f;
    _current_frame = 0;
  }

private:
  bool _first_frame = true;
  float _seconds_total = 0.0f;
  uint32_t _current_frame = 0;
  std::array<double, N> _frame_times;
};

}
