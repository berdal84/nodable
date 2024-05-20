# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(../../../scripts/cmake/log.cmake)
ndbl_log_title_header()

set(LIBS ../../../libs)
set(WHEREAMI_SOURCES
        ${LIBS}/whereami/src/whereami.c
        ${LIBS}/whereami/src/whereami.h
)
set(WHEREAMI_INCLUDE
        ${LIBS}/whereami/src/
)
set(RAPIDJSON_INCLUDE
        ${LIBS}/rapidjson/
)
# log options
add_library(
        framework-core
        STATIC
            async.cpp
            log.cpp
            memory/Pool.cpp
            format.cpp
            system.cpp
            reflection/func_type.cpp
            reflection/qword.cpp
            reflection/type.cpp
            reflection/type_register.cpp
            reflection/variant.cpp
            ${WHEREAMI_SOURCES}
        )

target_link_libraries(
        framework-core
        PUBLIC # <-------------- Those will be accessible by other targets if they link headless
            ghc_filesystem     # c++ compatible std::filesystem::path like API
            Observe            # event system using observer/observable pattern.
            xxhash
            glm                # math
)

target_include_directories(
        framework-core
        PUBLIC
            ../
            ${RAPIDJSON_INCLUDE}
            ${WHEREAMI_INCLUDE}
)


# Headless tests (nodable-core)
#------------------------------

add_executable(
        test-framework-core
        reflection/reflection.specs.cpp
        memory/Pool.specs.cpp)
target_link_libraries( test-framework-core PRIVATE gtest_main gtest framework-core )
add_test( NAME test_framework_core COMMAND test-framework-core )

# Benchmarks
#-----------

# string
add_executable       (bench-fw-core-string string.bench.cpp)
target_link_libraries(bench-fw-core-string PRIVATE benchmark::benchmark framework-core )

# Pool
add_executable       (bench-fw-core-Pool memory/Pool.bench.cpp)
target_link_libraries(bench-fw-core-Pool PRIVATE benchmark::benchmark framework-core )


