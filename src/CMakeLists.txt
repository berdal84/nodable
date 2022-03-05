
# Do not use this CMaleLists.txt directly, take the one from base directory.
#---------------------------------------------------------------------------

# defines
set(NODABLE_APP_ASSETS_DIR assets)
set(NODABLE_APP_ASSETS_ABSOLUTE_DIR "${CMAKE_CURRENT_LIST_DIR}/app/${NODABLE_APP_ASSETS_DIR}")

# Configure BuildInfo.h.in
configure_file("conf/BuildInfo.h.in" "${PROJECT_BINARY_DIR}/conf/nodable/BuildInfo.h" )

# add subdirectories:
add_subdirectory(./app )
add_subdirectory(./core )
