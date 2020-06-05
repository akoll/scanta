#include "ecs.hpp"
#include "storage/tuple_of_vecs.hpp"

#include <iostream>

struct Transform {
  int pos = 1337;
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
  void operator()(const TestSystem& test, const Color& color) const {
    std::cout << "Two" << std::endl;
    std::cout << "test: " << test.get_result() << std::endl;
    std::cout << "col: " << color.col << std::endl;
  }
};

int main() {
  TestSystem test_sys;
  TwoSystem two_sys;

  ecs::EntityComponentSystem<ecs::storage::TupleOfVecs>::SequentialRuntime ecs(
    test_sys,
    two_sys
  );
  ecs();
  ecs();

  return 0;
}