
HOST_OS            = RbConfig::CONFIG['host_os']
TARGET_OS          = RbConfig::CONFIG['target_os']
BUILD_TYPE         = ENV["BUILD_TYPE"]           || "release"
COMPILER           = ENV["COMPILER"]             || "gcc"
BUILD_DIR_ROOT     = ENV["BUILD_DIR_ROOT"]       || "rake-build-#{BUILD_TYPE}-#{COMPILER}"
BINARY_DIR_ROOT    = ENV["BINARY_DIR_ROOT"]      || "rake-out-#{BUILD_TYPE}-#{COMPILER}"
TARGET_NAME        = "#{TARGET_OS}_#{BUILD_TYPE}"
TARGET_BUILD_DIR   = "#{BUILD_DIR_ROOT}/#{TARGET_NAME}"
TARGET_PACKAGE_DIR = "#{BINARY_DIR_ROOT}/#{TARGET_NAME}"
