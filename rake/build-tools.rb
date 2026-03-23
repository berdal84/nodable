#---------------------------------------------------------------------------------
# BUILD TOOLS - ruby functions / rake tasks to build a simple C++ project
#----------------------------------------------------------------------------------
#
# A friend of mine has always been pisted about CMake, and he converted me to
# his "I'll do it by myself" state of mind.
# I have to admit that I was doubting, but eventually I could do what I wanted
# with ~500 additionnal lines of code to replace the feature I really wanted
# from CMake.
#
# The constrains I set for this script are:
#   - only two platforms: DESKTOP and WEB,
#   - only two OS: windows and linux, with a single architecture (x64 / x86_64)
#   - only two compilers: clang v21+ for DESKTOP, and emcc (emscriptem)
#     for the WEB,
#   - no dynamic dependencies such as submodules: the external dependencies sources
#     are stored in this repository, except big stuff like MSVC, clang, etc.
#     For large dependencies that would require long compilation time: store
#     them as binary (using vcpkg).
#
# Some of the consequences:
#   - I am now free to do whatever I want in my build system (putain oui!),
#   - I undertand what's happening, I can see the code and debug like with a real
#     programming language.
#   - I reduced the compilation time from ~10 to ~2 minutes (mainly due to storing
#     libs as binary),
#   - the repo size grew a little bit.
#
#                                                  Bérenger, 2026
#

require "rbconfig"
require 'json'
require 'date' # To add date in .clang export
require 'optparse'
require 'rubygems'

# First, detect BUILD_OS and ARCHitecture:

BUILD_OS_LINUX   = "linux"   # Operating System (we use VCPKG naming convention, type: `vcpkg help triplet`)
BUILD_OS_WINDOWS = "windows"

BUILD_OS = ->() { 
    
    build_os = RbConfig::CONFIG['build_os']

    if build_os.include?("linux")
        return BUILD_OS_LINUX
    elsif build_os.include?("mingw32") # Ruby is built on mingw32 (w32 does not stands for 32bits)
        return BUILD_OS_WINDOWS
    end

    raise "This script is not compatible with #{build_os}!"
}.call()   

# Architecture (we use VCPKG naming convention)
ARCH_64    = "x64"
ARCH_32    = "x32"

DEFAULT_ARCH = ->() {

    host_cpu = RbConfig::CONFIG['host_cpu']

    if host_cpu == "x64"
        return ARCH_64
    end
    
    if host_cpu == "x86_64"
        return ARCH_64
    end

    raise "This script is not compatible with #{host_cpu} architecture!"

}.call()

BUILD_ARCH = DEFAULT_ARCH

# Enums ------------------------------------------------------------------------------------------------

TARGET_TYPE_OBJECTS      = "self_objects"
TARGET_TYPE_EXECUTABLE   = "executable"

TARGET_WINDOWS           = "windows"
TARGET_LINUX             = "linux"
TARGET_EMSCRIPTEN        = "emscripten" # web assembly
TARGET_DEFAULT           = BUILD_OS == BUILD_OS_WINDOWS ? TARGET_WINDOWS : TARGET_LINUX
TARGETS                  = [TARGET_WINDOWS, TARGET_LINUX, TARGET_EMSCRIPTEN]

BUILD_CONFIG_DEBUG       = "debug"
BUILD_CONFIG_OPTIMIZED   = "optimized"
BUILD_CONFIG_RELEASE     = "release"
BUILD_CONFIG_DEFAULT     = BUILD_CONFIG_DEBUG
BUILD_CONFIGS            = [BUILD_CONFIG_DEBUG, BUILD_CONFIG_OPTIMIZED, BUILD_CONFIG_RELEASE]
 
BINARY_CLANG             = "clang"
BINARY_EMCC              = "emcc"
BINARY_EMRUN             = "emrun"
BINARY_CLOC              = 'cloc'

# Enums (end) ------------------------------------------------------------------------------------------

# Command Line Arguments ------------------------------------------------------------------------------------

# Declare/define a struct to store parsed options
OPTIONS = Struct.new(
    :verbose,
    :build_config,
    :build_dir,
    :target,
    :ignore_gui_tests,
    keyword_init: true
).new(
    verbose:            false,
    build_dir:          nil, 
    build_config:       BUILD_CONFIG_DEFAULT,
    target:             TARGET_DEFAULT,
    ignore_gui_tests:   false,
)

