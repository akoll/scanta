/// TODO: doc
/// https://youtu.be/sLlGEUO_EGE

namespace benchmark {

  namespace memory {

    namespace internal {

    static struct MemoryStats {
      size_t allocated = 0;
      size_t freed = 0;

      size_t get_usage() {
        return allocated - freed;
      }
    } memory_stats;

    }

    size_t get_usage() {
      return internal::memory_stats.get_usage();
    }

  }

}

void* operator new(size_t size) {
  benchmark::memory::internal::memory_stats.allocated += size;
  return malloc(size);
}

void operator delete(void* memory, size_t size) {
  benchmark::memory::internal::memory_stats.freed += size;
  free(memory);
}
