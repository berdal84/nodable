
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

nodable_log_title_header()

# checks
nodable_message("Checking ...")
if( NOT NODABLE_CONFIGURED_DIR )
    nodable_err("Variable NODABLE_CONFIGURED_DIR must be set in main CMakeLists.txt")
endif()

# defines
set(NODABLE_APP_ASSETS_DIR          assets)
set(NODABLE_APP_ASSETS_ABSOLUTE_DIR "${CMAKE_CURRENT_LIST_DIR}/${NODABLE_APP_ASSETS_DIR}")

# log variables
nodable_message(" - NODABLE_CONFIGURED_DIR:          ${NODABLE_CONFIGURED_DIR}")
nodable_message(" - NODABLE_APP_ASSETS_DIR:          ${NODABLE_APP_ASSETS_DIR}")
nodable_message(" - NODABLE_APP_ASSETS_ABSOLUTE_DIR: ${NODABLE_APP_ASSETS_ABSOLUTE_DIR}")

# Configure files
configure_file("include/nodable/app/build_info.h.in" "${NODABLE_CONFIGURED_DIR}/app/build_info.h" )

# add nodable executable
add_executable(
    nodable
    WIN32
        src/main.cpp
        src/App.cpp
        src/AppView.cpp
        src/Event.cpp
        src/File.cpp
        src/FileView.cpp
        src/GraphNodeView.cpp
        src/History.cpp
        src/ImGuiEx.cpp
        src/main.cpp
        src/MemberConnector.cpp
        src/MemberView.cpp
        src/NodeConnector.cpp
        src/NodeView.cpp
        src/NodeViewConstraint.cpp
        src/Settings.cpp
        src/View.cpp
        ${PROJECT_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl3.cpp # include backend as-is
        ${PROJECT_SOURCE_DIR}/libs/imgui/backends/imgui_impl_sdl.cpp     #          //
)

target_include_directories(
    nodable
    PRIVATE
        include/
        ${PROJECT_BINARY_DIR}/configured
        ${LIBS_INCLUDE_DIRS} )

nodable_message("Looking for libraries ...")

find_package(Threads REQUIRED)
if (NOT Threads_FOUND)
    nodable_err("Threads not found")
endif()

find_package(OpenGL REQUIRED)
if (NOT OpenGL_FOUND)
    nodable_err("OpenGL not found")
endif()

nodable_message("Threads found: ${Threads_FOUND}")
nodable_message("OpenGL found:  ${OpenGL_FOUND}")

target_link_libraries(
    nodable
    PUBLIC
        nodable-core
    PRIVATE
        ${CMAKE_DL_LIBS}
        IconFontCppHeaders      # Font Awesome
        OpenGL::GL gl3w         # OpenGL and wrangler
        SDL2-static SDL2main    # SDL2
        Threads::Threads
        ImGui
        ImGuiColorTextEdit
        lodepng
        nfd                     # native file dialog - extended
)

# link APPLE specific libraries
IF (APPLE)
    nodable_message("Looking for Apple specific libraries ...")

    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    if (NOT CORE_FOUNDATION_FRAMEWORK)
        nodable_err("CoreFoundation not found")
    endif()

    find_library(COCOA_FRAMEWORK Cocoa)
    if (NOT COCOA_FRAMEWORK)
        nodable_err("Cocoa not found")
    endif()

    nodable_message("CORE_FOUNDATION_FRAMEWORK: ${CORE_FOUNDATION_FRAMEWORK}")
    nodable_message("COCOA_FRAMEWORK:           ${COCOA_FRAMEWORK}")

    target_link_libraries(
        nodable
        PRIVATE
            ${CORE_FOUNDATION_FRAMEWORK}
            ${COCOA_FRAMEWORK} )
ENDIF()

set_target_properties(nodable PROPERTIES POSITION_INDEPENDENT_CODE FALSE)               # required to run well on recent os (ex: ubuntu)
set_target_properties(nodable PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio

# AFTER BUILD:
#=============

# Copy assets (fonts, scripts, etc..)
add_custom_command(
        TARGET nodable PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
        -E copy_directory
        "${NODABLE_APP_ASSETS_ABSOLUTE_DIR}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NODABLE_APP_ASSETS_DIR}"
)

# Installation :
#===============

install( DIRECTORY "${NODABLE_APP_ASSETS_ABSOLUTE_DIR}" DESTINATION . )
install( FILES     "${PROJECT_SOURCE_DIR}/CHANGELOG.md" DESTINATION . )
install( TARGETS   nodable RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE DESTINATION . )
