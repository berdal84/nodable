
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log_title_header()

# add executable
set(NAME framework-gui-example)
add_executable(${NAME} WIN32 main.cpp)
target_link_libraries(${NAME} PUBLIC framework-gui)
set_target_properties(${NAME} PROPERTIES POSITION_INDEPENDENT_CODE FALSE)               # required to run well on recent os (ex: ubuntu)
set_target_properties(${NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio

# Installation :
#===============

# install( TARGETS framework-gui-example RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE DESTINATION .)
