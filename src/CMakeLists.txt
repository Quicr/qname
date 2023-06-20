add_library(qname
            name.cpp
            namespace.cpp)

target_compile_options(qname
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
        $<$<CXX_COMPILER_ID:MSVC>: >)
set_target_properties(qname
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS OFF)
if(MSVC)
    target_compile_definitions(qname _CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(qname PUBLIC ${PROJECT_SOURCE_DIR}/include)

