
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

nodable_log_title_header()

# checks
nodable_message("Checking ...")
if( NOT NODABLE_CONFIGURED_DIR )
    nodable_err("Variable NODABLE_CONFIGURED_DIR must be set in main CMakeLists.txt")
endif()

# add nodable executable
add_executable(
    nodable-cli
    WIN32
        src/main.cpp
        src/CLI.cpp)

target_link_libraries(
    nodable-cli
    PRIVATE
        nodable-core
)

target_include_directories(
        nodable-cli
        PRIVATE
            include
)
set_target_properties(nodable-cli PROPERTIES POSITION_INDEPENDENT_CODE FALSE)               # required to run well on recent os (ex: ubuntu)
set_target_properties(nodable-cli PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio

# AFTER BUILD:
#=============

# Installation :
#===============

install( FILES     "${PROJECT_SOURCE_DIR}/CHANGELOG.md" DESTINATION . )
install( TARGETS  nodable-cli RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE DESTINATION . )
