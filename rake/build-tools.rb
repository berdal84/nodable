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

require 'rbconfig' # for RbConfig::CONFIG (access to build_os etc.)
require 'optparse' # for OptionParser (parse flags)

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

TARGET_TYPE_OBJECTS     = "self_objects"
TARGET_TYPE_EXECUTABLE  = "executable"
PLATFORM_WINDOWS        = "windows"
PLATFORM_LINUX          = "linux"
PLATFORM_WEB            = "web"
PLATFORMS               = [PLATFORM_WINDOWS, PLATFORM_LINUX, PLATFORM_WEB]

PLATFORM_DEFAULT = ->() {
    case BUILD_OS
    when BUILD_OS_WINDOWS
        return PLATFORM_WINDOWS
    when BUILD_OS_LINUX
        return PLATFORM_LINUX
    end
    raise "Unable to determine PLATFORM_DEFAULT: BUILD_OS is #{BUILD_OS}"
}.call()

CONFIG_DEBUG            = "debug"
CONFIG_OPTIMIZED        = "optimized"
CONFIG_RELEASE          = "release"
CONFIG_DEFAULT          = CONFIG_DEBUG
BUILD_CONFIGS           = [CONFIG_DEBUG, CONFIG_OPTIMIZED, CONFIG_RELEASE]
 
BINARY_CLANG            = "clang"
BINARY_EMCC             = "emcc"  # Assumes emsdk_env has been initialized
BINARY_EMRUN            = "emrun" # Assumes emsdk_env has been initialized
BINARY_CLOC             = 'cloc'

# Enums (end) ------------------------------------------------------------------------------------------

# Command Line Arguments ------------------------------------------------------------------------------------

@options_parser = OptionParser.new
@options_parser.banner = "Usage: rake <task> [-- options]\n\nOptions:"

def _get_environment_variable_or(env_var_name, default=nil, allowed_values=nil)

    has_env_var = ENV.key?(env_var_name)
    
    # Always check for the default value
    if (allowed_values != nil and !allowed_values.include?(default))
        raise "Invalid default value for ENV[#{env_var_name}]: '#{default}'. Allowed values: #{allowed_values.join(', ')}"
    end

    value = has_env_var ? ENV[env_var_name] : default

    if (allowed_values != nil and !allowed_values.include?(value))
        raise "Invalid ENV[#{env_var_name}]: '#{value}'. Allowed values: #{allowed_values.join(', ')}"  
    end
    
    return value      
end

# Declare/define a struct to store parsed @options
@options = Struct.new(
    :verbose,
    :config,
    :build_dir,
    :platform,
    :headless_tests,
    keyword_init: true
).new(
                    # We read from environment, it is sometimes more convenient to define them once rather than setting flags multiple times for each command.
    verbose:        _get_environment_variable_or('VERBOSE'          , default=false),
    build_dir:      _get_environment_variable_or('BUILD_DIR'        , default=nil), 
    config:         _get_environment_variable_or('CONFIG'           , default=CONFIG_DEFAULT    , allowed_values=BUILD_CONFIGS ),
    platform:       _get_environment_variable_or('PLATFORM'         , default=PLATFORM_DEFAULT  , allowed_values=PLATFORMS),
    headless_tests: _get_environment_variable_or('HEADLESS_TESTS'   , default=false),
)


@options_parser.on('--platform=PLATFORM', PLATFORMS, "#{PLATFORMS.join("|")} (default: #{PLATFORM_DEFAULT})",  ) do |value|
    @options.platform = value
end

@options_parser.on('--config=CONFIG', BUILD_CONFIGS, "#{BUILD_CONFIGS.join("|")} (default: #{CONFIG_DEFAULT})") do |value|
    @options.config = value
end

@options_parser.on('--build-dir=BUILD_DIR', "Build directory (default: build-{platform}-{config})") do |value|
    @options.build_dir = value
end

@options_parser.on("-v", "--verbose", "Print diagnostic messages") {
    @options.verbose = true
}

@options_parser.on("--headless-tests", "Skip tests requiring a GUI") {
    @options.headless_tests = true
}

# Extract flags (after `--`)
flags = ARGV.drop(1)
flags_index = ARGV.index('--')
if flags_index != nil
    flags = ARGV[(flags_index + 1)..-1]
end

# Parse flags and handle errors
begin
    @options_parser.parse!(flags)
