add_executable(${PROJECT_NAME}_test
                main.cpp
                name.cpp
                namespace.cpp
                hex_endec.cpp)
target_include_directories(${PROJECT_NAME}_test PRIVATE ${DOCTEST_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}_test PRIVATE qname doctest::doctest)

target_compile_options(${PROJECT_NAME}_test
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
        $<$<CXX_COMPILER_ID:MSVC>: >)

set_target_properties(${PROJECT_NAME}_test
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS ON)
if(MSVC)
    target_compile_definitions(${PROJECT_NAME}_test _CRT_SECURE_NO_WARNINGS)
endif()

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME}_test)