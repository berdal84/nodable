
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log_title_header()

# add imgui executable
add_library(
    framework-gui
        ${PROJECT_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl3.cpp # include backend as-is
        ${PROJECT_SOURCE_DIR}/libs/imgui/backends/imgui_impl_sdl.cpp     #          //
        src/App.cpp
        src/AppView.cpp
        src/EventManager.cpp
        src/FontManager.cpp
        src/ImGuiEx.cpp
        src/TextureManager.cpp
        src/View.cpp)

target_include_directories(
    framework-gui
    PUBLIC
        include)

ndbl_log("Looking for libraries ...")

find_package(Threads REQUIRED)
if (NOT Threads_FOUND)
    ndbl_err("Threads not found")
endif()

find_package(OpenGL REQUIRED)
if (NOT OpenGL_FOUND)
    ndbl_err("OpenGL not found")
endif()

ndbl_log("Threads found: ${Threads_FOUND}")
ndbl_log("OpenGL found:  ${OpenGL_FOUND}")

target_link_libraries(
    framework-gui
    PUBLIC
        ${CMAKE_DL_LIBS}
        framework-core
        IconFontCppHeaders      # Font Awesome
        ImGui
        ImGuiColorTextEdit
        OpenGL::GL gl3w         # OpenGL and wrangler
        SDL2-static SDL2main    # SDL2
        Threads::Threads
        lodepng
        nfd                     # native file dialog - extended
)

# link APPLE specific libraries
IF (APPLE)
    ndbl_log("Looking for Apple specific libraries ...")

    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    if (NOT CORE_FOUNDATION_FRAMEWORK)
        ndbl_err("CoreFoundation not found")
    endif()

    find_library(COCOA_FRAMEWORK Cocoa)
    if (NOT COCOA_FRAMEWORK)
        ndbl_err("Cocoa not found")
    endif()

    ndbl_log("CORE_FOUNDATION_FRAMEWORK: ${CORE_FOUNDATION_FRAMEWORK}")
    ndbl_log("COCOA_FRAMEWORK:           ${COCOA_FRAMEWORK}")

    target_link_libraries(
        framework-gui
        PRIVATE
            ${CORE_FOUNDATION_FRAMEWORK}
            ${COCOA_FRAMEWORK} )
ENDIF()

set_target_properties(framework-gui PROPERTIES POSITION_INDEPENDENT_CODE FALSE)               # required to run well on recent os (ex: ubuntu)
set_target_properties(framework-gui PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio
