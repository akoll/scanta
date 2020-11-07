#include <cstdlib>

#include "util/benchmark.hpp"

constexpr auto iters = 32u;
constexpr auto payload_size = 8u;

template<size_t size, size_t index>
struct Payload {
  static constexpr size_t id = index;
  std::array<size_t, size> data;
};

template<typename TPayload>
class ScreenSystem {
public:
  void operator()(TPayload& payload)
  #ifdef INNER_PARALLELISM
    const
  #endif
  {
    size_t sum = 0;
    for (auto _{0u}; _ < iters; ++_)
      for (auto i{0u}; i < payload.data.size(); ++i) sum += ++payload.data[i] * i;
    _screen = sum;
  }
private:
  volatile size_t _screen;
};

template<size_t... I>
auto make_scene(std::index_sequence<I...>) {
  return benchmark::Scene(
    ScreenSystem<Payload<payload_size, I>>{}...
  );
}

template<size_t... I>
auto spawn_entity(const auto& manager, std::index_sequence<I...>) {
    manager.new_entity(
      // Payload<payload_size, I>{}...
      Payload<payload_size, 0>{}
    );
}
  
int main(int argc, const char** argv) {
  if (argc < 2) return 1;
  unsigned int count = atoi(argv[1]);

  constexpr auto sequence = std::make_index_sequence<SYSTEM_COUNT>{};

  auto scene = make_scene(sequence);

  for (auto i{0u}; i < count; ++i)
    spawn_entity(scene->manager, sequence);

  scene.run();

  return 0;
}
