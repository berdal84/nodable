require "rbconfig"
require 'json'
require 'date' # To add date in .clang export
require 'optparse'
require 'rubygems'

# Enums-like ------------------------------------------------------------------------------------------------

TARGET_TYPE_OBJECTS    = "objects"
TARGET_TYPE_EXECUTABLE = "executable"

TARGET_WEB             = "web"
TARGET_DESKTOP         = "desktop"
TARGET_DEFAULT         = TARGET_DESKTOP
TARGETS                = [TARGET_DESKTOP, TARGET_WEB]

BUILD_CONFIG_DEBUG       = "debug"
BUILD_CONFIG_OPTIMIZED   = "optimized"
BUILD_CONFIG_RELEASE     = "release"
BUILD_CONFIG_DEFAULT     = BUILD_CONFIG_DEBUG
BUILD_CONFIGS            = [BUILD_CONFIG_DEBUG, BUILD_CONFIG_OPTIMIZED, BUILD_CONFIG_RELEASE]

# must match with vcpkg triplet naming convention
VCPKG_OS_NAME_WINDOWS  = "windows"
VCPKG_OS_NAME_LINUX    = "linux"

BINARY_CLANG           = "clang"
BINARY_EMCC            = "emcc"
BINARY_EMRUN           = "emrun"
BINARY_CLOC            = 'cloc'

# Enums-like (end) ------------------------------------------------------------------------------------------

# Command Line Arguments ------------------------------------------------------------------------------------

# Declare/define a struct to store parsed options
OPTIONS = Struct.new(
    :verbose,
    :build_type,
    :build_dir,
    :target,
    :ignore_gui_tests,
    keyword_init: true
).new(
    verbose:            false,
    build_dir:          nil, 
    build_type:         BUILD_CONFIG_DEFAULT,
    target:             TARGET_DEFAULT,
    ignore_gui_tests: false,
)

# Define a parser
$option_parser = OptionParser.new

$option_parser.banner = "Usage: rake <task> -- [flags]"

$option_parser.on('-t', '--target=TARGET', TARGETS, "#{TARGETS.join("|")} (default: #{TARGET_DEFAULT})",  ) do |value|
    OPTIONS.target = value
end

$option_parser.on('-c', '--build-config=BUILD_CONFIG', BUILD_CONFIGS, "#{BUILD_CONFIGS.join("|")} (default: #{BUILD_CONFIG_DEFAULT})") do |value|
    OPTIONS.build_type = value
end

$option_parser.on('-d', '--build-dir=BUILD_DIR', "Build directory, absolute or relative to the rakefile (default: 'build-{target}-{arch}-{os}-{build_type}')") do |value|
    OPTIONS.build_dir = value
end

$option_parser.on("-v", "--verbose", "Print diagnostic messages") {
    OPTIONS.verbose = true
}

$option_parser.on("--ignore-gui-tests", "Disable any test that requires to open a window") {
    OPTIONS.ignore_gui_tests = true
}

# Extract flags (after `--`)
flags = ARGV.drop(1)
flags_index = ARGV.index('--')
if flags_index != nil
    flags = ARGV[(flags_index + 1)..-1]
end

# Parse flags and handle errors
begin
    $option_parser.parse!(flags)
rescue OptionParser::InvalidOption, OptionParser::MissingArgument, OptionParser::InvalidArgument => e
    $stdout.puts e
    $stdout.puts $option_parser.help
    $stderr.puts "Unable to parse flags, see reason message and help above."
    exit 1
end

# Command Line Arguments (end) ----------------------------------------------------------------------------------

# Global Constants ----------------------------------------------------------------------------------------------

# Architecture (we use VCPKG naming convention)
ARCH = ->() {

    host_cpu = RbConfig::CONFIG['host_cpu']

    if host_cpu != "x86_64" and host_cpu != "x64"
        raise "This script is not compatible with #{host_cpu} architecture!"
    end

    return "x64"
}.call()

