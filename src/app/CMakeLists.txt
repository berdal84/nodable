
# Do not use this CMaleLists.txt directly, take the one from base directory.
#---------------------------------------------------------------------------

# App
#====

file( GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp )
file( GLOB HEADERS CONFIGURE_DEPENDS src/*.h )

set( LIBS_DIR ../../libs )

# add sources
add_library(
        nodable_app
        STATIC
            ${SOURCES}
            ${HEADERS}
            ${LIBS_DIR}/imgui/backends/imgui_impl_opengl3.cpp
            ${LIBS_DIR}/imgui/backends/imgui_impl_sdl.cpp
            )

# Include external and libs folders (for headers)
target_include_directories(
        nodable_app
        PUBLIC
            src
            ${PROJECT_BINARY_DIR}/conf
            ${LIBS_INCLUDE_DIRS} )

# Linking
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)

IF (WIN32)
    target_link_libraries(nodable_app LINK_PUBLIC ${SDL2_STATIC} )
ELSE()
    target_link_libraries(nodable_app LINK_PUBLIC ${SDL2_LIBRARIES})
ENDIF()

target_link_libraries(
        nodable_app
        PUBLIC
            Threads::Threads
            nodable_core
            imgui
            imgui_textedit
            imgui_filebrowser
            IconFontCppHeaders
            lodepng
            gl3w
)

# Main
#=====

add_executable( nodable ./src/main.cpp )
target_link_libraries( nodable PUBLIC nodable_app )

# required to run well on recent os (ex: ubuntu)
set_property(TARGET nodable PROPERTY POSITION_INDEPENDENT_CODE FALSE)

# fix working directory for visual studio
set_property(TARGET nodable PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}")

# AFTER BUILD:
#=============

# Copy assets (fonts, scripts, etc..)
add_custom_command(
        TARGET nodable_app PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
        -E copy_directory
        "${NODABLE_APP_ASSETS_ABSOLUTE_DIR}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NODABLE_APP_ASSETS_RELATIVE_DIR}"
)

# Copy SDL2.dll
if(WIN32)
    add_custom_command(
            TARGET nodable_app PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${SDL2_RUNTIME} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    )
endif()

# Installation :
#===============

set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/app/$<CONFIG>)

# Copy DLLs
install( FILES     ${SDL2_RUNTIME}                    DESTINATION ${INSTALL_PATH} )

# Copy assets
install( DIRECTORY ${NODABLE_APP_ASSETS_ABSOLUTE_DIR} DESTINATION ${INSTALL_PATH} )
install( FILES     ${PROJECT_SOURCE_DIR}/CHANGELOG.md DESTINATION ${INSTALL_PATH} )
install( FILES     ${PROJECT_SOURCE_DIR}/README.md    DESTINATION ${INSTALL_PATH} )

# Copy executable
install( TARGETS   nodable RUNTIME                    DESTINATION ${INSTALL_PATH} )