rescue OptionParser::InvalidOption, OptionParser::MissingArgument, OptionParser::InvalidArgument => e
    $stdout.puts e
    $stdout.puts @options_parser.help
    $stderr.puts "Unable to parse flags, see reason message and help above."
    exit 1
end

# Command Line Arguments (end) ----------------------------------------------------------------------------------

# Global Constants ----------------------------------------------------------------------------------------------

PLATFORM        = @options.platform
LINUX           = PLATFORM == PLATFORM_LINUX
WINDOWS         = PLATFORM == PLATFORM_WINDOWS
WEB             = PLATFORM == PLATFORM_WEB
DESKTOP         = !WEB
HEADLESS_TESTS  = !!@options.headless_tests
VERBOSE         = @options.verbose

# Triplet (we use VCPKG naming convention)
VCPKG_TRIPLET = ->() {

    case PLATFORM
    when PLATFORM_LINUX
        return "x64-linux"
    when PLATFORM_WINDOWS
        return "x64-windows-static" # windows convention is different than linux, dynamic by default
    when PLATFORM_WEB
        return "wasm32-emscripten"
    end

    raise "Unknown VCPKG_TRIPLET for this platform: #{PLATFORM}"
    
}.call()

# Path to installed folder (we decided to separate linux and windows folders)
VCPKG_INSTALL_ROOT  = ->() {
    case PLATFORM
    when PLATFORM_LINUX
        return "./vcpkg/linux"    
    when PLATFORM_WINDOWS
        return "./vcpkg/windows" 
    when PLATFORM_WEB
        return "./vcpkg/emscripten" 
    end
    raise "Unable to determine VCPKG_INSTALL_ROOT: PLATFORM is #{PLATFORM}"
}.call()

VCPKG_PACKAGES_ROOT = "#{VCPKG_INSTALL_ROOT}/#{VCPKG_TRIPLET}"

PKGCONF_BINARY = ->() {

    path = "#{VCPKG_PACKAGES_ROOT}/tools/pkgconf/pkgconf"

    if BUILD_OS == BUILD_OS_WINDOWS 
        path = path.ext("exe")
    end

    if (not File.exist?(path)) && PLATFORM != PLATFORM_WEB
        puts "Warning: PKGCONF_BINARY '#{path}' does not exist!"
        puts "         In principle this file is in the source code."
        puts "         Perhaps you delete it and forgot to run 'rake vcpkg' ?"
    end

    path 

}.call()

PKGCONF                 = "#{PKGCONF_BINARY} --with-path #{VCPKG_PACKAGES_ROOT}/lib/pkgconfig"
HOST_OS                 = RbConfig::CONFIG['host_os']
CONFIG                  = @options.config
RELEASE                 = CONFIG == CONFIG_RELEASE
DEBUG                   = CONFIG == CONFIG_DEBUG
OPTIMIZED               = CONFIG == CONFIG_OPTIMIZED
BUILD_DIR               = @options.build_dir || "build-#{PLATFORM}-#{CONFIG}"
ZIPPED_DIST_DIR         = "dist"
GITHUB_ACTIONS          = ENV["GITHUB_ACTIONS"]
HTTP_SERVER_HOSTNAME    = "0.0.0.0"  # TODO: add to flags
HTTP_SERVER_PORT        = "8000" # TODO: add to flags
HTTP_SERVER_URL         = "http://#{HTTP_SERVER_HOSTNAME}:#{HTTP_SERVER_PORT}/"
COMPILER                = WEB ? BINARY_EMCC : BINARY_CLANG # Same binary to compile both C and CPP
LINKER                  = WEB ? BINARY_EMCC : BINARY_CLANG # Same binary to compile both C and CPP

CXXSTD98                = "-std=c++98" # see https://clang.llvm.org/cxx_status.html
CXXSTD03                = "-std=c++03"
CXXSTD11                = "-std=c++11"
CXXSTD14                = "-std=c++14"
CXXSTD17                = "-std=c++17"
CXXSTD20                = "-std=c++20"
CXXSTD23                = "-std=c++23"
CXXSTD2X                = "-std=c++2c"

ALLOWED_CXXSTDS = [
    CXXSTD98,
    CXXSTD03,
    CXXSTD11,
    CXXSTD14,
    CXXSTD17,
    CXXSTD20,
    CXXSTD23,
    CXXSTD2X,
]

if @options.verbose
puts "------------------------------------------------------------------------------------------------------"
puts "@options: ............ #{@options}"
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
end # if @options.verbose