# Operating System (we use VCPKG naming convention)
OS = ->() { 
    
    build_os = RbConfig::CONFIG['build_os']

    if build_os.include?("linux")
        return "linux"
    elsif build_os.include?("mingw32") # Ruby is built on mingw32 (w32 does not stands for 32bits)
        return "windows"
    end

    raise "This script is not compatible with #{build_os}!"
}.call()   

# Helpers to simplify branching (if LINUX ... elif WINDOWS ... else ... end )
LINUX   = OS == VCPKG_OS_NAME_LINUX
WINDOWS = OS == VCPKG_OS_NAME_WINDOWS

# Triplet (we use VCPKG naming convention)
VCPKG_TRIPLET = ->() {

    triplet = "#{ARCH}-#{OS}"

    if OS == "windows"
        triplet += "-static" # windows convention is different than linux, dynamic by default
    end

    triplet
    
}.call()

# Path to installed folder (we decided to separate linux and windows folders)
VCPKG_INSTALL_ROOT  = "./vcpkg/#{OS}"
VCPKG_PACKAGES_ROOT = "#{VCPKG_INSTALL_ROOT}/#{VCPKG_TRIPLET}"

PKGCONF_BINARY = ->() {

    path = "#{VCPKG_PACKAGES_ROOT}/tools/pkgconf/pkgconf"

    if WINDOWS 
        path = path.ext("exe")
    end

    if not File.exist?(path)
        $stderr.puts "Error: PKGCONF_BINARY '#{path}' does not exist! In principle this file is in the source code, but perhaps you delete it and forgot to run 'rake vcpkg'? "
        exit 1
    end

    path 

}.call()

PKGCONF              = "#{PKGCONF_BINARY} --with-path #{VCPKG_PACKAGES_ROOT}/lib/pkgconfig"
HOST_OS              = RbConfig::CONFIG['host_os']
DESKTOP              = OPTIONS.target == TARGET_DESKTOP
WEB                  = OPTIONS.target == TARGET_WEB
RELEASE              = OPTIONS.build_type == BUILD_CONFIG_RELEASE
DEBUG                = OPTIONS.build_type == BUILD_CONFIG_DEBUG
OPTIMIZED            = OPTIONS.build_type == BUILD_CONFIG_OPTIMIZED
BUILD_DIR            = File.expand_path( OPTIONS.build_dir || "build-#{OPTIONS.target}-#{ARCH}-#{OS}-#{OPTIONS.build_type}", Dir.pwd )
DIST_DIR             = "#{BUILD_DIR}/dist" # Distribution files will be copied there (after a build)
OBJ_DIR              = "#{BUILD_DIR}/obj"
DEP_DIR              = "#{BUILD_DIR}/dep"
BIN_DIR              = "#{BUILD_DIR}/bin" # binaries will be generated there

GITHUB_ACTIONS       = ENV["GITHUB_ACTIONS"]
HTTP_SERVER_HOSTNAME = "0.0.0.0"  # TODO: add to flags
HTTP_SERVER_PORT     = "8000" # TODO: add to flags
HTTP_SERVER_URL      = "http://#{HTTP_SERVER_HOSTNAME}:#{HTTP_SERVER_PORT}/"
COMPILER             = DESKTOP ? BINARY_CLANG : BINARY_EMCC # Same binary to compile both C and CPP
LINKER               = DESKTOP ? BINARY_CLANG : BINARY_EMCC # Same binary to compile both C and CPP

if OPTIONS.verbose
puts "------------------------------------------------------------------------------------------------------"
puts "OPTIONS: ............ #{OPTIONS}"
puts "RUBY version: ....... #{`ruby -v`}"
puts "HOST_OS: ............ #{HOST_OS}"
puts "OS:.................. #{OS}"
puts "ARCH: ............... #{ARCH}"
puts "VCPKG_PACKAGES_ROOT:  #{VCPKG_PACKAGES_ROOT}"
puts "VCPKG_TRIPLET: ...... #{VCPKG_TRIPLET}"
puts "HTTP_SERVER_URL: .... #{HTTP_SERVER_URL}"
puts "PKGCONF_BINARY: ..... #{PKGCONF_BINARY}"
puts "PKGCONF: ............ #{PKGCONF}"
puts "COMPILER: ........... #{COMPILER}"
puts "LINKER: ............. #{LINKER}"
puts "Dir.pwd: ............ #{Dir.pwd }"
puts "__FILE__: ........... #{File.dirname(__FILE__)}"
puts "------------------------------------------------------------------------------------------------------"
end # if OPTIONS.verbose

