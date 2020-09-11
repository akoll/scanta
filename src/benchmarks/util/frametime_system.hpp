#pragma once

#include <cstddef>
#include <functional>

namespace benchmark {

/// ECS System to measure frame times and output them to stdout.
///
/// @tparam N Number of frames to measure.
template<size_t N>
class FrametimeSystem {
public:
  /// @param callback Callback to call when N frames have been measured.
  FrametimeSystem(const std::function<void()>& callback) : callback(callback) {}

  float get_seconds_total() const {
    return _seconds_total;
  }

  void operator()(const ecs::RuntimeManager& manager, double delta_time) {
    _seconds_total += delta_time;
    if (_current_frame < _frame_times.size()) {
      _frame_times[_current_frame++] = delta_time;
    } else {
      for (auto& time : _frame_times)
        std::cout << time << std::endl;
      callback();
    }
  }

  /// Reset counters and discard all unprinted measurements.
  void reset() {
    _seconds_total = 0.0f;
    _current_frame = 0;
  }

private:
  float _seconds_total = 0.0f;
  uint32_t _current_frame = 0;
  std::array<double, N> _frame_times;
  const std::function<void()>& callback;
};

}
