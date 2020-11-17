# scanta
> Configurable header-only Entity-Component-System library.

## Design philosophy
`scanta` (/ˈsen(t)ə/) is an Entity-Component-System (ECS) library catered towards game developers.  
It is designed to be highly customizable without sacrificing perfomance. Customization is done through selection _design options_ for certain _design choices_.  
To achieve this, a scaffold implementation is employed with slots for multiple design choices. The scaffold contains any functionality that is universal to all design options.  
The capability of using different design options shall have the least possible impact on the application's visible behavior, compared to a library with a pre-selected design option for each design choice.  
Because of this, the design option slots are implemented as template parameters which then allow plugging in different option implementations of some design choice. Because templates are resolved at compile-time, this avoids the performance-loss incurred by using vtable-based polymorphism.  
Due to its heavy usage of templates and compile-time calculations (entity layouts and execution schedules are inferred statically), `scanta` is a header-only library.

## Documentation
Source code documentation is done within the source files using a [Doxygen](https://www.doxygen.nl/) format. Run `doxygen` in the repository root to generate documentation pages inside the `doc` directory.  
More implementation details are also commented within the source code that are not included in the Doxygen pages.
## Dependencies

For compiling library headers:
* [gcc](https://gcc.gnu.org/) (tested on 10.2.0-2) with [OpenMP](https://www.openmp.org/) support
* [clang](https://clang.llvm.org/) is [not supported](http://clang.llvm.org/cxx_status.html) as of now, because it does not support [custom class instances as non-type template parameters](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0732r2.pdf)
* [boost libraries](https://www.boost.org/) ([hana](http://boostorg.github.io/hana/), [callable_traits](https://www.boost.org/doc/libs/1_67_0/libs/callable_traits/doc/html/index.html)) (tested on 1.72.0-2)

When compiling, make sure to include the `src/` directory.  

For running benchmarks:
* [python](https://www.python.org/) (tested on 3.8.5)
* [GNU make](https://www.gnu.org/software/make/)
* [GNU time](https://www.gnu.org/software/time/) (tested on 1.9-3)
* [perf](https://perf.wiki.kernel.org/index.php/Main_Page)
* [SDL2](https://www.libsdl.org/) (tested on 2.0.12-2)
* [LaTeX](https://www.latex-project.org/) (tested on texlive 2020.54586-5)

## Getting started
To get started, include the necessary library headers and set up configuration. A `ecs.hpp` convenience header is provided to include all options.
```cpp
#include <scanta/ecs.hpp>

// Set up an ECS instance, with a contiguous _tuple of vectors_ storage and a _parallel_ scheduler.
using ECS = scanta::EntityComponentSystem<
  scanta::storage::TupleOfVectors,
  scanta::scheduler::Parallel
>;
```

The two template parameters passed into the `EntityComponentSystem` are the _storage_ and the _scheduler_ used.
* The _storage_ is responsible for storing entity and component data and does so in a certain fashion. The available options shipped by default are:
  * `TupleOfVectors`. This storage stores component data of the same type contiguously and adjacently.
  * `VectorOfTuples`. This storage stores component data attached to the same entity contiguously and adjacently.
  * `Scattered`. This storage stores entity and component data in dynamically allocated and fragmented heap locations. This storage option is further configurable.
* The _scheduler_ mandates how systems are scheduled and executed at run-time.
  * `Sequential`. This scheduler executes every system one after the other in the order they are registered in the scene.
  * `Parallel`. This scheduler determines dependencies between systems at compile-time and infers an execution schedule where compatible systems are run concurrently.

See the library documentation for more information on each of these options.  

## Defining a scene

We can now start to define component types and systems that shall later run within our scene.  

### Components
_Components_ are just regular `struct`s:
```cpp
struct Position {
  float[2] value;
};

struct Flammable {
  bool on_fire = false;
  float fire_resistance = 0.1f;
};

struct Appearance {
  uint8_t color[3];
  float radius;
};
```

### Systems
_Systems_ can be any callable object.  
The _dependencies_ of a system are defined by its function parameters. All of the component types specified in the list of parameters need to be attached to an entity for the system to be called with that entity.
```cpp
// Systems can be class instances:
class RenderSystem {
public:
  RenderSystem(Context context) : context(context) {}

  void operator()(const Position& position, const Appearance& appearance) {
    // Draw the entity to the screen.
    Screen::draw_circle(position.value, appearance.radius, appearance.color);
  }
private:
  Context context;
} render_system;

// Systems can be lambdas:
auto fire_system = [](Flammable& flammable) {
  if (flammable.fire_resistance < 0.5f)
    flammable.on_fire = true;
};

// Systems can be namespace functions:
void tint_system(const Flammable& flammable, Appearance& appearance) {
  if (flammable.on_fire) appearance.color[0] = 0xff;
}
```

Systems may also contain their own state:
```cpp
class CountingSystem {
public:
  operator()() {
    // Increment internal state.
    ++count;
  }
private:
  int count = 0;
};
```
When system state is mutated in the execution function, it can not be marked `const` anymore. `const` systems are executed with `inner parallelism`, meaning that they are applied to all matching entities concurrently:
```cpp
class FireFighter {
public:
  void operator()(Flammable& flammable) const {
    // This function is concurrently called by the library for each individual entity.
    flammable.on_fire = false;
  }
};
```

Not all system parameters are component types. Some special exceptions exist:
* Parameters of type `ECS::Entity` get passed the entity ID:
```cpp
[](ECS::Entity entity, const Flammable&) {
  std::cout << "Entity " << entity << " is flammable." << std::endl;
}
```
* Parameters of type `float` and `double` get passed the _delta time_, the time in seconds since the last frame executed:
```cpp
[](float delta_time) {
  std::cout << "The last frame was " << delta_time << " seconds ago." << std::endl;
}
```
* Systems can also depend on one another:
```cpp
struct Alice {
  void operator()() const {
    if (secret_number)
      std::cout << "Message received." << std::endl;
  }
};

struct Bob {
  int secret_number = 0;
  void operator()(Alice& alice) const {
    alice.secret_number = 5;
  }
};

struct Eve {
  void operator()(const Alice& alice) const {
    std::cout << "The number is " << alice.secret_number << std::endl;
  }
};
```

### Runtime manager
Sometimes, a system may want to perform certain operations directly on the ECS scene. Common such operations include:
* Activating or deactivating systems
* Getting information about the scene, e.g., the number of active entities
* Deferring a critical operation to the end of the frame

These operations are exposed to the systems via a runtime _manager_. To access it, the system must return a callable object (typically a lambda) which is immediately called by the scheduler with the manager passed in:
```cpp
auto corpse_remover(ECS::Entity entity, const Hitpoints& hp) const {
  return [&](auto& manager) {
    manager.defer([]() {
      // Do something critical:
      std::filesystem::permissions("kek.txt", 0777);
    });

    // `get_entity_count` is processed immediately.
    if (manager.get_entity_count() > 100) {
      // `remove_entity` will automatically be deferred.
      if (hp.value <= 0) manager.remove_entity(entity);
    }
  };
}
```
If an operation done by a system is not parallelizable, but only conflicts with other system invocations, it does not need to be deferred. Outer parallelism may still be used, but inner parallelism can't. To prevent the scheduler from applying inner parallelism, simply omit the `const` qualifier from the function declaration:
```cpp
class ParSystem {
  void operator()(const Flammable&) const {
    // This function will be reentered concurrently, because it is marked `const`.
  }
};

class SeqSystem {
  void operator()(const Flammable&) { // <- no `const`
    // This function will not be called concurrently, even though no formal dependencies exist between invocations.
  }
};
```

### Scene
To start simulation, instantiate a _scene_ and register the scene's systems.
The systems are to be passed in as rvalue-references, move your system if need be.
```cpp
ECS::Scene scene(
  ParSystem{},
  RenderSystem(context),
  std::move(corpse_remover)
);
```
Component types do not need to be specified and are inferred from the union of all registered systems' component dependencies.

To execute each system once, run the `update` function of the scene:
```cpp
// Update indefinitely.
while (true)
  scene.update();
```

The execution of each system in the scene is done such that the observable behavior is the same as if they executed sequentially (as done in the `Sequential` scheduler). The `Parallel` scheduler will infer a schedule to allow this.  
Keep in mind that this means that if results from another system are depended on in some system, the results present are those from the last frame:
```cpp
class SystemA {
public:
  operator()() {
    ++value;
  };

  int get_result() {
    return value;
  }
private:
  int value = 0;
};

class SystemB {
public:
  operator()(const SystemA system_a) const {
    std::cout << system_a.get_result() << std::endl;
  }
};

ECS::Scene scene(
  SystemB{}, // SystemB is registered before SystemA. Thus, SystemA's results from the last frame will be accessed.
  SystemA{}
);
```
