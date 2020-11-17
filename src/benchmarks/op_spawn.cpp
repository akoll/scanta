#include <cstring>
#include <cassert>

#include "scanta/storage/scattered.hpp"

#include "util/timer.hpp"

using Storage = scanta::storage::ScatteredCustom
  #ifdef STORAGE_SCATTERED_SMART
  ::WithSmartPointers
  #endif
  #ifdef STORAGE_SCATTERED_SET
    ::WithEntitySet
  #endif
  ::Storage<size_t>;

constexpr size_t iterations = ENTITY_COUNT / SPAWN_RATE;

int main() {
  Storage storage;
  for (auto i{0u}; i < iterations; ++i) {
    benchmark::Timer _;
    for (auto e{0u}; e < SPAWN_RATE; ++e)
      storage.new_entity(size_t{5});
  }
  
  return 0;
}
