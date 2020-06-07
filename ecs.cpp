#include "ecs.hpp"

#include "storage/tuple_of_vecs.hpp"
#include "runtime/sequential.hpp"

#include <iostream>

using ECS = ecs::EntityComponentSystem<ecs::storage::TupleOfVecs, ecs::runtime::Sequential>;

struct Transform {
  Transform() {
    std::cout << "new transform" << std::endl;
  }

  Transform(const Transform&) {
    std::cout << "copied transform" << std::endl;
  }

  Transform(Transform&&) {
    std::cout << "moved transform" << std::endl;
  }
  int pos = 1337;
};

struct Color {
  Color() {
    std::cout << "new color" << std::endl;
  }

  Color(const Color&) {
    std::cout << "copied color" << std::endl;
  }

  Color(Color&&) {
    std::cout << "moved color" << std::endl;
  }
  int col;
};

class TestSystem {
public:
  TestSystem() {
    std::cout << "new test" << std::endl;
  }

  TestSystem(const TestSystem&) {
    std::cout << "copied test" << std::endl;
  }

  TestSystem(TestSystem&&) {
    std::cout << "moved test" << std::endl;
  }

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
  TwoSystem() {
    std::cout << "new two" << std::endl;
  }

  TwoSystem(const TwoSystem&) {
    std::cout << "copied two" << std::endl;
  }

  TwoSystem(TwoSystem&&) {
    std::cout << "moved two" << std::endl;
  }

  void operator()(const TestSystem& test, const Color& color) const {
    std::cout << "Two" << std::endl;
    std::cout << "test: " << test.get_result() << std::endl;
    std::cout << "col: " << color.col << std::endl;
  }
};

int main() {
  TwoSystem two_sys;
  ECS::Runtime tick(
    TestSystem{},
    two_sys
  );

  for (auto i{0}; i < 2; ++i) tick();

  return 0;
}