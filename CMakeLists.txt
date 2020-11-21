
cmake_minimum_required (VERSION 3.14)

# Get architecture
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(ARCHITECTURE "x64")
else ()
  set(ARCHITECTURE "x86")
endif ()

# General :
#==========

# Declare project name
project("Nodable" VERSION "0.7.0")
set(PROJECT_ASSETS_DIR "assets")

# Configure headers
configure_file("sources/Core/Config.h.in" "configured/Config.h" )

include(CheckPIESupported) 
check_pie_supported() # avoid "unable to run shared libraries" under linux

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# If a default install prefix has not been set, we set it.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/install" CACHE PATH "..." FORCE)
endif()

# Find packages
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

IF (WIN32)
  # We store the SDL2 binaries inside the repo.
  find_package(SDL2 REQUIRED HINTS "./libs/SDL2")
ELSE()
  find_package(SDL2 REQUIRED)
ENDIF()

# reflexion framework
find_package(MIRROR REQUIRED HINTS "./extern/mirror")

# Add additionnal source directories
aux_source_directory( "./extern/imgui" IMGUI)
aux_source_directory( "./extern/ImGuiColorTextEdit" TEXTEDIT)
aux_source_directory( "./libs/gl3w/GL" GL3W)
aux_source_directory( "./libs/lodepng" LODE_PNG)

# Get all files in sources directory recursively
file(GLOB_RECURSE NODABLE_SRC "${CMAKE_CURRENT_LIST_DIR}/sources/**/*")

# Create a variable with all sources
set(SOURCES
  ${NODABLE_SRC}
  ${GL3W}  
  ${TEXTEDIT}
  ${IMGUI}
  ${MIRROR_SOURCES}
  ${LODE_PNG}
  "./extern/imgui/backends/imgui_impl_opengl3.cpp"
  "./extern/imgui/backends/imgui_impl_sdl.cpp"
  )

# Group the previous files using the folder tree 
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${SOURCES})

# Include nodable folders (for headers)
include_directories(
	"./sources"
	"./sources/Core"
	"./sources/Test"
	"./sources/Node"
	"./sources/Component"
	"./sources/Language")

# Include external and libs folders (for headers)
include_directories(
    ${PROJECT_BINARY_DIR}/configured
    "./extern"
    "./extern/imgui"
    "./extern/ImGuiColorTextEdit"
    "./extern/IconFontCppHeaders"
    "./extern/imgui-filebrowser"
    "./libs"
    ${SDL2_INCLUDE_DIRS}
    "./libs/gl3w/GL"
    "./libs/gl3w"
    "./libs/glfw/include/GLFW"
    "./libs/lodepng"
    ${MIRROR_INCLUDE_DIRS}
)

# Nodable Library:
#=================

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)

# Add a nodable library to build it once for the app and tests
add_library(NodableLib ${SOURCES})

# link SDL
IF (WIN32)
    target_link_libraries(NodableLib LINK_PUBLIC ${SDL2_STATIC} )
ELSE()
    target_link_libraries(NodableLib PUBLIC ${SDL2_LIBRARIES})
ENDIF()

# link other libs
target_link_libraries(NodableLib PUBLIC ${OPENGL_LIBRARY} ${CMAKE_DL_LIBS} Threads::Threads)

# Nodable Application:
#=====================

# add sources
add_executable (Nodable "sources/main.cpp")

# link with lib
target_link_libraries(Nodable PRIVATE NodableLib)

# required to run well on recent os (ex: ubuntu)
set_property(TARGET Nodable PROPERTY POSITION_INDEPENDENT_CODE FALSE)

# fix working directory for visual studio
set_property(TARGET Nodable PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}")

# AFTER BUILD:
#=============

# Copy assets (fonts, scripts, etc..)
add_custom_command(
    TARGET Nodable PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_ASSETS_DIR}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_ASSETS_DIR}
)

# Copy SDL2.dll
IF (WIN32)
  add_custom_command(
    TARGET Nodable PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${SDL2_RUNTIME}
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  )
ENDIF()

