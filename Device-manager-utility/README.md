# Device Manager Utility (C++ System Design Demo)

A modular C++ application demonstrating:
- Pure Virtual Interfaces
- Singleton Design Pattern
- STL Containers (map, vector)
- Threading with `std::mutex` & `condition_variable`
- Compile-time and runtime polymorphism
- Static asserts and constexpr
- Exception handling and smart pointers
- Protected Inheritance
- Type Casting (static_cast, dynamic_cast, reinterpret_cast)

## Structure
- `include/` — Header files for each component
- `src/` — Source files for implementation
- `main.cpp` — Ties together all system components

## Build
```bash
mkdir build && cd build
cmake ..
make
./device-manager-utility
