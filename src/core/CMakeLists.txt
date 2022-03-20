
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

install( TARGETS   nodable_core ARCHIVE DESTINATION lib)
install( DIRECTORY include/             DESTINATION lib/include FILES_MATCHING PATTERN "*.h")