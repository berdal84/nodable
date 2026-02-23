#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "LZ4::lz4_static" for configuration "Debug"
set_property(TARGET LZ4::lz4_static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LZ4::lz4_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/liblz4d.a"
  )

list(APPEND _cmake_import_check_targets LZ4::lz4_static )
list(APPEND _cmake_import_check_files_for_LZ4::lz4_static "${_IMPORT_PREFIX}/debug/lib/liblz4d.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
