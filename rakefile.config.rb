
HOST_OS            = RbConfig::CONFIG['host_os']
TARGET_OS          = RbConfig::CONFIG['target_os']
BUILD_TYPE         = ENV["BUILD_TYPE"]      || "release"
BUILD_DIR          = ENV["BUILD_DIR"]       || "./rake-build-#{BUILD_TYPE}"
BINARY_DIR         = ENV["BINARY_DIR"]      || "./rake-out-#{BUILD_TYPE}"
OBJ_DIR            = ENV["OBJ_DIR"]         || "#{BUILD_DIR}/obj"

TARGET_OS_DARWIN22 = "darwin22" # macos-13 on github actions
TARGET_OS_MINGW32  = "mingw32",  # windows-2022
TARGET_OS_LINUX_GNU    = "linux-gnu" # ubuntu 22.04