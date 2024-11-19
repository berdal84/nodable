
HOST_OS            = RbConfig::CONFIG['host_os'] || "UnknownOS"
BUILD_TYPE         = ENV["BUILD_TYPE"] || "Release"
TARGET_NAME        = "#{HOST_OS}_#{BUILD_TYPE}"
BUILD_DIR_ROOT     = ENV["BUILD_DIR_ROOT"] || "build"
BINARY_DIR_ROOT    = ENV["BINARY_DIR_ROOT"] || "out" # legacy, ci is going to get the files from there
TARGET_BUILD_DIR   = "#{BUILD_DIR_ROOT}/#{TARGET_NAME}"
TARGET_PACKAGE_DIR = "#{BINARY_DIR_ROOT}/#{TARGET_NAME}"
