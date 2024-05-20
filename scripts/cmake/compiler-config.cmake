include_guard(GLOBAL)
ndbl_log_title_header()

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
set(CMAKE_CXX_STANDARD            11)
set(CMAKE_CXX_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_EXTENSIONS          ON)

# enable threads (we have some std::async in ndbl_app)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# define DEBUG in DEBUG (there is no cross-compiler solution)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DFW_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DNDBL_DEBUG")

if(WIN32)
    add_compile_definitions(NOMINMAX) # avoid min/max macros causing conflicts with min/max functions
endif()

if (FW_NO_POOL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFW_NO_POOL")
endif ()