$option_parser = OptionParser.new

$option_parser.banner = "Usage: rake <task> -- [flags]"

$option_parser.on('-t', '--target=TARGET', TARGETS, "#{TARGETS.join("|")} (default: #{TARGET_DEFAULT})",  ) do |value|
    OPTIONS.target = value
end

$option_parser.on('-b', '--build=BUILD_CONFIG', BUILD_CONFIGS, "#{BUILD_CONFIGS.join("|")} (default: #{BUILD_CONFIG_DEFAULT})") do |value|
    OPTIONS.build_config = value
end

$option_parser.on('-o', '--output-dir=OUTPUT_DIR', "Build output directory, absolute or relative to the rakefile (default: 'build-{target}-{arch}-{os}-{build_config}')") do |value|
    OPTIONS.build_dir = value
end

$option_parser.on("-v", "--verbose", "Print diagnostic messages") {
    OPTIONS.verbose = true
}

$option_parser.on("--no-gui-tests", "Disable any test that requires to open a window") {
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

# Helpers to simplify branching (if LINUX ... elif WINDOWS ... elif EMSCRIPTEN ... end )
LINUX      = OPTIONS.target == TARGET_LINUX
WINDOWS    = OPTIONS.target == TARGET_WINDOWS
EMSCRIPTEN = OPTIONS.target == TARGET_EMSCRIPTEN
WEB        = EMSCRIPTEN
DESKTOP    = LINUX || WINDOWS
ARCH       = EMSCRIPTEN ? ARCH_32 : DEFAULT_ARCH

# Triplet (we use VCPKG naming convention)
VCPKG_TRIPLET = ->() {

    if LINUX
        return "x64-linux"
    end

    if WINDOWS
        return "x64-windows-static" # windows convention is different than linux, dynamic by default
    end

    if EMSCRIPTEN
        return "wasm32-emscripten"
    end

    raise "Unknown VCPKG_TRIPLET for this target"
    
}.call()

# Path to installed folder (we decided to separate linux and windows folders)
VCPKG_INSTALL_ROOT  = "./vcpkg/#{OPTIONS.target}"
VCPKG_PACKAGES_ROOT = "#{VCPKG_INSTALL_ROOT}/#{VCPKG_TRIPLET}"

PKGCONF_BINARY = ->() {

    path = "#{VCPKG_PACKAGES_ROOT}/tools/pkgconf/pkgconf"

    if BUILD_OS == BUILD_OS_WINDOWS 
        path = path.ext("exe")
    end

    if not File.exist?(path)
        $stderr.puts "Error: PKGCONF_BINARY '#{path}' does not exist! In principle this file is in the source code, but perhaps you delete it and forgot to run 'rake vcpkg'? "
    end

    path 

}.call()

PKGCONF              = "#{PKGCONF_BINARY} --with-path #{VCPKG_PACKAGES_ROOT}/lib/pkgconfig"
HOST_OS              = RbConfig::CONFIG['host_os']
RELEASE              = OPTIONS.build_config == BUILD_CONFIG_RELEASE
DEBUG                = OPTIONS.build_config == BUILD_CONFIG_DEBUG
OPTIMIZED            = OPTIONS.build_config == BUILD_CONFIG_OPTIMIZED
OUTPUT_DIR            = File.expand_path( OPTIONS.build_dir || "build-#{OPTIONS.target}-#{OPTIONS.build_config}", Dir.pwd )
DIST_DIR             = "#{OUTPUT_DIR}/dist" # Distribution files will be copied there (after a build)
OBJ_DIR              = "#{OUTPUT_DIR}/obj"
DEP_DIR              = "#{OUTPUT_DIR}/dep"
BIN_DIR              = "#{OUTPUT_DIR}/bin" # binaries will be generated there

GITHUB_ACTIONS       = ENV["GITHUB_ACTIONS"]
HTTP_SERVER_HOSTNAME = "0.0.0.0"  # TODO: add to flags
HTTP_SERVER_PORT     = "8000" # TODO: add to flags
HTTP_SERVER_URL      = "http://#{HTTP_SERVER_HOSTNAME}:#{HTTP_SERVER_PORT}/"
COMPILER             = EMSCRIPTEN ? BINARY_EMCC : BINARY_CLANG # Same binary to compile both C and CPP
LINKER               = EMSCRIPTEN ? BINARY_EMCC : BINARY_CLANG # Same binary to compile both C and CPP

if OPTIONS.verbose
puts "------------------------------------------------------------------------------------------------------"
puts "OPTIONS: ............ #{OPTIONS}"
puts "RUBY version: ....... #{`ruby -v`}"
puts "HOST_OS: ............ #{HOST_OS}"
puts "BUILD_OS:............ #{BUILD_OS}"
puts "BUILD_ARCH: ......... #{BUILD_ARCH}"
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
    :assets, # List of patterns like: "<source>" or "<source>:<destination>"
    :c_flags,
    :cached_defines_flags,
    :cached_includes_flags,
    :compiled_objects_count,
    :compiler_flags,
    :cxx_flags,
    :defines,
    :depends_on_target, # list of other targets to link with (if their sources are not compiled yet, it will compile them as *.o and will be linked)
    :distribute,
    :includes, # list of path dir to include
    :is_initialized, # is ready to compile (e.g. pkg-config was run)
    :linker_flags,
    :name,
    :sources, # list of .c|.cpp files
    :type, # TARGET_XXX
    :vcpkg, # list of (static) vcpkg package names
    keyword_init: true # If the optional keyword_init keyword argument is set to true, .new takes keyword arguments instead of normal arguments.
)

