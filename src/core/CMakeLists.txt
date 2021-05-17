
# Do not use this CMaleLists.txt directly, take the one from base directory.
#---------------------------------------------------------------------------

# Declare source files:
file( GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp )

# Declare header files:
file( GLOB HEADERS CONFIGURE_DEPENDS src/*.h )

add_library( nodable_core STATIC ${SOURCES} ${HEADERS} )

target_link_libraries(
        nodable_core
        PUBLIC
            mirror
            rapidjson
            Observe
)

target_include_directories(
        nodable_core
        PUBLIC
            src/
)