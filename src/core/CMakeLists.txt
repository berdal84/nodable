
# Do not use this CMaleLists.txt directly, take the one from base directory.
#---------------------------------------------------------------------------

# Declare source files:
file( GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp )

add_library( nodable_core STATIC ${SOURCES} )

target_link_libraries(
        nodable_core
        PUBLIC
            rapidjson
            Observe
            mpark_variant
)

target_include_directories(
        nodable_core
        PUBLIC
            include/
)

# Installation :
#===============

set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/$<CONFIG>)
install( TARGETS   nodable_core DESTINATION ${INSTALL_PATH} )
install( DIRECTORY include/         DESTINATION ${INSTALL_PATH}/include FILES_MATCHING PATTERN "*.h")