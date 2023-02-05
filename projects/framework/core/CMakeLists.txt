# WARNING: Do not use this CMakeLists.txt directly, take the one from base directory.
include(../../../cmake/log.cmake)
ndbl_log_title_header()

# checks
ndbl_log("Checking ...")
if( NOT FW_INSTALL_DESTINATION)
    ndbl_warn(" - Variable FW_INSTALL_DESTINATION not defined, default value will be used.")
endif()

# options
option(FW_INSTALL_ENABLE      "Enable fw static library install" ON )
option(FW_INSTALL_DESTINATION "Install folder"                      dev/fw )

# log options
ndbl_log("Logging options ...")
ndbl_log(" - FW_INSTALL_ENABLE:      ${FW_INSTALL_ENABLE}")
ndbl_log(" - FW_INSTALL_DESTINATION: ${FW_INSTALL_DESTINATION}")

add_library(
        framework-core
        STATIC
            src/Log.cpp
            src/String.cpp
            src/System.cpp
            src/reflection/func_type.cpp
            src/reflection/qword.cpp
            src/reflection/type.cpp
            src/reflection/type_register.cpp
            src/reflection/variant.cpp)

target_link_libraries(
        framework-core
        PUBLIC # <-------------- Those will be accessible by other targets if they link headless
            ghc_filesystem     # c++ compatible std::filesystem::path like API
            Observe            # event system using observer/observable pattern.
            rapidjson          # read/write json
        PRIVATE
            whereami           # get absolute executable/dll directory path

)

target_include_directories(
        framework-core
        PUBLIC
            include
)

if( ${FW_INSTALL_ENABLE} )
    ndbl_log("Defining install ...")
    install( TARGETS   framework-core ARCHIVE DESTINATION "${FW_INSTALL_DESTINATION}")
    install( DIRECTORY include DESTINATION "${FW_INSTALL_DESTINATION}lib/include" FILES_MATCHING PATTERN "*.h")
else()
    ndbl_log("Skip install")
endif()
