#include <iostream>

#include <functional>

#include <boost/hana.hpp>
#include <boost/callable_traits.hpp>
#include <boost/mp11.hpp>

#include "ecs.hpp"
#include "storage/tuple_of_vecs.hpp"

namespace hana = boost::hana;
namespace ct = boost::callable_traits;

// using namespace std::literals;
using namespace hana::literals;

struct Transform {
  int pos{1337};
};

struct Color {
  int col;
};

class TestSystem {
public:
  void operator()(const Transform& transform, Color& color) {
    std::cout << "Test" << std::endl;
    std::cout << "tr: " << transform.pos << std::endl;
    color.col += transform.pos;
    ++result;
  }

  const int& get_result() const { return result; }
private:
  int result{5};
};

class TwoSystem {
public:
  void operator()(const Color& color) const {
    std::cout << "Two" << std::endl;
    std::cout << "col: " << color.col << std::endl;
  }
};

template<typename T>
constexpr auto get_argtypes(const std::reference_wrapper<T>& system) {
  return to_hana_tuple<ct::args_t<T>>;
}

template<typename... TSystems>
static constexpr auto component_types = hana::to_set(hana::transform(hana::flatten(hana::make_tuple(to_hana_tuple<ct::args_t<TSystems>>...)), hana::traits::decay));

static_assert(component_types<TestSystem, TwoSystem> == hana::to_set(hana::tuple_t<Transform, Color>));
static_assert(hana::to_tuple(component_types<TestSystem, TwoSystem>) == hana::tuple_t<Transform, Color>);

template<typename... TSystems>
using SystemComponentsStorage = typename decltype(hana::unpack(component_types<TSystems...>, hana::template_<ecs::TupleOfVecs>))::type;

static_assert(hana::type_c<SystemComponentsStorage<TestSystem, TwoSystem>> == hana::type_c<ecs::TupleOfVecs<Transform, Color>>);

namespace ecs {

template<typename... TSystems>
class ECS {
public:
  ECS(TSystems&... systems): _systems(hana::make_tuple(std::reference_wrapper(systems)...)) {};

  void operator()() {
    hana::for_each(_systems, [this](auto& system) {
      auto args = hana::transform(get_argtypes(system), [this](auto argtype) {
        using ComponentType = typename decltype(hana::traits::decay(argtype))::type;
        static_assert(hana::type_c<ComponentType> == hana::type_c<Transform> || hana::type_c<ComponentType> == hana::type_c<Color>);
        auto& vector = _storage.template get_vector<ComponentType>();
        if (vector.size() == 0) vector.emplace_back();
        // reference_wrapper is needed to store the reference in the args container (to later be unpacked into the system call)
        return std::reference_wrapper<ComponentType>(vector[0]);
      });
      hana::unpack(args, system);
    });
  }
private:
  hana::tuple<std::reference_wrapper<TSystems>...> _systems;
  SystemComponentsStorage<TSystems...> _storage;
};

}

/*
template<typename... TSystems>
auto make_ecs(TSystems&... systems) {
  constexpr auto types = hana::transform(hana::tuple_t<TSystems...>, hana::traits::decay);
  static_assert(types[0_c] == hana::type_c<TestSystem>);
  static_assert(types[1_c] == hana::type_c<TwoSystem>);

  static_assert(hana::tuple_t<decltype(systems)...>[0_c] == hana::type_c<TestSystem&>);
  static auto callables = hana::make_tuple(std::reference_wrapper(systems)...);
  // static_assert(hana::type_c<decltype(callables[0_c])> == hana::type_c<TestSystem*>);
  std::cout << "inmake " << &static_cast<TestSystem&>(callables[0_c]) << std::endl;

  constexpr auto params = hana::transform(types, [](auto system) {
    return internal::hana_tuple<ct::args_t<typename decltype(system)::type>>;
  });
  static_assert(params[0_c] == hana::tuple_t<const Transform&, Color&>);
  static_assert(params[1_c] == hana::tuple_t<Color>);

  // constexpr auto consts = hana::transform(types, [](auto system) {
  //   return ct::is_const_member_v<typename decltype(system)::type>;
  // });

  constexpr auto indices = hana::make_range(0_c, hana::length(types));

  // auto params = internal::hana_tuple<ct::args_t<ft>>;

  // return hana::make_tuple(callables, types);

  // auto system_types = hana::tuple_t<TSystems...>;
  // auto component_types = hana::flatten(hana::transform(system_types, [](auto system) {
  //   return hana::transform(internal::hana_tuple<ct::args_t<typename decltype(system)::type>>, hana::traits::decay);
  // }));

  // constexpr auto args = hana::transform(ToHanaTuple<ct::args_t<TestSystem>>::tuple, hana::traits::decay);
  // constexpr auto arg_types = hana::transform(internal::hana_tuple<ct::args_t<TestSystem>>, hana::traits::decay);

  // constexpr auto vector_of_vectors = hana::unpack(hana::transform(args, hana::template_<std::vector>), hana::template_<std::tuple>);

  // static_assert(vector_of_vectors == hana::type_c<std::tuple<std::vector<Transform>, std::vector<Color>>>);
  // static_assert(hana::type_c<ct::return_type_t<TestSystem>> == hana::type_c<void>);

  return [&]() {
    hana::for_each(indices, [&](auto index) {
      auto args = hana::transform(params[index], [](auto param) {
        return typename decltype(hana::traits::decay(param))::type{};
      });
      hana::unpack(args, callables[index]);
    });
  };
}
*/

using EntityHandle = uint64_t;

template<typename... TComponents>
EntityHandle new_entity(TComponents&&... components) {
  constexpr auto component_types = hana::tuple_t<TComponents...>;
  return 5;
}

// static_assert(aggregate_component_types(TestSystem{}, TwoSystem{}) == hana::tuple_t<Transform, Color, Transform>);

// using VecOfVecs = decltype(vector_of_vectors)::type;

int main() {
  TestSystem test_sys;
  TwoSystem two_sys;

  // auto ecs = make_ecs(test_sys, two_sys);
  ecs::EntityComponentSystem<ecs::TupleOfVecs>::Runtime ecs(test_sys, two_sys);
  std::cout << test_sys.get_result() << std::endl;
  ecs();
  std::cout << test_sys.get_result() << std::endl;
  ecs();
  // std::cout << "params: " << hana::length(args) << std::endl;

  // VecOfVecs vecs;
  // std::get<std::vector<Transform>>(vecs).push_back(Transform{5});
  // std::cout << std::get<std::vector<Transform>>(vecs)[0].pos << std::endl;

  return 0;
}