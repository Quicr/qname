cmake_minimum_required(VERSION 3.13)

# Build tests by default only if not a sub-project
if(DEFINED PROJECT_NAME)
    option(qname_BUILD_TESTS "Build tests for qname" OFF)
    option(qname_BUILD_BENCHMARKS "Build benchmarks for qname" OFF)
else()
    option(qname_BUILD_TESTS "Build tests for qname" ON)
    option(qname_BUILD_BENCHMARKS "Build benchmarks for qname" ON)
endif()

project(qname
        VERSION 1.0.0.0
        DESCRIPTION "Quicr Name library"
        LANGUAGES CXX)

option(CLANG_TIDY "Perform linting with clang-tidy" OFF)

if(CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY  ${CLANG_TIDY_EXE})
    else()
        message(WARNING "clang-tidy requested, but not found")
    endif()
endif()

add_library(qname INTERFACE)
add_library(quicr::name ALIAS qname)
target_include_directories(qname INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS qname EXPORT quicr::name)


#=============================================================================#
# Build tests/benchmarks
#=============================================================================#

if(BUILD_TESTING OR BUILD_BENCHMARKING)
    include(FetchContent)
endif()

if(BUILD_TESTING AND qname_BUILD_TESTS)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
    )
    FetchContent_MakeAvailable(doctest)

    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()

if (BUILD_BENCHMARKING AND qname_BUILD_BENCHMARKS)
    set(BENCHMARK_USE_BUNDLED_GTEST OFF)
    set(BENCHMARK_ENABLE_TESTING OFF)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(benchmark)

    add_subdirectory(benchmark)
endif()