def bt_target(name, type)
    
    target = Target.new

    target.assets                   = FileList[]
    target.c_flags                  = []
    target.cached_defines_flags     = ""
    target.cached_includes_flags    = ""
    target.compiled_objects_count   = []
    target.compiler_flags           = []
    target.cxx_flags                = []
    target.defines                  = []
    target.depends_on_target        = []
    target.distribute               = false
    target.includes                 = FileList[]
    target.is_initialized           = false;
    target.linker_flags             = []
    target.name                     = name
    target.sources                  = FileList[]
    target.type                     = type
    target.vcpkg                    = []

    return target

end

def bt_convert_src_to_obj( obj )
    "#{OBJ_DIR}/#{obj.ext(".o")}"
end

def bt_convert_src_to_dep( src )
    "#{DEP_DIR}/#{src.ext(".d")}"
end

def bt_find_deps_for_src( src )
    
    # get *.d file
    dep = bt_convert_src_to_dep( src )

    deps = []

    if File.exist?(dep)
        content = File.read(dep)
        content = content.split(": ")[1]
        content = content.gsub(/\\$/, '').strip  # Remove line continuations
        deps    = content.split " "
    end

    deps
end

def bt_find_src_for_obj( sources, obj )
    stem = obj.sub("#{OBJ_DIR}/", "").ext("")
    sources.detect{|src| src.ext("") == stem } or raise "unable to find #{obj}'s source (stem: #{stem})"
end

def bt_convert_array_of_src_to_obj( sources )
    sources.map{|src| bt_convert_src_to_obj(src) };
end

def bt_target_get_sources( target, recursively = false )
    
    sources = []

    if recursively
        target.depends_on_target.each do |other_target|
            sources |= bt_target_get_sources( other_target, recursively: true )
        end
    end

    sources |= target.sources

    sources
end

def bt_target_get_objects( target, recursively = false )
    
    objects = []

    if recursively
        target.depends_on_target.each do |each_dependency_target|
            objects |= bt_target_get_objects( each_dependency_target, recursively: true )
        end
    end

    objects |= bt_convert_array_of_src_to_obj( target.sources )

    objects
end

def bt_target_get_binary_path( target )
    path = "#{BIN_DIR}/#{target.name}"
    if EMSCRIPTEN
        path = path.ext("js") # will generate also a .wasm, .wasm.map and *.data
    elsif DESKTOP and WINDOWS
        path = path.ext("exe")
    end
    path
end

$_mutex_initializing = Mutex.new

