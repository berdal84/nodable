
# Do not use this CMaleLists.txt directly, take the one from base directory.
#---------------------------------------------------------------------------

# defines
set(NODABLE_APP_ASSETS_ABSOLUTE_DIR "${CMAKE_CURRENT_LIST_DIR}/app/assets")
set(NODABLE_APP_ASSETS_RELATIVE_DIR ./assets)

# Configure Config.h.in
configure_file("conf/Config.h.in" "${PROJECT_BINARY_DIR}/conf/nodable/Config.h" )

# add subdirectories:
add_subdirectory(./app )
add_subdirectory(./core )
