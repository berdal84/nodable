
# set a build type to Release by default if not set:
if ( NOT DEFINED CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release)
endif()

# Get architecture
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    set(ARCHITECTURE "x64")
else ()
    set(ARCHITECTURE "x86")
endif ()

# avoid "unable to run shared libraries" under linux
include(CheckPIESupported)
check_pie_supported()

# specify the C++ standard
set(CMAKE_CXX_STANDARD          11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        ON)

# enable threads (we have some std::async in nodable_app)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# define _DEBUG (MSVC define it under windows but this is not the default behavior
# with other compilers)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(NOMINMAX) # avoid macros min/max causing std::min / std::max conflicts
endif()