#include <cstring>
#include <cassert>

#include "ecs/storage/scattered.hpp"

#include "util/timer.hpp"

using Storage = ecs::storage::ScatteredCustom
  #ifdef STORAGE_SCATTERED_SMART
  ::WithSmartPointers
  #endif
  #ifdef STORAGE_SCATTERED_SET
    ::WithEntitySet
  #endif
  ::Storage<size_t>;

int main() {
  Storage storage;
  for (auto i{0u}; i < ENTITY_COUNT; ++i)
    storage.new_entity(size_t{5});
  
  size_t de_index = 0;
  while (de_index < ENTITY_COUNT) {
    size_t index = 0;
    Storage::Entity entity{nullptr};
    storage.for_entities_with<size_t>([&](Storage::Entity e) {
      if (index++ == de_index)
        entity = e;
    });
    
    {
      benchmark::Timer _;
      storage.remove_entity(entity);
    }

    storage.new_entity(size_t{5});
    de_index += INTERVAL;
  }

  return 0;
}
