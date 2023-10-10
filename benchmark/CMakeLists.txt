find_package(benchmark REQUIRED)
if (NOT benchmark_FOUND)
    set(BENCHMARK_USE_BUNDLED_GTEST OFF)
    set(BENCHMARK_ENABLE_TESTING OFF)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(benchmark)
endif()

add_executable(${PROJECT_NAME}_benchmark
                name.cpp
                namespace.cpp
                hex_endec.cpp)

target_link_libraries(${PROJECT_NAME}_benchmark PRIVATE qname benchmark::benchmark benchmark::benchmark_main)
target_include_directories(${PROJECT_NAME}_benchmark PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_compile_options(${PROJECT_NAME}_benchmark
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
        $<$<CXX_COMPILER_ID:MSVC>: >)

set_target_properties(${PROJECT_NAME}_benchmark
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS ON)
if(MSVC)
    target_compile_definitions(${PROJECT_NAME}_benchmark _CRT_SECURE_NO_WARNINGS)
endif()

