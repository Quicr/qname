# Test Binary

add_executable(qname_test
                main.cpp
                name.cpp
                namespace.cpp
                hex_endec.cpp)
target_include_directories(qname_test PRIVATE ${DOCTEST_INCLUDE_DIR})

target_link_libraries(qname_test PRIVATE qname doctest::doctest)

target_compile_options(qname_test
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
        $<$<CXX_COMPILER_ID:MSVC>: >)

set_target_properties(qname_test
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS ON)
if(MSVC)
    target_compile_definitions(qname_test _CRT_SECURE_NO_WARNINGS)
endif()

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(qname_test)