# Global Constants (end) ------------------------------------------------------------------------------------------

# Target utilities ------------------------------------------------------------------------------------------------

Target = Struct.new(
    :name,
    :type, # TARGET_XXX
    :sources, # list of .c|.cpp files
    :depends_on_target, # list of other targets to link with (if their sources are not compiled yet, it will compile them as *.o and will be linked)
    :includes, # list of path dir to include
    :defines,
    :compiler_flags,
    :c_flags,
    :cxx_flags,
    :linker_flags,
    :assets, # List of patterns like: "<source>" or "<source>:<destination>"
    :vcpkg, # list of (static) vcpkg package names
    :is_initialized, # is ready to compile (e.g. pkg-config was run)
    :cached_includes_flags,
    :cached_defines_flags,
    :distribute,
    keyword_init: true # If the optional keyword_init keyword argument is set to true, .new takes keyword arguments instead of normal arguments.
)

def new_empty_target(name, type)
    target = Target.new
    target.name = name
    target.type = type
    target.sources  = FileList[]
    target.depends_on_target = []
    target.includes = FileList[]
    target.c_flags  = []
    target.cxx_flags = []
    target.linker_flags = []
    target.assets = FileList[]
    target.defines = []
    target.compiler_flags = []
    target.vcpkg = []
    target.is_initialized = false;
    target.cached_includes_flags = ""
    target.cached_defines_flags  = ""
    target.distribute = false
    target
end

def src_to_obj( obj )
    "#{OBJ_DIR}/#{ obj.ext(".o")}"
end

def src_to_dep( src )
    "#{DEP_DIR}/#{src.ext(".d")}"
end

def obj_to_src( obj, _target)
    stem = obj.sub("#{OBJ_DIR}/", "").ext("")
    _target.sources.detect{|src| src.ext("") == stem } or raise "unable to find #{obj}'s source (stem: #{stem})"
end

def to_objects( sources )
    sources.map{|src| src_to_obj(src) };
end

def get_self_objects( target )
    to_objects( target.sources )
end

def get_objects__incl_deps( target )
    
    # Take this target's objects
    objects = get_self_objects( target )

    # Append dependencies's objects
    target.depends_on_target.each do |other_target|
        objects |= get_objects__incl_deps( other_target )
    end

    objects
end

def get_binary_path( target )
    path = "#{BIN_DIR}/#{target.name}"
    if WEB
        path = path.ext("html")
    elsif DESKTOP and WINDOWS
        path = path.ext("exe")
    end
    path
end

def generate_flags_for_vcpkg( target )

    puts "#{target.name} | Generate flags for vcpkg .."
    puts "#{target.name} | -- vcpkg list: #{target.vcpkg}"

    # We must add default include path for headers and libraries because some vcpkg do not have a .pc file
    # and their location is 99% of the time in those two folders:
    temp_cxx_flags    = ["-I#{VCPKG_PACKAGES_ROOT}/include"]
    temp_linker_flags = ["-L#{VCPKG_PACKAGES_ROOT}/lib"]
    
    target.vcpkg.each do |vcpkg_name|

        # we use |= to make sure there is no duplicates
        temp_cxx_flags    |= get_library_cflags(vcpkg_name).split(" ")
        temp_linker_flags |= get_library_linker_flags(vcpkg_name, "static").split(" ")
    
    end

    # we use += here because we would like to see compiler warnings if a flag from these temp_xxx_flags already exist in the target.xxx_flags,
    # that would mean some flags can be removed from target.xxx_flags perhaps...
    target.cxx_flags    += temp_cxx_flags 
    target.linker_flags += temp_linker_flags

    puts "#{target.name} | -- cxx_flags added:    #{temp_cxx_flags}"
    puts "#{target.name} | -- linker_flags added: #{temp_linker_flags}"

    puts "#{target.name} | Generate vcpkg flags DONE"
