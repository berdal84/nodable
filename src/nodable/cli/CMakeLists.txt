
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log_title_header()

# checks
ndbl_log("Checking ...")
if( NOT NDBL_CONFIGURED_DIR )
    ndbl_err("Variable NDBL_CONFIGURED_DIR must be set in main CMakeLists.txt")
endif()

# add nodable executable
add_executable(
    cli
        src/main.cpp
        src/CLI.cpp)

find_package(Threads REQUIRED)
if (NOT Threads_FOUND)
    ndbl_err("Threads not found")
endif()

target_link_libraries(
    cli
    PRIVATE
        nodable-core
        Threads::Threads
)

target_include_directories(
        cli
        PRIVATE
        include
)
set_target_properties(cli PROPERTIES POSITION_INDEPENDENT_CODE FALSE)               # required to run well on recent os (ex: ubuntu)
set_target_properties(cli PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio

# AFTER BUILD:
#=============

# Installation :
#===============

install( TARGETS  cli RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE DESTINATION bin )
