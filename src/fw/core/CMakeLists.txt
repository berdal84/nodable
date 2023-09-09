# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(../../../scripts/cmake/log.cmake)
ndbl_log_title_header()

# log options
add_library(
        framework-core
        STATIC
            log.cpp
            format.cpp
            system.cpp
            Pool.cpp
            reflection/func_type.cpp
            reflection/qword.cpp
            reflection/type.cpp
            reflection/type_register.cpp
            reflection/variant.cpp async.cpp)

target_link_libraries(
        framework-core
        PUBLIC # <-------------- Those will be accessible by other targets if they link headless
            ghc_filesystem     # c++ compatible std::filesystem::path like API
            Observe            # event system using observer/observable pattern.
            rapidjson          # read/write json
            xxhash
        PRIVATE
            whereami           # get absolute executable/dll directory path

)

target_include_directories(
        framework-core
        PUBLIC
            ../
)


# Headless tests (nodable-core)
#------------------------------

add_executable(
        test-framework-core
        reflection/reflection.specs.cpp
        Pool.specs.cpp)
target_link_libraries( test-framework-core PRIVATE gtest_main gtest framework-core )
add_test( COMMAND test-framework-core )

# Benchmarks
#-----------

# string
add_executable       (bench-fw-core-string string.bench.cpp)
target_link_libraries(bench-fw-core-string PRIVATE benchmark::benchmark framework-core )


