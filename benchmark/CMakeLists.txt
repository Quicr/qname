add_executable(qname_benchmark
                name.cpp
                hex_endec.cpp)

target_link_libraries(qname_benchmark PRIVATE qname benchmark::benchmark benchmark::benchmark_main)
target_include_directories(qname_benchmark PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_compile_options(qname_benchmark
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
        $<$<CXX_COMPILER_ID:MSVC>: >)

set_target_properties(qname_benchmark
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS ON)
if(MSVC)
    target_compile_definitions(qname_benchmark _CRT_SECURE_NO_WARNINGS)
endif()

