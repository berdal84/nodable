
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log_title_header()

# add nodable executable
add_executable(
    nodable-app
        WIN32
        main.cpp
    )

target_link_libraries(
    nodable-app
    PUBLIC
        nodable-gui
)

set_target_properties(nodable-app PROPERTIES POSITION_INDEPENDENT_CODE FALSE)               # required to run well on recent os (ex: ubuntu)
set_target_properties(nodable-app PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio
set_target_properties(nodable-app PROPERTIES OUTPUT_NAME "nodable" )

set(README "${PROJECT_SOURCE_DIR}/README.md")
set(LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

configure_file("${README}" . COPYONLY)
configure_file("${LICENSE}" . COPYONLY)

# Installation :
#===============

install(TARGETS nodable-app DESTINATION . RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE )
install(DIRECTORY "${CMAKE_BINARY_DIR}/assets" DESTINATION .)
install(FILES ${LICENSE} ${README} DESTINATION .)