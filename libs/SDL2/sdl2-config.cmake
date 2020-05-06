
# This script was not present in the SDL2 archive
# I read only and made my own to use this specifid folder instead of system one.
#
# This script is used by the parent CMakeList.txt locatedd in the root of Nodable folder.

# Declare include folder
set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

# Get architecture
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(ARCHITECTURE "x64")
else ()
  set(ARCHITECTURE "x86")
endif ()

# Declare the SDL2 folder
set(SDL2_FOLDER "${CMAKE_CURRENT_LIST_DIR}/lib/${ARCHITECTURE}")

# Declare static libraries
set(SDL2_STATIC "${SDL2_FOLDER}/SDL2.lib" "${SDL2_FOLDER}/SDL2main.lib")

# Declare dynamic libraries
set(SDL2_RUNTIME "${SDL2_FOLDER}/SDL2.dll")
