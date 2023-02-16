# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(../../../cmake/log.cmake)
ndbl_log_title_header()

# log options
add_library(
        framework-core
        STATIC
        src/log.cpp
        src/string.cpp
        src/system.cpp
            src/reflection/func_type.cpp
            src/reflection/qword.cpp
            src/reflection/type.cpp
            src/reflection/type_register.cpp
            src/reflection/variant.cpp)

target_link_libraries(
        framework-core
        PUBLIC # <-------------- Those will be accessible by other targets if they link headless
            ghc_filesystem     # c++ compatible std::filesystem::path like API
            Observe            # event system using observer/observable pattern.
            rapidjson          # read/write json
        PRIVATE
            whereami           # get absolute executable/dll directory path

)

target_include_directories(
        framework-core
        PUBLIC
            include
)
