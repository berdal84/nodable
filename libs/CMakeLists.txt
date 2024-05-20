#
# Create a target per library to facilitate usage by nodable targets
#-------------------------------------------------------------------

# GLM
#-------------
set(GLM_EXT_INCLUDED on)
add_subdirectory(glm EXCLUDE_FROM_ALL)

# google tests
#-------------
add_subdirectory(googletest EXCLUDE_FROM_ALL)

# freetype
#------------------------------
add_library(xxhash INTERFACE)
target_include_directories(xxhash INTERFACE ./)

# google benchmark
#-----------------
# Did not find Google Test sources! Either pass correct path in
# GOOGLETEST_PATH, or enable BENCHMARK_DOWNLOAD_DEPENDENCIES, or disable
# BENCHMARK_USE_BUNDLED_GTEST, or disable BENCHMARK_ENABLE_GTEST_TESTS /
# BENCHMARK_ENABLE_TESTING.
set(BENCHMARK_ENABLE_GTEST_TESTS off)
set(BENCHMARK_ENABLE_TESTING off)
add_subdirectory(google/benchmark EXCLUDE_FROM_ALL)

# freetype
#------------------------------
add_subdirectory(freetype EXCLUDE_FROM_ALL)

# native file dialog - extended
#------------------------------
set(NFD_PORTAL ON)                          # use portal instead of gtk, read: https://github.com/btzy/nativefiledialog-extended#using-xdg-desktop-portal-on-linux
add_subdirectory(nativefiledialog-extended EXCLUDE_FROM_ALL) # target is nfd

# filesystem (c++11 compatible)
#------------------------------
add_subdirectory(gulrak/filesystem EXCLUDE_FROM_ALL)

# Observe
add_subdirectory( Observe EXCLUDE_FROM_ALL) # EXCLUDE_FROM_ALL: we don't want to install it.

# SDL2
#------
add_subdirectory( SDL EXCLUDE_FROM_ALL)

