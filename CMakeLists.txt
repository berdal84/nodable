# Project  CMakeLists.txt
#========================

# include *.cmake scripts
include(scripts/cmake/log.cmake)
include(scripts/cmake/set-build-ref-variable.cmake)

# prerequisites:
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version") # must be declared BEFORE project
# 10.9 is earliest version supporting c++11

# project declaration
project("Nodable"
        VERSION "0.9"
        LANGUAGES CXX C)

# options/variables
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/out/app")      # ./out/ folder must not be changed, referenced in readme.md
set(NDBL_APP_NAME "Nodable")
set(NDBL_PACKAGE_DIR "${CMAKE_CURRENT_LIST_DIR}/out/package")  #     //
set(NDBL_CONFIGURED_DIR "${PROJECT_BINARY_DIR}/configured/ndbl")
set(NDBL_SKIP_TESTS OFF)
option(FW_NO_POOL "Disable Pool (for benchmark only)" OFF)

# set a build type to Release by default if not set:
if ( NOT DEFINED CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release)
endif()

# Get architecture
if ( NOT ${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    ndbl_err("A 64bits architecture CPU is required to build this project")
endif ()

# avoid "unable to run shared libraries" under linux
include(CheckPIESupported)
check_pie_supported()

# specify the C++ standard
set(CMAKE_CXX_STANDARD            17)
set(CMAKE_CXX_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_EXTENSIONS          ON)

# enable threads (we have some std::async in ndbl_app)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Defines FW_DEBUG/NDBL_DEBUG when in Debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DFW_DEBUG -DNDBL_DEBUG")

if(WIN32)
    add_compile_definitions(NOMINMAX) # avoid min/max macros causing conflicts with min/max functions
endif()

if (FW_NO_POOL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFW_NO_POOL")
endif ()

# add subdirectories
#-------------------

# Libraries: we only add the libraries that have a CMakeList, otherwise we append sources directly.

if (NOT NDBL_SKIP_TESTS)
    enable_testing()
    # https://stackoverflow.com/questions/12540970/how-to-make-gtest-build-mdd-instead-of-mtd-by-default-using-cmake
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # we don't need mocks (for now) nor install
    option(BUILD_GMOCK OFF)
    option(INSTALL_GTEST OFF)
    add_subdirectory(libs/googletest EXCLUDE_FROM_ALL)
endif ()

set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(libs/google/benchmark EXCLUDE_FROM_ALL)
set(GLM_EXT_INCLUDED ON)
add_subdirectory(libs/glm EXCLUDE_FROM_ALL)
add_subdirectory(libs/freetype EXCLUDE_FROM_ALL)
set(NFD_PORTAL ON) # use portal instead of gtk, read: https://github.com/btzy/nativefiledialog-extended#using-xdg-desktop-portal-on-linux
add_subdirectory(libs/nativefiledialog-extended EXCLUDE_FROM_ALL) # target is nfd
add_subdirectory(libs/gulrak/filesystem EXCLUDE_FROM_ALL) # cpp11 compatible filesystem library.
add_subdirectory(libs/Observe EXCLUDE_FROM_ALL) # EXCLUDE_FROM_ALL: we don't want to install it.
add_subdirectory(libs/SDL EXCLUDE_FROM_ALL) # SDL2

# 1) Nodable
#===========

ndbl_log("Looking for libraries ...")

find_package(Threads REQUIRED)
if (NOT Threads_FOUND)
    ndbl_err("Threads not found")
endif ()

find_package(OpenGL REQUIRED)
if (NOT OpenGL_FOUND)
    ndbl_err("OpenGL not found")
endif ()

ndbl_log("Threads found: ${Threads_FOUND}")
ndbl_log("OpenGL found:  ${OpenGL_FOUND}")

# Includes for all (fw and nodable) targets
include_directories(
    src
    libs
    libs/Observe/include
    libs/SDL/include
    libs/gl3w
    libs/gl3w/GL
    libs/gulrak/filesystem/include
    libs/imgui
    libs/whereami/src
    libs/glm
    libs/IconFontCppHeaders
)

# 1.1) Framework Core
#--------------------

add_library(
    framework-core
    STATIC
    libs/whereami/src/whereami.c
    src/fw/core/Color.h
    src/fw/core/TIdentifier.h
    src/fw/core/assertions.h
    src/fw/core/async.cpp
    src/fw/core/async.h
    src/fw/core/format.cpp
    src/fw/core/format.h
    src/fw/core/geometry/Box2D.h
    src/fw/core/geometry/Rect.h
    src/fw/core/geometry/Space.h
    src/fw/core/geometry/Vec2.h
    src/fw/core/geometry/Vec4.h
    src/fw/core/geometry/XForm2D.h
    src/fw/core/hash.h
    src/fw/core/log.cpp
    src/fw/core/log.h
    src/fw/core/math.h
    src/fw/core/memory/Pool.cpp
    src/fw/core/memory/Pool.h
    src/fw/core/reflection/Operator.h
    src/fw/core/reflection/Operator_t.h
    src/fw/core/reflection/class.h
    src/fw/core/reflection/enum.h
    src/fw/core/reflection/func_type.cpp
    src/fw/core/reflection/func_type.h
    src/fw/core/reflection/invokable.h
    src/fw/core/reflection/qword.cpp
    src/fw/core/reflection/qword.h
    src/fw/core/reflection/reflection
    src/fw/core/reflection/registration.h
    src/fw/core/reflection/type.cpp
    src/fw/core/reflection/type.h
    src/fw/core/reflection/type_register.cpp
    src/fw/core/reflection/type_register.h
    src/fw/core/reflection/union.h
    src/fw/core/reflection/variant.cpp
    src/fw/core/reflection/variant.h
    src/fw/core/string.h
    src/fw/core/system.cpp
    src/fw/core/system.h
    src/fw/core/types.h
    # libraries
    libs/whereami/src/whereami.c
    libs/whereami/src/whereami.h
)

target_link_libraries(
    framework-core
    PUBLIC
        ghc_filesystem # c++ compatible std::filesystem::path like API
        Observe        # event system using observer/observable pattern.
        glm            # math
        Threads::Threads
)

target_include_directories(
    framework-core
    PUBLIC
        src/
        src/fw/
        libs/
        libs/Observe/include/
        libs/whereami/src/
)

add_executable(
    test-framework-core
    src/fw/core/reflection/reflection.specs.cpp
    src/fw/core/memory/Pool.specs.cpp
)

target_link_libraries(test-framework-core PRIVATE gtest_main gtest framework-core)
add_test(NAME test_framework_core COMMAND test-framework-core)

add_executable(bench-fw-core-string src/fw/core/string.bench.cpp)
target_link_libraries(bench-fw-core-string PRIVATE benchmark::benchmark framework-core)

add_executable(bench-fw-core-Pool src/fw/core/memory/Pool.bench.cpp)
target_link_libraries(bench-fw-core-Pool PRIVATE benchmark::benchmark framework-core)

# 1.2) Framework GUI
#-------------------

add_library(
    framework-gui
    STATIC
    src/fw/gui/Action.cpp
    src/fw/gui/Action.h
    src/fw/gui/ActionManager.cpp
    src/fw/gui/ActionManager.h
    src/fw/gui/ActionManagerView.cpp
    src/fw/gui/ActionManagerView.h
    src/fw/gui/App.cpp
    src/fw/gui/App.h
    src/fw/gui/AppView.cpp
    src/fw/gui/AppView.h
    src/fw/gui/Config.h
    src/fw/gui/Event.h
    src/fw/gui/EventManager.cpp
    src/fw/gui/EventManager.h
    src/fw/gui/FontManager.cpp
    src/fw/gui/FontManager.h
    src/fw/gui/FontManagerConfig.h
    src/fw/gui/ImGuiEx.cpp
    src/fw/gui/ImGuiEx.h
    src/fw/gui/ImGuiExConfig.h
    src/fw/gui/Texture.h
    src/fw/gui/TextureManager.cpp
    src/fw/gui/TextureManager.h
    src/fw/gui/View.cpp
    src/fw/gui/View.h
    libs/imgui/imgui.cpp
    libs/imgui/imgui_demo.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/misc/freetype/imgui_freetype.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp # include backend as-is
    libs/imgui/backends/imgui_impl_sdl.cpp     #  //
    libs/ImGuiColorTextEdit/TextEditor.cpp
    libs/gl3w/GL/gl3w.c # Open GL Wrangler
    libs/gl3w/GL/gl3w.h
    libs/gl3w/GL/gl3w.gcda
    libs/gl3w/GL/gl3w.gcno
    libs/lodepng/lodepng.cpp
)

target_link_libraries(
    framework-gui
    PUBLIC
        ${CMAKE_DL_LIBS}
        OpenGL::GL
        SDL2-static SDL2main
        nfd # native file dialog - extended
        freetype # because: https://github.com/ocornut/imgui/tree/master/misc/freetype
        framework-core
)

target_include_directories(
    framework-gui
    PRIVATE
        src/fw/
    PUBLIC
        libs/gl3w/
        libs/gl3w/GL/
        src/
        libs/imgui/
        libs/SDL/include/
        libs/gulrak/filesystem/include/
)

# link APPLE specific libraries
IF (APPLE)
    ndbl_log("Looking for Apple specific libraries ...")

    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    if (NOT CORE_FOUNDATION_FRAMEWORK)
ndbl_err("CoreFoundation not found")
    endif ()

    find_library(COCOA_FRAMEWORK Cocoa)
    if (NOT COCOA_FRAMEWORK)
ndbl_err("Cocoa not found")
    endif ()

    ndbl_log("CORE_FOUNDATION_FRAMEWORK: ${CORE_FOUNDATION_FRAMEWORK}")
    ndbl_log("COCOA_FRAMEWORK:   ${COCOA_FRAMEWORK}")

    target_link_libraries(
    framework-gui
    PRIVATE
    ${CORE_FOUNDATION_FRAMEWORK}
    ${COCOA_FRAMEWORK})
ENDIF ()

target_compile_definitions(framework-gui PUBLIC IMGUI_USER_CONFIG="${CMAKE_CURRENT_LIST_DIR}/src/fw/gui/ImGuiExConfig.h") # Override imconfig.h
set_target_properties(framework-gui PROPERTIES POSITION_INDEPENDENT_CODE FALSE)       # required to run well on recent os (ex: ubuntu)
set_target_properties(framework-gui PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio

# define assets
set(FW_ASSETS
"assets/fonts/CenturyGothic.ttf"
"assets/fonts/fa-solid-900.ttf"
)

# copy each file (will dirty the build when file changes)
foreach (ASSET ${FW_ASSETS})
    configure_file("${ASSET}" "${CMAKE_BINARY_DIR}/${ASSET}" COPYONLY)
endforeach ()

# 1.3) Framework GUI - Example application
#---------------------------------------

# add executable
add_executable(framework-gui-example WIN32 src/fw/gui-example/main.cpp)
target_link_libraries(framework-gui-example PRIVATE framework-gui)
set_target_properties(framework-gui-example PROPERTIES POSITION_INDEPENDENT_CODE FALSE)       # required to run well on recent os (ex: ubuntu)
set_target_properties(framework-gui-example PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio

# Installation :
# install( TARGETS framework-gui-example RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE DESTINATION .)

# 2.1) Nodable Core
#------------------

add_library(
    nodable-core
    STATIC
    src/nodable/core/Component.cpp
    src/nodable/core/ComponentBag.cpp
    src/nodable/core/IfNode.cpp
    src/nodable/core/DataAccess.cpp
    src/nodable/core/DirectedEdge.cpp
    src/nodable/core/ForLoopNode.cpp
    src/nodable/core/Graph.cpp
    src/nodable/core/GraphUtil.cpp
    src/nodable/core/InvokableComponent.cpp
    src/nodable/core/LiteralNode.cpp
    src/nodable/core/Node.cpp
    src/nodable/core/NodeFactory.cpp
    src/nodable/core/Property.cpp
    src/nodable/core/PropertyBag.cpp
    src/nodable/core/Scope.cpp
    src/nodable/core/Slot.cpp
    src/nodable/core/SlotRef.cpp
    src/nodable/core/Token.cpp
    src/nodable/core/TokenRibbon.cpp
    src/nodable/core/VariableNode.cpp
    src/nodable/core/VirtualMachine.cpp
    src/nodable/core/WhileLoopNode.cpp
    src/nodable/core/assembly/Code.cpp
    src/nodable/core/assembly/Compiler.cpp
    src/nodable/core/assembly/Instruction.cpp
    src/nodable/core/language/Nodlang.cpp
    src/nodable/core/language/Nodlang_biology.cpp
    src/nodable/core/language/Nodlang_math.cpp
)

target_link_libraries(
    nodable-core
    PUBLIC
    framework-core
)

target_include_directories(
    nodable-core
    PUBLIC
    src/nodable/
)

ndbl_log("Defining install ...")
set_target_properties(nodable-core PROPERTIES OUTPUT_NAME "core")

ndbl_log("NDBL_SKIP_TESTS: ${NDBL_SKIP_TESTS}")

if (NDBL_SKIP_TESTS)
    return()
endif ()

add_executable(
    test-nodable-core
    src/nodable/core/Graph.specs.cpp
    src/nodable/core/Node.specs.cpp
    src/nodable/core/Property.specs.cpp
    src/nodable/core/Slot.specs.cpp
    src/nodable/core/Token.specs.cpp
    src/nodable/core/language/Nodlang.specs.cpp
    src/nodable/core/language/Nodlang.tokenize.specs.cpp
    src/nodable/core/language/Nodlang.parse_function_call.specs.cpp
    src/nodable/core/language/Nodlang.parse_token.specs.cpp
    src/nodable/core/VirtualMachine.specs.cpp
    src/nodable/core/language/Nodlang.parse_and_eval.specs.cpp
)
target_link_libraries(test-nodable-core PRIVATE gtest_main gtest nodable-core)
add_test(NAME test_nodable_core COMMAND test-nodable-core)

# Benchmarks
add_executable(bench-nodable-core-Nodlang src/nodable/core/language/Nodlang.bench.cpp)
target_link_libraries(bench-nodable-core-Nodlang PRIVATE benchmark::benchmark nodable-core)

# 2.1) Nodable CLI
#-----------------

ndbl_log("Checking ...")
if (NOT NDBL_CONFIGURED_DIR)
    ndbl_err("Variable NDBL_CONFIGURED_DIR must be set in main CMakeLists.txt")
endif ()

add_executable(
    nodable-cli
    src/nodable/cli/main.cpp
    src/nodable/cli/CLI.cpp
)

target_link_libraries(nodable-cli PRIVATE nodable-core)
set_target_properties(nodable-cli PROPERTIES POSITION_INDEPENDENT_CODE FALSE)       # required to run well on recent os (ex: ubuntu)
set_target_properties(nodable-cli PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio
set_target_properties(nodable-cli PROPERTIES OUTPUT_NAME "cli")

# Installation :
install(TARGETS nodable-cli DESTINATION . RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE)

# 2.1) Nodable GUI
#------------------

# checks
ndbl_log("Checking ...")
if (NOT NDBL_CONFIGURED_DIR)
    ndbl_err("Variable NDBL_CONFIGURED_DIR must be set in main CMakeLists.txt")
endif ()

# defines
set(NDBL_APP_ASSETS_DIR assets)
set(NDBL_APP_ASSETS_ABSOLUTE_DIR "${CMAKE_CURRENT_LIST_DIR}/${NDBL_APP_ASSETS_DIR}")

# log variables
ndbl_log(" - NDBL_CONFIGURED_DIR:  ${NDBL_CONFIGURED_DIR}")
ndbl_log(" - NDBL_APP_ASSETS_DIR:  ${NDBL_APP_ASSETS_DIR}")
ndbl_log(" - NDBL_APP_ASSETS_ABSOLUTE_DIR: ${NDBL_APP_ASSETS_ABSOLUTE_DIR}")

# Configure files
configure_file(
    src/nodable/gui/build_info.h.in
    ${NDBL_CONFIGURED_DIR}/gui/build_info.h
)

add_library(
    nodable-gui
    STATIC
    src/nodable/gui/Config.cpp
    src/nodable/gui/GraphView.cpp
    src/nodable/gui/History.cpp
    src/nodable/gui/HybridFile.cpp
    src/nodable/gui/HybridFileView.cpp
    src/nodable/gui/Nodable.cpp
    src/nodable/gui/NodableView.cpp
    src/nodable/gui/NodeView.cpp
    src/nodable/gui/NodeViewConstraint.cpp
    src/nodable/gui/Physics.cpp
    src/nodable/gui/PropertyView.cpp
    src/nodable/gui/SlotView.cpp
)

target_link_libraries(
    nodable-gui
    PUBLIC
        nodable-core
        framework-gui
)

target_include_directories(
    nodable-gui
    PRIVATE
        src/nodable/
        ${PROJECT_BINARY_DIR}/configured/ndbl/gui/
)

# define assets
set(ASSETS
    assets/examples/arithmetic.cpp
    assets/examples/for-loop.cpp
    assets/examples/if-else.cpp
    assets/examples/multi-instructions.cpp
    assets/fonts/JetBrainsMono-Bold.ttf
    assets/fonts/JetBrainsMono-Italic.ttf
    assets/fonts/JetBrainsMono-Medium.ttf
    assets/fonts/JetBrainsMono-Regular.ttf
    assets/images/nodable-logo-xs.png
)

# copy each file (will dirty the build when file changes)
foreach (ASSET ${ASSETS})
    configure_file("${ASSET}" "${CMAKE_BINARY_DIR}/${ASSET}" COPYONLY)
endforeach ()

ndbl_log("NDBL_SKIP_TESTS: ${NDBL_SKIP_TESTS}")

if (NDBL_SKIP_TESTS)
    return()
endif ()

add_executable(test-nodable-gui src/nodable/gui/Nodable.specs.cpp)
target_link_libraries(test-nodable-gui PRIVATE gtest_main gtest nodable-gui)

# GUI tests does not work on every machine (only MacOS in software on GitHub Actions)
if ($ENV{JETBRAINS_IDE})
    ndbl_log("JETBRAINS_IDE is defined:  Enable Nodable GUI tests (hardware rendering)")
    add_test(NAME test_nodable_gui COMMAND test-nodable-gui)
    add_definitions(-D NDBL_GUI_TEST_HUMAN_SPEED)
elseif (WIN32)
    ndbl_log("Windows detected: Skip Nodable GUI tests")
elseif (APPLE)
    ndbl_log("Apple detected: Enable Nodable GUI tests (software rendering)")
    add_test(NAME test_nodable_gui COMMAND test-nodable-gui)
elseif (UNIX) # Should be tested after APPLE
    ndbl_log("Linux detected: Skip Nodable GUI tests")
endif ()

# 2.1) Nodable App
#-----------------

add_executable(nodable-app WIN32 src/nodable/app/main.cpp)
target_link_libraries(nodable-app PUBLIC nodable-gui)
set_target_properties(nodable-app PROPERTIES POSITION_INDEPENDENT_CODE FALSE)       # required to run well on recent os (ex: ubuntu)
set_target_properties(nodable-app PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${BUILD_PATH}") # fix working directory for visual studio
set_target_properties(nodable-app PROPERTIES OUTPUT_NAME "nodable")

set(README README.md)
set(LICENSE LICENSE)
configure_file("${README}" . COPYONLY)
configure_file("${LICENSE}" . COPYONLY)

# Installation :
install(TARGETS nodable-app DESTINATION . RUNTIME PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE)
install(DIRECTORY assets DESTINATION .)
install(FILES ${LICENSE} ${README} DESTINATION .)

# 3) Packaging:
#==============

include(scripts/cmake/cpack.cmake)
