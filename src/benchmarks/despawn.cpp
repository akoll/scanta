#include <cstring>
#include <cassert>

#include "util/benchmark.hpp"
#include "util/timer.hpp"

struct Payload {
  size_t value;
};

class DespawnSystem {
public:
  auto operator()(ECS::Entity entity, const Payload&) {
    return [this, entity](const auto& manager) {
      #ifdef REVERSE
      if (++_removed_count > manager.get_entity_count() - DESPAWN_RATE)
      #else
      if (_removed_count++ < DESPAWN_RATE)
      #endif
        _queue.push_back(entity);
      
      #ifdef REVERSE
      if (_removed_count == manager.get_entity_count()) {
      #else
      if (_removed_count == DESPAWN_RATE) {
      #endif
        ++_removed_count;
        assert(_queue.size() == DESPAWN_RATE);
        manager.defer([this](const auto& deferred_manager){
          {
            benchmark::Timer _;
            #ifdef REVERSE
            for (auto it = _queue.rbegin(); it != _queue.rend(); ++it)
            #else
            for (auto it = _queue.begin(); it != _queue.end(); ++it)
            #endif
              deferred_manager.remove_entity(*it);
          }
          _queue.clear();
          _removed_count = 0;
        });
      }
    };
  }
private:
  size_t _removed_count = 0;
  std::vector<ECS::Entity> _queue;
};
  
int main(int argc, const char** argv) {
  benchmark::Scene scene(
    #ifdef BENCHMARK_ENTITY_COUNT
    [](){ return [](const auto& manager) {
      std::cout << manager.get_entity_count() << std::endl;
    }; },
    #endif
    DespawnSystem{}
  );
  for (auto i{0u}; i < INITIAL_COUNT; ++i)
    scene->manager.new_entity(Payload{});
  scene.run();
  return 0;
}