# Global Constants (end) ------------------------------------------------------------------------------------------

# Target API ------------------------------------------------------------------------------------------------

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
    :includes, # list of path dir to include
    :is_initialized, # is ready to compile (e.g. pkg-config was run)
    :linker_flags,
    :name,
    :sources, # list of .c|.cpp files
    :type, # TARGET_XXX
    :vcpkg, # list of (static) vcpkg package names
    :unity_build_on, # enable/disable unity build
    :unity_build_slice_size, # file count per slice
    :_build_dir,
    :_dist_dir,
    :_bin_dir,
    :_dep_dir,
    :_obj_dir,
    :_src_dir,
)

def target(
    name,
    type,
    cxxstd = CXXSTD20
    )

    raise "Wrong value for cxxstd:\n  Allowed: #{ALLOWED_CXXSTDS.join ", "}.\n  Actual: #{cxxstd}" if not ALLOWED_CXXSTDS.include?(cxxstd)
    
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
    target.includes                 = FileList[]
    target.is_initialized           = false;
    target.linker_flags             = []
    target.name                     = name
    target.sources                  = FileList[]
    target.type                     = type
    target.vcpkg                    = []
    target.unity_build_on           = false
    target.unity_build_slice_size   = RELEASE ? 512 : 4

    target._build_dir               = "#{BUILD_DIR}/#{target.name}"
    target._dist_dir                = "#{target._build_dir}/dist"
    target._bin_dir                 = "#{target._build_dir}/bin"
    target._dep_dir                 = "#{target._build_dir}/dep"
    target._obj_dir                 = "#{target._build_dir}/obj"
    target._src_dir                 = "#{target._build_dir}/src"

    if VERBOSE
        target.compiler_flags   |= ["-v"]
        target.linker_flags     |= ["-v"]
    end

    target.cxx_flags |= [
        "-x c++", # we use clang, not clang++ (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-x)
        cxxstd,   # see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-std 
    ]
    
    if WINDOWS        
        target.compiler_flags |=[
            "-fms-extensions" # turn ON MSVC compatibility
        ]
    end

    # Set some flags related to optimization depending on CONFIG
    case CONFIG
    when CONFIG_RELEASE

        target.compiler_flags |= [
            "-Oz", # O2 + extra reduced size (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption)
            # "-pedantic", # https://clang.llvm.org/docs/UsersManual.html#cmdoption-pedantic
            # "-Werror", # It's too much!
        ]

    when CONFIG_OPTIMIZED

        target.compiler_flags |= [
            "-g",  # Generate debug information (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-g)
            "-O2", # Moderate level of optimization which enables most optimizations. (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-O2)
        ]

    when CONFIG_DEBUG

        target.compiler_flags |= [
            "-g",  # Generate debug information (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-g)
            "-O0", # No optimizations (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-O0)
        ]

    end

    return target

end

def target_src_to_obj( target, src )   
    "#{target._obj_dir}/#{src.ext(".o")}"
end

def target_src_to_dep( target, src )
    "#{target._dep_dir}/#{src.ext(".d")}"
end

def target_find_alldeps_from_src( target, src )
    
    # get *.d file
    dep = target_src_to_dep( target, src )

    deps = []

    if File.exist?(dep)
        content = File.read(dep)
        content = content.split(": ")[1]
        content = content.gsub(/\\$/, '').strip  # Remove line continuations
        deps    = content.split " "
    end

    deps
end

def target_find_src_from_obj( target, obj )
    stem = obj.sub("#{target._obj_dir}/", "").ext("")    
    target.sources.detect{|src| src.ext("") == stem } or raise "unable to find #{obj}'s source (stem: #{stem})"
end

def target_sources( target, recursively = false )
    
    sources = []

    if recursively
        target.depends_on_target.each do |other_target|
            sources |= target_sources( other_target, recursively: true )
        end
    end

    sources |= target.sources

    sources
end

def target_objects( target, recursively = false )
    
    target_initialize_if_needed(target)

    objects = []

    if recursively
        target.depends_on_target.each do |each_dependency_target|
            objects |= target_objects( each_dependency_target, recursively: true )
        end
    end

    objects |= target.sources.map{|src| target_src_to_obj( target, src) };

    objects
end

