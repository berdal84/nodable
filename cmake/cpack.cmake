include_guard(GLOBAL)
nodable_log_title_header()

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

nodable_message("Logging variables ...")
nodable_message("_GENERATOR:                     ${_GENERATOR}")
nodable_message("_SOURCE_GENERATOR:              ${_SOURCE_GENERATOR}")
nodable_message("CPACK_GENERATOR:                ${CPACK_GENERATOR}")
nodable_message("CPACK_BUNDLE_NAME:              ${CPACK_BUNDLE_NAME}")
nodable_message("CPACK_SOURCE_GENERATOR:         ${CPACK_SOURCE_GENERATOR}")
nodable_message("CPACK_PACKAGE_VENDOR:           ${CPACK_PACKAGE_VENDOR}")
nodable_message("CPACK_PACKAGE_CONTACT:          ${CPACK_PACKAGE_CONTACT}")
nodable_message("CPACK_RESOURCE_FILE_LICENSE:    ${CPACK_RESOURCE_FILE_LICENSE}")
nodable_message("CPACK_RESOURCE_FILE_README:     ${CPACK_RESOURCE_FILE_README}")
nodable_message("CPACK_PACKAGE_DIRECTORY:        ${CPACK_PACKAGE_DIRECTORY}")
nodable_message("CPACK_PACKAGE_FILE_NAME:        ${CPACK_PACKAGE_FILE_NAME}")
nodable_message("CPACK_SOURCE_PACKAGE_FILE_NAME: ${CPACK_SOURCE_PACKAGE_FILE_NAME}")


include(CPack)
