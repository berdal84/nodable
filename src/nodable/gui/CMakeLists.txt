
# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log_title_header()

# checks
ndbl_log("Checking ...")
if( NOT NDBL_CONFIGURED_DIR )
    ndbl_err("Variable NDBL_CONFIGURED_DIR must be set in main CMakeLists.txt")
endif()

# defines
set(NDBL_APP_ASSETS_DIR ../../../assets)
set(NDBL_APP_ASSETS_ABSOLUTE_DIR "${CMAKE_CURRENT_LIST_DIR}/${NDBL_APP_ASSETS_DIR}")

# log variables
ndbl_log(" - NDBL_CONFIGURED_DIR:          ${NDBL_CONFIGURED_DIR}")
ndbl_log(" - NDBL_APP_ASSETS_DIR:          ${NDBL_APP_ASSETS_DIR}")
ndbl_log(" - NDBL_APP_ASSETS_ABSOLUTE_DIR: ${NDBL_APP_ASSETS_ABSOLUTE_DIR}")

# Configure files
configure_file("build_info.h.in" "${NDBL_CONFIGURED_DIR}/gui/build_info.h" )

add_library(
    nodable-gui
        Config.cpp
        GraphView.cpp
        History.cpp
        HybridFile.cpp
        HybridFileView.cpp
        Nodable.cpp
        NodableView.cpp
        NodeView.cpp
        NodeViewConstraint.cpp
        Physics.cpp
        PropertyView.cpp
        SlotView.cpp)

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
        ${PROJECT_BINARY_DIR}/configured/ndbl/gui/
)

# define assets
set(ASSETS
    "assets/examples/arithmetic.cpp"
    "assets/examples/for-loop.cpp"
    "assets/examples/if-else.cpp"
    "assets/examples/multi-instructions.cpp"
    "assets/fonts/JetBrainsMono-Bold.ttf"
    "assets/fonts/JetBrainsMono-Italic.ttf"
    "assets/fonts/JetBrainsMono-Medium.ttf"
    "assets/fonts/JetBrainsMono-Regular.ttf"
    "assets/images/nodable-logo-xs.png"
    )

# copy each file (will dirty the build when file changes)
foreach(EACH_FILE ${ASSETS})
    configure_file("${EACH_FILE}" "${CMAKE_BINARY_DIR}/${EACH_FILE}" COPYONLY)
endforeach()

ndbl_log_title_header()

# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.

ndbl_log("NDBL_SKIP_TESTS: ${NDBL_SKIP_TESTS}")

if ( NDBL_SKIP_TESTS )
    return()
endif()

# GUI tests (nodable-gui)
#------------------------

add_executable( test-nodable-gui Nodable.specs.cpp )
target_link_libraries( test-nodable-gui PRIVATE gtest_main gtest nodable-gui )

# Add test executables
#---------------------

# GUI tests does not work on every machine (only MacOS in software on GitHub Actions)
if( $ENV{JETBRAINS_IDE} )
    ndbl_log("JETBRAINS_IDE is defined:  Enable Nodable GUI tests (hardware rendering)")
    add_test( COMMAND test-nodable-gui)
    add_definitions(-D NDBL_GUI_TEST_HUMAN_SPEED )
elseif(WIN32)
    ndbl_log("Windows detected: Skip Nodable GUI tests")
elseif(APPLE)
    ndbl_log("Apple detected: Enable Nodable GUI tests (software rendering)")
    add_test( NAME "Nodable GUI (software rendering)" COMMAND nodable-gui-tests)
elseif(UNIX) # Should be tested after APPLE
    ndbl_log("Linux detected: Skip Nodable GUI tests")
endif()