def target_binary_file( target )
    path = "#{target._bin_dir}/#{target.name}"
    if WEB
        path = path.ext("html") # will generate also a *.js, *.wasm, *.wasm.map and *.data
    elsif DESKTOP and WINDOWS
        path = path.ext("exe")
    end
    path
end

$_mutex_initializing = Mutex.new

def target_initialize_if_needed(target)

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

        debug( target, "Initialization .." )
    
        # 1) Generate flags for linked libraries
        #    It relies on pkgconf for vcpkg, if library can't be found we add a default flag (-lmylib)
        #
        debug( target, "Generate flags for vcpkg (#{target.vcpkg})..")
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

        debug( target, "-- cxx_flags added:    #{temp_cxx_flags}")
        debug( target, "-- linker_flags added: #{temp_linker_flags}")
        debug( target, "Generate vcpkg flags DONE")
        
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

        # 3) Generate unity_build slices
        if target.unity_build_on && target.sources.size > 1 && target.unity_build_slice_size > 1

            debug(target, "Unity Build - Slicing ...")

            unity_sources = FileList[]

            # Generate each slice
            require 'digest'
            slices = target.sources.each_slice(target.unity_build_slice_size)
            slices.each_with_index do |slice, index|
                
                content = [
                    "//",
                    "// This code was generated by #{__FILE__}, do not edit.",
                    "//",
                    *slice.map{|file| "#include \"#{File.absolute_path file}\"" }
                ].join("\n")

                hash       = Digest::SHA256.hexdigest(content)[0..17]     # We want to make sure filename changes even if index does not
                filename   = "#{target._src_dir}/slice-#{1+index}-#{hash}.cpp"

                unity_sources += [filename]

                FileUtils.mkdir_p File.dirname( filename )

                if !File.exist?(filename)
                    File.write(filename, content)
                end

                # puts "Slice files #{slice}, content is:"
                # puts content
            end

            # Then we replace the sources by the unity build ones
            # We want this change to be propagater to any target that depends on this target.
            target.sources = unity_sources

        end
        
        # 4) init some vars
        target.compiled_objects_count = 0
        
        target.is_initialized = true
        
        debug(target, "Initialization DONE")
    }    
end

def target_compile_file(target, src)

    target_initialize_if_needed(target)
    
    is_cpp = File.extname( src ) == ".cpp"

    dep = target_src_to_dep( target, src )
    obj = target_src_to_obj( target, src )

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

def target_link( target )

    if (target.type != TARGET_TYPE_EXECUTABLE)
        raise "#{target.name}'s type is expected to be: '#{TARGET_TYPE_EXECUTABLE}', actual: #{target.type}"
    end

    target_initialize_if_needed(target)

    log(target, "Linking ...")

    binary = target_binary_file(target)

    args = [
        target.compiler_flags,
        target.cached_defines_flags,
        "-o #{binary}", # Output binary (emcc requires "-o path/to/file" syntax )
        target_objects(target, recursively: true ),
        target.linker_flags
    ]
    
    if WEB
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

    log(target, "Linking DONE - (#{binary})")
end

def target_clean(target, recursively = false)
    # FileUtils.rm_rf was too slow, that's why we use:
    sh "rm", "-rf", target._build_dir

    if recursively
        target.depends_on_target.each do |each_dependency|
            target_clean(each_dependency, recursively: true)
        end
    end
end

