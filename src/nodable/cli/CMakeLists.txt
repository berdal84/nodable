
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log_title_header()

# checks
ndbl_log("Checking ...")
if( NOT NDBL_CONFIGURED_DIR )
    ndbl_err("Variable NDBL_CONFIGURED_DIR must be set in main CMakeLists.txt")
endif()

# add nodable executable
add_executable(
    nodable-cli
        main.cpp
        CLI.cpp)

find_package(Threads REQUIRED)
if (NOT Threads_FOUND)
    ndbl_err("Threads not found")
endif()

target_link_libraries(
    nodable-cli
    PRIVATE
        nodable-core
        Threads::Threads
)

set_target_properties(nodable-cli PROPERTIES POSITION_INDEPENDENT_CODE FALSE)               # required to run well on recent os (ex: ubuntu)
set_target_properties(nodable-cli PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio
set_target_properties(nodable-cli PROPERTIES OUTPUT_NAME "cli" )

# AFTER BUILD:
#=============

# Installation :
#===============

install( TARGETS nodable-cli DESTINATION bin RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE  )
