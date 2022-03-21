
#Generators
#7Z                           = 7-Zip file format
#Bundle                       = Mac OSX bundle
#DragNDrop                    = Mac OSX Drag And Drop
#External                     = CPack External packages
#IFW                          = Qt Installer Framework
#NSIS                         = Null Soft Installer
#NSIS64                       = Null Soft Installer (64-bit)
#NuGet                        = NuGet packages
#OSXX11                       = Mac OSX X11 bundle
#PackageMaker                 = Mac OSX Package Maker installer
#STGZ                         = Self extracting Tar GZip compression
#TBZ2                         = Tar BZip2 compression
#TGZ                          = Tar GZip compression
#TXZ                          = Tar XZ compression
#TZ                           = Tar Compress compression
#ZIP                          = ZIP file format
#productbuild                 = Mac OSX pkg

if(WIN32)
    set(_GENERATOR        ZIP)
    set(_SOURCE_GENERATOR ZIP)
elseif(APPLE)
    set(_GENERATOR        ZIP ) # or DragNDrop
    set(_SOURCE_GENERATOR ZIP)
elseif(UNIX)
    set(_GENERATOR        ZIP)
    set(_SOURCE_GENERATOR ZIP)
endif()

message("cpack.cmake: _GENERATOR:        ${_GENERATOR}")
message("cpack.cmake: _SOURCE_GENERATOR: ${_SOURCE_GENERATOR}")

string(TOLOWER ${CMAKE_SYSTEM_NAME} _sys)
string(TOLOWER ${PROJECT_NAME}      _project_lower)

set(CPACK_GENERATOR                   ${_GENERATOR})
set(CPACK_BUNDLE_NAME                 "${PROJECT_NAME}-${GIT_TAG}${GIT_SHORT_HASH}")
set(CPACK_SOURCE_GENERATOR            ${_SOURCE_GENERATOR})
set(CPACK_PACKAGE_VENDOR              "Bérenger DALLE-CORT")
set(CPACK_PACKAGE_CONTACT             "Bérenger DALLE-CORT")
set(CPACK_RESOURCE_FILE_LICENSE       "${PROJECT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README        "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DIRECTORY           "${PROJECT_SOURCE_DIR}/package")
set(CPACK_PACKAGE_FILE_NAME           "${_project_lower}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME    "${_project_lower}-sources")

include(CPack)
