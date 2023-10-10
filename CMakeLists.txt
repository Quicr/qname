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
        VERSION 0.4.1
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

add_library(${PROJECT_NAME} INTERFACE)
add_library(quicr::name ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS ON)

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS ${PROJECT_NAME} EXPORT quicr::name)

#=============================================================================#
# Build tests/benchmarks
#=============================================================================#

if(${PROJECT_NAME}_BUILD_TESTS OR ${PROJECT_NAME}_BUILD_BENCHMARKS)
    include(FetchContent)
    if(${PROJECT_NAME}_BUILD_TESTS)
        FetchContent_Declare(
            doctest
            GIT_REPOSITORY https://github.com/doctest/doctest.git
        )
        FetchContent_MakeAvailable(doctest)

        include(CTest)
        enable_testing()
        add_subdirectory(test)
    endif()

    if(${PROJECT_NAME}_BUILD_BENCHMARKS)
        add_subdirectory(benchmark)
    endif()
endif()
