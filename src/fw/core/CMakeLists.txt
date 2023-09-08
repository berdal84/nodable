# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(../../../cmake/log.cmake)
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

# Declare our main executable
add_executable(
        framework-core-tests
        reflection/reflection.specs.cpp
        Pool.specs.cpp)

# Link libraries
target_link_libraries(
        framework-core-tests
        PRIVATE
        gtest_main
        gtest
        framework-core
)

# Benchmarks
#-----------

# Declare our main executable
add_executable(
        framework-core-benchmark
        benchmark.cpp
)

# Link libraries
target_link_libraries(
        framework-core-benchmark
        PRIVATE
        benchmark::benchmark # google's
        framework-core
)

# Add test executables
#---------------------
cmake_policy(SET CMP0110 NEW) # Allow spaces in test names
add_test( NAME "Framework Core" COMMAND framework-core-tests )


