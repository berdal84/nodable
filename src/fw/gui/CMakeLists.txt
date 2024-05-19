
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log_title_header()

# add imgui executable
add_library(
    framework-gui
        Action.cpp
        ActionManager.cpp
        ActionManagerView.cpp
        App.cpp
        AppView.cpp
        EventManager.cpp
        FontManager.cpp
        ImGuiEx.cpp
        TextureManager.cpp
        View.cpp)

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
        ImGuiColorTextEdit
        OpenGL::GL gl3w         # OpenGL and wrangler
        SDL2-static SDL2main    # SDL2
        Threads::Threads
        lodepng
        nfd                     # native file dialog - extended
        freetype                # because: https://github.com/ocornut/imgui/tree/master/misc/freetype
    PRIVATE
        ImGui
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

# define assets
set(ASSETS
        "assets/fonts/CenturyGothic.ttf"
        "assets/fonts/fa-solid-900.ttf"
        )

# copy each file (will dirty the build when file changes)
foreach(EACH_FILE ${ASSETS})
    configure_file("${EACH_FILE}" "${CMAKE_BINARY_DIR}/${EACH_FILE}" COPYONLY)
endforeach()