def target_define_tasks(target)
namespace target.name do

    task :clean do
        target_clean( target )
    end

    task :clean_all do
        target_clean( target, recursively: true )
    end

    task :rebuild     => [:clean    , :build]
    task :rebuild_all => [:clean_all, :build]

    # Define a task per source to build
    # Not that we do not include the sources from dependencies here,
    # each dependency declares its own tasks, if this target requires an other target's task,
    # it will be invoked by rake automatically.
    target_objects(target).each_with_index do |obj, index|
        src  = target_find_src_from_obj(target, obj)
        deps = target_find_alldeps_from_src(target, src) # *.c|cpp, and any deps in *.d
        file obj => [src, *deps] do |task|
            log(target, "Compiling #{src} ...")
            target_compile_file( target, src )
            target.compiled_objects_count += 1
        end
    end

    case target.type
    when TARGET_TYPE_OBJECTS
        
        multitask :build => target_objects(target, recursively: true) do
            update_llvm_json_compilation_database()
        end

    when TARGET_TYPE_EXECUTABLE

        binary = target_binary_file(target)

        file binary => target_objects(target, recursively: true) do
            update_llvm_json_compilation_database()
            target_link(target)
        end

        task :build => binary do

            # Copy assets
            target.assets.each_with_index do |file_pattern, i|
                src, dst = file_pattern_split(file_pattern)
                file_copy_or_overwrite( src, "#{target._bin_dir}/#{dst}" )
            end  

            log(target, "Build DONE")
        end
        
        task :run => binary do
            log(target, "Running ...")
            case PLATFORM
            when PLATFORM_WEB
                system("#{BINARY_EMRUN} --hostname #{HTTP_SERVER_HOSTNAME} --port #{HTTP_SERVER_PORT} #{binary.ext("html")}", exception: true)
            else
                system(binary, exception: true)
            end
            log(target, "Running DONE")
        end

        task :zip do

            # Copy binary and related files + assets into dist folder
            if WEB
                binary_filename = File.basename(binary).ext("") # clean extension out
                binary_and_additionnal_files = FileList[
                "#{target._bin_dir}/#{binary_filename}.html", # User can override this by adding an asset
                "#{target._bin_dir}/#{binary_filename}.js",
                "#{target._bin_dir}/#{binary_filename}.wasm",
                "#{target._bin_dir}/#{binary_filename}.wasm.map",
                "#{target._bin_dir}/#{binary_filename}.data",
                ];
                binary_and_additionnal_files.each do |each|;
                    file_copy_or_overwrite( each, "#{target._dist_dir}/#{File.basename(each)}" )
                end
            else
                # Copy the binary
                binary_filename = File.basename(binary)
                file_copy_or_overwrite( binary, "#{target._dist_dir}/#{binary_filename}" )
            end
            target.assets.each_with_index do |pattern, i|
                src, dst = file_pattern_split(pattern)
                file_copy_or_overwrite( src, "#{target._dist_dir}/#{dst}" )
            end

            # Zip all the files
            zip_filename = File.absolute_path "#{ZIPPED_DIST_DIR}/#{target.name}-#{PLATFORM}-#{CONFIG}.zip"
            
            FileUtils.rm zip_filename if File.exist? zip_filename
            
            FileUtils.mkdir_p File.dirname( zip_filename )
            
            level = 9 # zip compression level

            if BUILD_OS == BUILD_OS_WINDOWS
                # GitHub Runner OS has 7zip on windows
                sh "7z a -tzip -mx#{level} #{zip_filename} #{target._dist_dir}/*"
            else
                sh "cd #{target._dist_dir} && zip -r -#{level} #{zip_filename} ."
            end
        end
    end # case target.type
end # namespace end
end

# Target API (end) ------------------------------------------------------------------------------------------------

# Others

def clobber()
    # FileUtils.rm_rf was too slow, that's why we use:
    sh "rm", "-rf", BUILD_DIR, ZIPPED_DIST_DIR
end

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

def log(target, message)
    puts "[#{target.name}:#{PLATFORM}:#{CONFIG}] #{message}"
end

def debug(target, message)
    log(target,message) if VERBOSE
end

def print_help()

    # Print regular option parser help
    print @options_parser.help    
    puts "Tasks:"

    # Print tasks
    # mimics rake --tasks without invoking rake again
    column_width = 32 # matches with option_parser formatting
    Rake.application.tasks.each do |task|
        next if !task.comment
        name = task.name + ' '
        name = name.ljust(column_width, '.') # dots from last space to comment
        puts "    #{name} #{task.full_comment}"
    end
end

def update_llvm_json_compilation_database()

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
    command_files = FileList["#{BUILD_DIR}/**/*.json"]

    # combine
    commands = command_files.map{|f| File.read(f)}.join()
    content = "[" + commands[0...-2] + "]" # -2: remove trailing comma
    File.write( output_file, content)
end

def file_copy_or_overwrite(src, dst)
    
    # Ensure destination folder exists
    dst_dir = File.dirname(dst)
    FileUtils.mkdir_p dst_dir

    # Copy file (will overwrite)
    FileUtils.cp( src, dst )

    puts "  File copied: #{src} => #{dst}"
end

# With a string like "<src>[:<dest>]", returns src, dst.
# dst = src when not defined.
def file_pattern_split(pattern)

    arr = pattern.split(':') 
    src = arr[0] or raise ("Wrong asset pattern: '#{pattern}', expecting '<src>[:<dest>]'")
    dst = "#{arr[1] || src}"

    return src, dst
end

