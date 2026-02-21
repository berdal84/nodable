#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::gl3w::gl3w" for configuration "Release"
set_property(TARGET unofficial::gl3w::gl3w APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(unofficial::gl3w::gl3w PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/gl3w.lib"
  )

list(APPEND _cmake_import_check_targets unofficial::gl3w::gl3w )
list(APPEND _cmake_import_check_files_for_unofficial::gl3w::gl3w "${_IMPORT_PREFIX}/lib/gl3w.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
