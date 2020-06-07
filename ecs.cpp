#include "ecs.hpp"

#include "storage/tuple_of_vecs.hpp"
#include "runtime/sequential.hpp"

#include <iostream>

using ECS = ecs::EntityComponentSystem<ecs::storage::TupleOfVecs, ecs::runtime::Sequential>;

struct Transform {
  int pos = 1337;
};

struct Color {
  int col;
};

struct Flammable {
  bool on_fire;
};

class TestSystem {
public:
  void operator()(ECS::Entity entity, const Transform& transform, Color& color) {
    std::cout << "Test " << entity << std::endl;
    std::cout << "tr: " << transform.pos << " " << color.col << std::endl;
    color.col += transform.pos;
    ++result;
  }

  const int& get_result() const { return result; }
private:
  int result{5};
};

class TwoSystem {
public:
  void operator()(const TestSystem& test, const Color& color) const {
    std::cout << "Two" << std::endl;
    std::cout << "test: " << test.get_result() << std::endl;
    std::cout << "col: " << color.col << std::endl;
  }
};

int main() {
  ECS::Runtime tick(
    TestSystem{},
    TwoSystem{}
  );

  for (auto i{0}; i < 2; ++i) tick();

  return 0;
}