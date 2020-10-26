#pragma once

#include <iostream>
#include <cstddef>
#include <functional>

namespace benchmark {

/// ECS System to call some function as soon as a specified number of frames have passed.
///
/// The callback is first called in the frame _after_ `N` frames have completed.
/// @tparam N Number of frames to be completed before the callback is first called.
template<size_t N>
class FrameLimitSystem {
public:
  /// @param callback Callback to call when N frames have passed.
  FrameLimitSystem(std::function<void()> callback) : _callback(callback) {}

  void operator()() {
    // Call the callback if at least N frames have passed.
    if (_frame_number++ >= N) _callback();
  }

private:
  size_t _frame_number = 0;
  const std::function<void()> _callback;
};

}