def bt_target_initialize_if_needed(target)

    # Perform a quick check that won't lock the thread (readonly)
    if target.is_initialized
        return
    end

    # We synchronize this scope to make sure only 1 thread at a time can run it
    $_mutex_initializing.synchronize {

        # There is a possibility where 1 task already started to execute this scope while an other was waiting to execute it too.
        # In such case, once the first finishes the execution (and set the flag to true) the second enter
        # But, the second should abort.
        if target.is_initialized
            return
        end

        bt_debug( target, "Initialization .." )
    
        # 1) Generate flags for linked libraries
        #    It relies on pkgconf for vcpkg, if library can't be found we add a default flag (-lmylib)
        #
        bt_debug( target, "Generate flags for vcpkg (#{target.vcpkg})..")
        # We must add default include path for headers and libraries because some vcpkg do not have a .pc file
        # and their location is 99% of the time in those two folders:
        temp_cxx_flags    = ["-I#{VCPKG_PACKAGES_ROOT}/include"]
        temp_linker_flags = ["-L#{VCPKG_PACKAGES_ROOT}/lib"]

        target.vcpkg.each do |vcpkg_name|

            has_pkg_config = system("#{PKGCONF} --exists #{vcpkg_name}")

            if !has_pkg_config
                temp_linker_flags |= ["-l#{vcpkg_name}"] # By default, we simply link it, considering that 99% of the time the *.lib|a|so is in the base folder.
            else
                lib_cxx_flags      = `#{PKGCONF} --cflags #{vcpkg_name}`.chomp        # Get compiler flags (ex: "-I/path/to/folder"      
                lib_linker_flags   = `#{PKGCONF} --libs --static #{vcpkg_name}`.chomp # Get linker flags (ex: "-L/path/to/lib/folder -lxxx" 

                temp_cxx_flags    |=    lib_cxx_flags.split(" ") # we use |= to make sure there is no duplicates
                temp_linker_flags |= lib_linker_flags.split(" ")
            end

        end

        # we use += here because we would like to see compiler warnings if a flag from these temp_xxx_flags already exist in the target.xxx_flags,
        # that would mean some flags can be removed from target.xxx_flags perhaps...
        target.cxx_flags    += temp_cxx_flags 
        target.linker_flags += temp_linker_flags

        bt_debug( target, "-- cxx_flags added:    #{temp_cxx_flags}")
        bt_debug( target, "-- linker_flags added: #{temp_linker_flags}")
        bt_debug( target, "Generate vcpkg flags DONE")
        
        # Enable LTO (link time optimization)
        if RELEASE

            lto_flags = [
                "-flto",        # lto|lto=thin, LTO: link time optimization
                "-fuse-ld=lld"  # required by LTO
            ]
            
            target.linker_flags   |= lto_flags;
            target.compiler_flags |= lto_flags;
        end

        # 2) Cache some flags as string to share the data accross multiple compilation units
        #
        target.cached_defines_flags  = target.defines.map{|d|  "--define-macro=\"#{d}\"" }.join(" ") # see https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-D-macro
        target.cached_includes_flags = target.includes.map{|f| "--include-directory=#{File.absolute_path(f)}"}.join(" ") # see https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-I-dir

        target.is_initialized = true

        # 3) init some vars
        target.compiled_objects_count = 0

        bt_debug(target, "Initialization DONE")
    }    
end

def bt_target_compile_file(target, src)

    bt_target_initialize_if_needed(target)
    
    is_cpp = File.extname( src ) == ".cpp"

    dep = bt_convert_src_to_dep( src )
    obj = bt_convert_src_to_obj( src )

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

def bt_target_link( target )

    if (target.type != TARGET_TYPE_EXECUTABLE)
        raise "Target type is expected to be: '#{TARGET_TYPE_EXECUTABLE}', actual: #{target.type}"
    end

    bt_target_initialize_if_needed(target)

    bt_log(target, "Linking ...")

    binary = bt_target_get_binary_path(target)

    args = [
        target.compiler_flags,
        target.cached_defines_flags,
        "-o #{binary}", # Output binary (emcc requires "-o path/to/file" syntax )
        bt_target_get_objects(target, recursively: true ),
        target.linker_flags
    ]
    
    if EMSCRIPTEN
        if BUILD_OS == BUILD_OS_WINDOWS
            args += ["--output-eol", "windows"]
        elsif BUILD_OS == BUILD_OS_LINUX
            args += ["--output-eol", "linux"]
        else
            raise "Unexpected HOST_OS: #{HOST_OS}"
        end
    end

    FileUtils.mkdir_p File.dirname(binary)

    system("#{LINKER} #{args.join(" ")}", exception: true)

    bt_log(target, "Linking DONE - (#{binary})")
end

