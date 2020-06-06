#pragma once

#include <tuple>
#include <vector>

namespace ecs {
  namespace storage {

  template<typename... TComponents>  
  class TupleOfVecs {
  public:
    using Entity = size_t;

    // TODO: REMOVE
    TupleOfVecs() {
      [](auto...){}((std::get<std::vector<TComponents>>(_vectors).resize(2), 0)...);
      size = 2;
    }

    template<typename TComponent>
    TComponent& get_component(Entity entity) {
      return std::get<std::vector<TComponent>>(_vectors)[entity];
    }

    template<typename... TRequiredComponents>
    void for_entities_with(auto callable) {
      for (size_t i{0}; i < size; ++i)
        callable(Entity{i});
    }

  private:
    size_t size;
    std::tuple<std::vector<TComponents>...> _vectors;
  };

  }

}