
HOST_OS            = RbConfig::CONFIG['host_os']
TARGET_OS          = RbConfig::CONFIG['target_os']
BUILD_TYPE         = ENV["BUILD_TYPE"]      || "release"
BUILD_DIR          = ENV["BUILD_DIR"]       || "./rake-build-#{BUILD_TYPE}"
BINARY_DIR         = ENV["BINARY_DIR"]      || "./rake-out-#{BUILD_TYPE}"
OBJ_DIR            = ENV["OBJ_DIR"]         || "#{BUILD_DIR}/obj"
