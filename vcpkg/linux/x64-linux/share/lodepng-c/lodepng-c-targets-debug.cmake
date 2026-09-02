#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "lodepng-c" for configuration "Debug"
set_property(TARGET lodepng-c APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(lodepng-c PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/liblodepng-c.a"
  )

list(APPEND _cmake_import_check_targets lodepng-c )
list(APPEND _cmake_import_check_files_for_lodepng-c "${_IMPORT_PREFIX}/debug/lib/liblodepng-c.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
