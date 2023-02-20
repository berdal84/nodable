
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log_title_header()

# add nodable executable
add_executable(
    framework-gui-example
        WIN32
        src/main.cpp
    )

target_link_libraries(
    framework-gui-example
    PUBLIC
        framework-gui
        gl3w
)

set_target_properties(framework-gui-example PROPERTIES POSITION_INDEPENDENT_CODE FALSE)               # required to run well on recent os (ex: ubuntu)
set_target_properties(framework-gui-example PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio

# Installation :
#===============

# install( TARGETS framework-gui-example RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE DESTINATION .)
