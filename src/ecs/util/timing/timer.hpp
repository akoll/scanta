#include <chrono>
#include <string>
#include <iostream>

namespace benchmark {

class Timer {
public:
  Timer(std::string name) : _name(name), _start(std::chrono::high_resolution_clock::now()) {}

  ~Timer() {
    auto end = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    auto start = std::chrono::time_point_cast<std::chrono::nanoseconds>(_start).time_since_epoch().count();
    auto elapsed = end - start;
    std::cout << _name << ": " << elapsed * 0.000001 << "ms" << std::endl;
  }
private:
  std::string _name;
  std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};

}

namespace timing {

class Timer {
public:
  Timer() : _last(std::chrono::high_resolution_clock::now()) {}

  double reset() {
    auto last = std::chrono::time_point_cast<std::chrono::nanoseconds>(_last).time_since_epoch().count();
    _last = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::time_point_cast<std::chrono::nanoseconds>(_last).time_since_epoch().count() - last;
    return elapsed * 0.000001;
  }
private:
  // TODO: benchmark usage of nanos since epoch here (to avoid time_point_cast)
  std::chrono::time_point<std::chrono::high_resolution_clock> _last;
};

}