# Installation :
#===============

set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/$<CONFIG>)

# Copy DLLs
install(FILES ${SDL2_RUNTIME} DESTINATION ${INSTALL_PATH})

# Copy assets
install(DIRECTORY ${PROJECT_ASSETS_DIR} DESTINATION ${INSTALL_PATH} )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md DESTINATION ${INSTALL_PATH} )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.md DESTINATION ${INSTALL_PATH} )

# Copy executable
install(TARGETS Nodable RUNTIME DESTINATION ${INSTALL_PATH})

# Testing (using google test)
#============================

enable_testing()

# add gtest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(libs/googletest-release-1.10.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# find test sources
file(GLOB_RECURSE TEST_SRC "${CMAKE_CURRENT_LIST_DIR}/tests/*")
add_executable( Tests ${TEST_SRC})

# Link libraries
target_link_libraries(Tests PRIVATE NodableLib gtest gtest_main)

# declare tests
add_test( NAME Tests COMMAND Tests )

# MMMMMMMMMMMMMMMMMMMMMMNOo:;,;;lxKWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMMW0o:;;;;;;;;lkXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# WWWWWWWWWMMMMMMMMMMWOc;;;;;:;;;;;:oKWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# kxolllllxKWMMMMMMMMXo;;::ccccc:::;:xNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# NXOo;'''':xNMMMMMMMKc,,;;;;::;;;;,;oXMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWNXK0XWMMMMMMMMMMMMMMMMMMMMMMMMWNKKNMMMMMMMMMMMMMMMMMWNKKXWMMMMMMMMMMMMMMMMMMMM
# MMMXo''.'.,l0WMMMMMNd,'''''''''''':kNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN0koc:;;oXMMMMMMMMMMMMMMMMMMMMWXOdl:;cOWMMMMMMMMMMMMWKOxlc;;xNMMMMMMMMMMMMMMMMMMMM
# MMMNo',,'...;dXWMMMMXd;'''''''''';dXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWX0o,',,lKWMMMMMMMMMMMMMMMMMMMMNKk:'':kWMMMMMMMMMMMMWXKOl,',oXMMMMMMMMMMMMMMMMMMMM
# MMMNo':xo,...':ONMMMMW0dc;,''',,';dXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMO;'',lKWMMMMMMMMMMMMMMMMMMMMMMXl'';kWMMMMMMMMMMMMMMMWx,',oXMMMMMMMMMMMMMMMMMMMM
# MMMNo.:0NOc'...'oKWMMMMWNKOl';dxc,,cONMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMO;.''lKMMMMMMMMMMMMMMMMMMMMMMMXl.';kWMMMMMMMMMMMMMMMWx,.'oXMMMMMMMMMMMMMMMMMMMM
# MMMNo.:0WMXx,....;xNMMMMMMWd.;OWXd;',oO0kdoooodxOKNMMMMMMMMMMMWNKOkkOKNk,.''lKWMMMMMWX0OOkkOKNWMMMMMMXc..;kNNKOkkO0XWMMMMMMWx'.'oXMMMMMMMWX0Okkk0XWMMM
# MMMNo.:0WMMW0l.....c0WMMMMWd.,OWMW0c'''''.......',cxKWMMMMMMN0o:codxdol:'..'cKWMMWXxc:ldkxd:,:xNMMMMMXc..,oo:,''..';o0WMMMMWx'.'oXMMMMMXxlllolc;',lONM
# MMMNl.:0WMMMMNk;....'dXMMMWd.,OWMW0c'..'''''''''''.';xXMMMMKo',o0NWWWN0c....cKWMMKc..:ONMMWO;.'dNMMMMXc...,cdkOkd:...,dXMMMWd...oXMMMWk:,o0NWWN0c..'oX
# MMMNl.;0WMMMMMWKo.....:ONMWd.,OWWO:''''',,,,,,,,,'''''lKMM0:..lKWMMMMMWx'...cKMMMKc''lKWMMMK:..:KWMMMXc..'oKWMMMWNd'..'dNMMWd...lXMMWk,.:kKXXXXXx'..'d
# MMMNl.;0WMMMMMMMWO:.  ..lKNd.,OWKc'',,;;;;::::;;;;,,,''oNXl..,kWMMMMMMMk'...cKMMMMX0OXWWNKOo'..;0WMMMX:..'kWMMMMMMNo...;0WMWd...lXMMK:..,cllllllc;;;;o
# MMMNl.;0WMMMMMMMMMXd'   .,dl.'OWx,',,;;:ccllllcc::;;,'.:0O,..;OWMMMMMMMk....:KMMMMMWXOdoooxo'..;OWMMMX:..'xWMMMMMMMO'..'kWMWd...lXMMk'..oKXXXXXXXXXXXN
# MMMNc.;0WMMMMMMMMMMW0:.   .. 'ONx......',,;;;;;,,''....,Ok'  ,OWMMMMMMMk. ..:KMMMW0l,'ckKNW0;  ,OWMMMK:  'xWMMMMMMM0,  ,OWMWd. .lXMMk. .cXMMMMMMMMMMMM
# MMMNc ,0WMMMMMMMMMMMMNx'     'kWO,                    .:0O,  .oNMMMMMMMx. ..:KWMNd. .lXWMMMK;  ,OWMMMK;  .xWMMMMMMMk' .cKWMWo. .lXMM0,  'xWMMMMMMMMMXx
# MMMX; 'OWMMMMMMMMMMMMMWKl.   'kWNo.                  .'xNNo.  .dNMMMMMWd.  .:KWM0,  .dNMMMWO'  ,OWMMMK;  .xWMMMMMMXc..:0WMMWo. .cXMMNo.  .oKWMMMMWXx:c
# MWNd. .lXWMMMMMMMMMMMMMMNk,  .kWMNo.                 .cKWMNo.  .;dOK0ko,    .cxKK:   'okkdo:.  .lkOXMX:  .lXWMMMNO:.'oKWMMMNc   :KMMMXo.   .:lool:''l0
# dl;.....;ld0WMMMMMMMMMMMMMXo.'kWMXl.              ... .:kNMW0l,.....';lc..',;lxKN0c.. ..':dOo.  .,oKWW0l,..,:lllc;:d0NMMWKxl'...'lxKWMWOl'.    ..,oONW
# 0OOOOOOOOO0XWMMMMMMMMMMMMMMW0OXWXc. .coc,......',cxOx:. .;xXWWN0OxxkOXNX00XNNWWMMMWKOxkOKNWMWKkxOKNWMMMWNK0kxxxk0KNWMMMMWNK0000000KXWMMMWX0kxxkOKXWMMM
# MMMMMMMMMMMMMMMMMMMMMMMMWNKOkkOk;  .dXWWNX0OOO0KNWWMMNOc.  'oKWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMMMMNkc'.       .dNMMMMMMMMMMMMMMMMMMW0o'  .cONMMMWNKOOOO0KNWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMMMKc.          .oXMMMMMMMMMMMMMMMMMMMMWXd,  .;x0x:'.    ..,ckNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMMXc .....''.... .dNMMMMMMMMMMMMMMMMMMMMMMNk:.  .            .;OWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMM0'    ......    ;KMMMMMMMMMMMMMMMMMMMMMMMMNk'                'kWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMMK,              cKMMMMMMMMMMMMMMMMMMMMMMMMM0,   ...........   :KMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMMWk.            ,kNMMMMMMMMMMMMMMMMMMMMMMMMMO.       ..        ,OWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMMMWO:.        .cONMMMMMMMMMMMMMMMMMMMMMMMMMMX;                 :KWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMMMMMW0xl:;;:lx0NWMMMMMMMMMMMMMMMMMMMMMMMMMMMMO,               ;OWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMMMMMMMMWWWNWWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMKl.           .l0WMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
# MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMW0c.      .'ckXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
