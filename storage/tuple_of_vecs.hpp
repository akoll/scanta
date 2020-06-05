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
      [](auto...){}((std::get<std::vector<TComponents>>(_vectors).resize(1), 0)...);
    }

    template<typename TComponent>
    TComponent& getComponent(Entity entity) {
      return std::get<std::vector<TComponent>>(_vectors)[entity];
    }
  private:
    std::tuple<std::vector<TComponents>...> _vectors;
  };

  }

}