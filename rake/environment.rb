require "rbconfig"

BUILD_OS           = RbConfig::CONFIG['build_os']
HOST_OS            = RbConfig::CONFIG['host_os']
TARGET_OS          = RbConfig::CONFIG['target_os']
BUILD_TYPE         = (ENV["BUILD_TYPE"] || "release").downcase
BUILD_TYPE_RELEASE = BUILD_TYPE == "release"
BUILD_TYPE_DEBUG   = !BUILD_TYPE_RELEASE
BUILD_DIR          = ENV["BUILD_DIR"]       || "./rake-build-#{BUILD_TYPE}"
BINARY_DIR         = ENV["BINARY_DIR"]      || "./rake-out-#{BUILD_TYPE}"
OBJ_DIR            = ENV["OBJ_DIR"]         || "#{BUILD_DIR}/obj"
INSTALL_DIR        = ENV["INSTALL_DIR"]    || "./out"
BUILD_OS_LINUX     = BUILD_OS.include?("linux")
BUILD_OS_MACOS     = BUILD_OS.include?("darwin")
BUILD_OS_WINDOWS   = BUILD_OS.include?("windows") || BUILD_OS.include?("mingw32")

system "echo Ruby version: && ruby -v"
puts "BUILD_OS_LINUX:     #{BUILD_OS_LINUX}"
puts "BUILD_OS_MACOS:     #{BUILD_OS_MACOS}"
puts "BUILD_OS_WINDOWS:   #{BUILD_OS_WINDOWS}"
CLANG_FOUND = system "clang --version"
puts "CLANG_FOUND:        #{CLANG_FOUND}"
puts "BUILD_TYPE_RELEASE: #{BUILD_TYPE_RELEASE}"
puts "BUILD_TYPE_DEBUG:   #{BUILD_TYPE_DEBUG}"

if not CLANG_FOUND
    raise "clang compiler is required, please install an retry."
elsif (not BUILD_OS_LINUX) and (not BUILD_OS_MACOS) and (not BUILD_OS_WINDOWS)
    raise "Unable to determine the operating system"
end