end

$mutex_initializing = Mutex.new

def ensure_is_initialized(target)

    # Let's check if ready first (we don't need to sync threads to read)
    if target.is_initialized
        return
    end

    # Since multiple task may run this in parrallel, we need to lock this portion
    $mutex_initializing.synchronize {

        # There is a possibility where 1 task already started to execute this scope while an other was waiting to execute it too.
        # In such case, once the first finishes the execution (and set the flag to true) the second enter
        # But, the second should abort.
        if target.is_initialized
            return
        end

        puts "#{target.name} | Initialization .."
    
        # First, we have to generate flags for vcpkg
        # then we cache flags

        generate_flags_for_vcpkg(target)

        # Then, we cache some flags as string to share the data accross multiple compilation units

        target.cached_defines_flags  = target.defines.map{|d|  "--define-macro=\"#{d}\"" }.join(" ") # see https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-D-macro
        target.cached_includes_flags = target.includes.map{|f| "--include-directory=#{File.absolute_path(f)}"}.join(" ") # see https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-I-dir

        target.is_initialized = true

        puts "#{target.name} | Initialization DONE"
    }    
end

def compile_file(src, target)

    ensure_is_initialized(target)
    
    is_cpp = File.extname( src ) == ".cpp"

    dep = src_to_dep( src )
    obj = src_to_obj( src )

    # Ensure target folders exist
    FileUtils.mkdir_p File.dirname( obj )
    FileUtils.mkdir_p File.dirname( dep )

    args = [
        target.compiler_flags,
        is_cpp ? target.cxx_flags : target.c_flags,
        "-c", # no linking
        target.cached_includes_flags,
        target.cached_defines_flags,
        # Write dependency database
        # TODO: skip this in release might speed up build?
        "--write-user-dependencies", # Write a depfile containing user headers https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-MMD
        "-MF#{dep}", # Write depfile output from -MMD, -MD, -MM, or -M to <file> https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-MF-file
        "-MJ#{obj.ext("o.json")}", # Write a compilation database entry per input, see https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-MJ-arg
        "--output=#{obj}",
        src
    ].join(" ")

    system("#{COMPILER} #{args}", exception: true)
end

def link_binary( target )

    if (target.type != TARGET_TYPE_EXECUTABLE)
        raise "Target type is expected to be: '#{TARGET_TYPE_EXECUTABLE}', actual: #{target.type}"
    end

    ensure_is_initialized(target)

    args = [
        target.compiler_flags,
        target.cached_defines_flags,
        "--output=#{get_binary_path(target)}",
        get_objects__incl_deps(target),
        target.linker_flags
    ].join(" ")

    FileUtils.mkdir_p File.dirname(get_binary_path(target))

    system("#{LINKER} #{args}", exception: true)
end

def update_compile_commands_json()

    #
    # Update compile_commands.json
    # This file can be read by clangd to perform static analysis (e.g. in VSCode or CLION )
    #

    output_file = "./compile_commands.json"

    # clear existing file
    if File.exist? output_file
        FileUtils.rm_f output_file
    end

    # Grab all *.o.json files from object dir
    command_files = FileList["#{OBJ_DIR}/**/*.json"]

    # combine
    commands = command_files.map{|f| File.read(f)}.join()
    content = "[" + commands[0...-2] + "]" # -2: remove trailing comma
    File.write( output_file, content)
end

