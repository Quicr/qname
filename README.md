# qname

[![CMake](https://github.com/Quicr/qname/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/Quicr/qname/actions/workflows/cmake.yml)

A library for the QUICR Name and Namespace type definitions.

## Including in your Project

### CMake

To include qname in your project, simply add `qname` or `quicr::name` to your `target_link_libraries`:
```cmake
target_link_libraries(project_name PUBLIC/PRIVATE/INTERFACE qname)

# Or
target_link_libraries(project_name PUBLIC/PRIVATE/INTERFACE quicr::name)
```

## Building

To build qname on it's own, simply use
```bash
cmake -B build
cmake --build build
```

To build with tests, simply add the `BUILD_TESTING` flag during configuration:
```bash
cmake -B build -DBUILD_TESTING=ON
```

To build with benchmarking, simply add the `BUILD_BENCHMARKING` flag during configuration:
```bash
cmake -B build -DBUILD_BENCHMARKING=ON
```

Example of a full build configuration, including testing and benchmarking:
```bash
cmake -B build -DBUILD_TESTING=ON -DBUILD_BENCHMARKING=ON
cmake --build build
```
