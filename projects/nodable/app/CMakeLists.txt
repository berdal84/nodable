
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log_title_header()

# add nodable executable
add_executable(
    nodable
        WIN32
        src/main.cpp
    )

target_link_libraries(
    nodable
    PUBLIC
        nodable-gui
)

set_target_properties(nodable PROPERTIES POSITION_INDEPENDENT_CODE FALSE)               # required to run well on recent os (ex: ubuntu)
set_target_properties(nodable PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio

# Installation :
#===============

install( TARGETS nodable RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE DESTINATION .)