def tasks_for_target(target)

    objects = get_self_objects(target)
    objects_with_deps = get_objects__incl_deps(target)

    task :clean do
        FileUtils.rm_f objects
    end

    task :clean_all do
        FileUtils.rm_f objects_with_deps
    end

    task :rebuild => [:clean, :build]

    task :rebuild_all => [:clean_all, :build]

    if target.type == TARGET_TYPE_OBJECTS

        multitask :build => objects_with_deps do
            puts "#{target.name} | Build DONE"
        end

    elsif target.type == TARGET_TYPE_EXECUTABLE

        task :build => [:link] do

            if target.distribute            
                binary_path = get_binary_path(target)
                src = binary_path
                dst = "#{DIST_DIR}/#{File.basename(src)}"

                FileUtils.mkdir_p(File.dirname(dst))
                FileUtils.cp( src, dst )            
                puts "  Copy binary: #{src} => #{dst}"          
            end  

            # Copy assets
            target.assets.each_with_index do |pattern, i|

                # Handle pattern (format is <src>[:<dest>], by default dest=src)
                arr = pattern.split(':') 
                src = arr[0] or raise ("Wrong pattern: #{pattern}, expecting '<src>[:<dest>]'")
                dst = "#{BIN_DIR}/#{arr[1] || src}";

                FileUtils.mkdir_p File.dirname(dst)
                FileUtils.cp( src, dst )
                puts "  Copy asset: #{src} => #{dst}"

                if target.distribute
                    dst = "#{DIST_DIR}/#{arr[1] || src}";
                    FileUtils.mkdir_p(File.dirname(dst))
                    FileUtils.cp( src, dst )
                    puts "  Copy asset: #{src} => #{dst}"
                end
            end

            puts "#{target.name} | Build DONE"
        end
        
        task :link => get_binary_path(target)

        file get_binary_path(target) => :compile_objects do
            puts "#{target.name} | Linking '#{get_binary_path(target)}'..."
            link_binary(target)
            puts "#{target.name} | Linking '#{get_binary_path(target)}' DONE"
        end

        multitask :compile_objects => objects_with_deps do
            update_compile_commands_json()
        end

        task :run => :build do

            if DESKTOP
                system("./#{get_binary_path(target)}", exception: true)
            elsif WEB
                system("#{BINARY_EMRUN} --hostname #{HTTP_SERVER_HOSTNAME} --port #{HTTP_SERVER_PORT} #{get_binary_path(target)}", exception: true)
            end
        end
    end

    # Add a task per object to build (dependencies excluded)
    objects.each_with_index do |obj, index|
        src = obj_to_src( obj, target )
        file obj => src do |task|
            puts "#{target.name} | Compiling #{src} ..."
            compile_file( src, target)
            puts "#{target.name} | Compiling #{src} DONE"
        end
    end
end

def get_library_cflags(libname, fallback="-I#{VCPKG_PACKAGES_ROOT}/include")

    pkg_config_flags = ['--cflags']
    
    # try to get pkg-config flags first
    has_pkg_config = system("#{PKGCONF} --exists #{libname}")

    if has_pkg_config
        flags = `#{PKGCONF} #{pkg_config_flags.join(" ")} #{libname}`.chomp
    else
        flags = fallback
    end

    return flags
end

def get_library_linker_flags(libname, type, fallback="-l#{libname}")

    type == "static" or raise "Unexpected type: #{type}, for now only 'static' is supported."
    
    pkg_config_flags = ['--libs']
    
    #if is_static
        pkg_config_flags.append("--static")
    #end

    # try to get pkg-config flags first
    has_pkg_config = system("#{PKGCONF} --exists #{libname}")

    if has_pkg_config
        flags = `#{PKGCONF} #{pkg_config_flags.join(" ")} #{libname}`.chomp
    else
        flags = fallback
    end

    return flags
end

# Target utilities (end) ------------------------------------------------------------------------------------------------

# Others

def vcpkg_install()
    system("vcpkg install --triplet #{VCPKG_TRIPLET} --x-install-root=#{VCPKG_INSTALL_ROOT}", exception: true)
end

def count_lines_of_code(at_location = "./")
    begin
        puts "Counting lines .."
        sh "#{BINARY_CLOC} --by-file-by-lang #{at_location}", verbose: true
        puts "Counting lines DONE"
    rescue
        $stderr.puts "Error: Unable to count lines. The program '#{BINARY_CLOC}' is required for that, make sure you have it installed."
                puts "       Note that cloc is available at https://github.com/AlDanial/cloc"
        exit 1
    end
end
