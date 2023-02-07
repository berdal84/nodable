
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log_title_header()

# checks
ndbl_log("Checking ...")
if( NOT NDBL_CONFIGURED_DIR )
    ndbl_err("Variable NDBL_CONFIGURED_DIR must be set in main CMakeLists.txt")
endif()

# defines
set(NDBL_APP_ASSETS_DIR assets)
set(NDBL_APP_ASSETS_ABSOLUTE_DIR "${CMAKE_CURRENT_LIST_DIR}/${NDBL_APP_ASSETS_DIR}")

# log variables
ndbl_log(" - NDBL_CONFIGURED_DIR:          ${NDBL_CONFIGURED_DIR}")
ndbl_log(" - NDBL_APP_ASSETS_DIR:          ${NDBL_APP_ASSETS_DIR}")
ndbl_log(" - NDBL_APP_ASSETS_ABSOLUTE_DIR: ${NDBL_APP_ASSETS_ABSOLUTE_DIR}")

# Configure files
configure_file("include/ndbl/gui/build_info.h.in" "${NDBL_CONFIGURED_DIR}/gui/build_info.h" )

add_library(
    nodable-gui
        src/App.cpp
        src/AppView.cpp
        src/File.cpp
        src/FileView.cpp
        src/GraphNodeView.cpp
        src/History.cpp
        src/NodeConnector.cpp
        src/NodeView.cpp
        src/PropertyConnector.cpp
        src/PropertyView.cpp
        src/Settings.cpp
        src/ViewConstraint.cpp
)

target_link_libraries(
    nodable-gui
    PUBLIC
        gl3w
        framework-gui
        nodable-core
    PRIVATE
        ImGuiColorTextEdit
)

target_include_directories(
    nodable-gui
    PUBLIC
        include
        ${PROJECT_BINARY_DIR}/configured
)


# AFTER BUILD:
#=============

# Copy assets (fonts, scripts, etc..)
add_custom_command(
        TARGET nodable-gui PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
        -E copy_directory
        "${NDBL_APP_ASSETS_ABSOLUTE_DIR}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NDBL_APP_ASSETS_DIR}"
)

# Installation :
#===============

install( DIRECTORY "${NDBL_APP_ASSETS_ABSOLUTE_DIR}" DESTINATION .)
install( FILES     "../../../CHANGELOG.md" DESTINATION .)