def bt_update_llvm_json_compilation_database()

    #
    # Update ./compile_commands.json
    # This file can be read by clangd to perform static analysis (e.g. with cnagd, in VSCode or CLION )
    # 
    # see https://clang.llvm.org/docs/JSONCompilationDatabase.html
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

def bt_file_copy_or_overwrite(src, dst)
    
    # Ensure destination folder exists
    dst_dir = File.dirname(dst)
    FileUtils.mkdir_p dst_dir

    # Copy file (will overwrite)
    FileUtils.cp( src, dst )

    puts "  File copied: #{src} => #{dst}"
end

def bt_tasks_for_target(target)

    task :clean do
        FileUtils.rm_f bt_target_get_objects( target, recursively: false )
    end

    task :clean_all do
        FileUtils.rm_f bt_target_get_objects( target, recursively: true  )
    end

    task :rebuild     => [:clean    , :build]
    task :rebuild_all => [:clean_all, :build]

    # Define a task per source to build
    # Not that we do not include the sources from dependencies here,
    # each dependency declares its own tasks, if this target requires an other target's task,
    # it will be invoked by rake automatically.
    bt_target_get_objects(target).each_with_index do |obj, index|
        src  = bt_find_src_for_obj(target.sources, obj)
        deps = bt_find_deps_for_src(src) # *.c|cpp, and any deps in *.d
        file obj => [src, *deps] do |task|
            bt_log(target, "Compiling #{src} ...")
            bt_target_compile_file( target, src )
            target.compiled_objects_count += 1
            progress = 100 * target.compiled_objects_count / (bt_target_get_objects(target).size() )
        end
    end

    multitask :compile_objects => bt_target_get_objects(target, recursively: true) do
        bt_debug(target, "Updating llvm compilation database ...")
        bt_update_llvm_json_compilation_database()
    end

    if target.type == TARGET_TYPE_OBJECTS
        
        multitask :build => :compile_objects
        
    elsif target.type == TARGET_TYPE_EXECUTABLE

        binary = bt_target_get_binary_path(target)

        file binary => :compile_objects do
            bt_target_link(target)
        end

        task :build => binary do

            if target.distribute            
                
                # Copy the binary
                binary_filename = File.basename(binary)
                bt_file_copy_or_overwrite( binary, "#{DIST_DIR}/#{binary_filename}" )

                # And some additionnal file
                if EMSCRIPTEN
                    binary_no_ext = binary_filename.ext("")
                    binary_and_additionnal_files = FileList[
                       "#{BIN_DIR}/#{binary_no_ext}.wasm",
                       "#{BIN_DIR}/#{binary_no_ext}.wasm.map",
                       "#{BIN_DIR}/#{binary_no_ext}.data",
                    ];
                    binary_and_additionnal_files.each do |each|;
                        bt_file_copy_or_overwrite( each, "#{DIST_DIR}/#{File.basename(each)}" )
                    end
                end
            end  

            # Copy assets
            target.assets.each_with_index do |pattern, i|

                # Handle pattern (format is <src>[:<dest>], by default dest=src)
                arr = pattern.split(':') 
                src = arr[0] or raise ("Wrong pattern: #{pattern}, expecting '<src>[:<dest>]'")
                dst = "#{arr[1] || src}"

                bt_file_copy_or_overwrite( src, "#{BIN_DIR}/#{dst}" )

                if target.distribute
                    bt_file_copy_or_overwrite( src, "#{DIST_DIR}/#{dst}" )
                end
            end

            bt_log(target, "Build DONE")
        end
        
        task :run => :build do
            bt_log(target, "Running ...")
            if DESKTOP
                system(binary, exception: true)
            elsif WEB
                system("#{BINARY_EMRUN} --hostname #{HTTP_SERVER_HOSTNAME} --port #{HTTP_SERVER_PORT} #{binary.ext("html")}", exception: true)
            end
            bt_log(target, "Running DONE")
        end
    end

end

# Target utilities (end) ------------------------------------------------------------------------------------------------

# Others

def bt_vcpkg_install()
    system("vcpkg install --triplet #{VCPKG_TRIPLET} --x-install-root=#{VCPKG_INSTALL_ROOT}", exception: true)
end

def bt_count_lines_of_code(at_location = "./")
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

def bt_log(target, message)
    puts "#{target.name} | #{message}"
end

def bt_debug(target, message)
    if OPTIONS.verbose
        puts "#{target.name} | #{message}"
    end
end