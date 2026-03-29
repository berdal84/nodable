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

@OPTIONS_PARSER = OptionParser.new
@OPTIONS_PARSER.banner = "Usage: rake <task> [-- options]\n\nOptions:"

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

# Declare/define a struct to store parsed @OPTIONS
@OPTIONS = Struct.new(
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


@OPTIONS_PARSER.on('--platform=PLATFORM', PLATFORMS, "#{PLATFORMS.join("|")} (default: #{PLATFORM_DEFAULT})",  ) do |value|
    @OPTIONS.platform = value
end

@OPTIONS_PARSER.on('--config=CONFIG', BUILD_CONFIGS, "#{BUILD_CONFIGS.join("|")} (default: #{CONFIG_DEFAULT})") do |value|
    @OPTIONS.config = value
end

@OPTIONS_PARSER.on('--build-dir=BUILD_DIR', "Build directory (default: build-{platform}-{config})") do |value|
    @OPTIONS.build_dir = value
end

@OPTIONS_PARSER.on("-v", "--verbose", "Print diagnostic messages") {
    @OPTIONS.verbose = true
}

@OPTIONS_PARSER.on("--headless-tests", "Skip tests requiring a GUI") {
    @OPTIONS.headless_tests = true
}

# Extract flags (after `--`)
flags = ARGV.drop(1)
flags_index = ARGV.index('--')
if flags_index != nil
    flags = ARGV[(flags_index + 1)..-1]
end

# Parse flags and handle errors
begin
    @OPTIONS_PARSER.parse!(flags)
rescue OptionParser::InvalidOption, OptionParser::MissingArgument, OptionParser::InvalidArgument => e
    $stdout.puts e
    $stdout.puts @OPTIONS_PARSER.help
    $stderr.puts "Unable to parse flags, see reason message and help above."
    exit 1
end

# Command Line Arguments (end) ----------------------------------------------------------------------------------

# Global Constants ----------------------------------------------------------------------------------------------

PLATFORM        = @OPTIONS.platform
LINUX           = PLATFORM == PLATFORM_LINUX
WINDOWS         = PLATFORM == PLATFORM_WINDOWS
WEB             = PLATFORM == PLATFORM_WEB
DESKTOP         = !WEB
HEADLESS_TESTS  = !!@OPTIONS.headless_tests
VERBOSE         = @OPTIONS.verbose

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
CONFIG                  = @OPTIONS.config
RELEASE                 = CONFIG == CONFIG_RELEASE
DEBUG                   = CONFIG == CONFIG_DEBUG
OPTIMIZED               = CONFIG == CONFIG_OPTIMIZED
BUILD_DIR               = @OPTIONS.build_dir || "build-#{PLATFORM}-#{CONFIG}"
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

if @OPTIONS.verbose
puts "------------------------------------------------------------------------------------------------------"
puts "@OPTIONS: ........... #{@OPTIONS}"
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
end # if @OPTIONS.verbose

# Global Constants (end) ------------------------------------------------------------------------------------------

# Target API ------------------------------------------------------------------------------------------------

Target = Struct.new(
    :assets, # List of patterns like: "<source>" or "<source>:<destination>"
    :c_flags,
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
    :binary, # path to the binary (.exe, .html, etc)
    :build_dir,
    :dist_dir,
    :bin_dir,
    :dep_dir,
    :obj_dir,
    :src_dir,
    :cached_defines_flags,
    :cached_includes_flags,
    :sources_to_compile,
    :objects_to_link,
)

@TARGETS = []

def target(
    name,
    type,
    cxxstd = CXXSTD20
    )

    raise "Wrong value for cxxstd:\n  Allowed: #{ALLOWED_CXXSTDS.join ", "}.\n  Actual: #{cxxstd}" if not ALLOWED_CXXSTDS.include?(cxxstd)
    
    target      = Target.new
    build_dir   = "#{BUILD_DIR}/#{name}"

    target.build_dir                = build_dir
    target.dist_dir                 = "#{build_dir}/dist"
    target.bin_dir                  = "#{build_dir}/bin"
    target.dep_dir                  = "#{build_dir}/dep"
    target.obj_dir                  = "#{build_dir}/obj"
    target.src_dir                  = "#{build_dir}/unity-build-slices"

    target.cached_defines_flags     = ""
    target.cached_includes_flags    = ""
    target.sources_to_compile       = []
    target.objects_to_link          = []

    target.assets                   = FileList[]
    target.c_flags                  = []
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
    target.binary                   = File.absolute_path "#{target.bin_dir}/#{target.name}"

    case PLATFORM
    when PLATFORM_WEB
        target.binary = target.binary.ext("html") # will generate also a *.js, *.wasm, *.wasm.map and *.data
    when PLATFORM_WINDOWS
        target.binary = target.binary.ext("exe")
    end

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


    @TARGETS += [target]

    return target
end

def target_src_to_obj( target, src )   
    "#{target.obj_dir}/#{src.ext(".o")}"
end

def target_src_to_dep( target, src )
    "#{target.dep_dir}/#{src.ext(".d")}"
end

def target_find_alldeps_from_obj( target, obj )
    
    src = target_find_src_from_obj( target, obj )

    raise "Unable to find src from '#{obj}' for '#{target.name}'" if src == nil
     
    deps = [src]

    # get *.d file that contains a list of the dependencies
    dep = target_src_to_dep( target, src )
    if !File.exist?(dep)
        return deps
    end

    content = File.read(dep)
    content = content.split(": ")[1]
    content = content.gsub(/\\$/, '').strip  # Remove line continuations
    deps    = content.split " "

    deps
end

def target_find_src_from_obj( target, obj )

    # remove obj_dir prefix, this will give a path relative rakefile's dir
    relative_stem = obj.sub("#{target.obj_dir}/", "").ext("")

    src = target.sources_to_compile.detect{|src| src.ext("") == relative_stem }

    if src == nil && obj.include?(target.obj_dir)
        puts target
        puts target.sources_to_compile
        puts obj
        raise "Possible error: the stem contains obj_dir for this target, but the file could not be found!"
    end

    src
end

def target_compile_file(target, src)

    log(target, "Compiling #{src} ...")

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

    log(target, "Compiling #{src} DONE")
end

def target_link( target )

    if (target.type != TARGET_TYPE_EXECUTABLE)
        raise "#{target.name}'s type is expected to be: '#{TARGET_TYPE_EXECUTABLE}', actual: #{target.type}"
    end

    log(target, "Linking ...")

    args = [
        target.compiler_flags,
        target.cached_defines_flags,
        "-o #{target.binary}", # Output binary (emcc requires "-o path/to/file" syntax )
        target.objects_to_link,
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

    FileUtils.mkdir_p File.dirname(target.binary)

    system("#{LINKER} #{args.join(" ")}", exception: true)

    log(target, "Linking DONE - (#{target.binary})")
end


def target_initialize(target)

    debug( target, "Initializing ..." )

    FileUtils.mkdir_p target.bin_dir
    FileUtils.mkdir_p target.obj_dir
    FileUtils.mkdir_p target.dist_dir
    FileUtils.mkdir_p target.dep_dir
    FileUtils.mkdir_p target.src_dir

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
    unity_sources = nil
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
            filename   = "#{target.src_dir}/slice-#{1+index}-#{hash}.cpp"

            unity_sources += [filename]

            FileUtils.mkdir_p File.dirname( filename )

            if !File.exist?(filename)
                File.write(filename, content)
            end
        end

        debug(target, "Unity Build - #{unity_sources.size} slice(s) were generated (slice size: #{target.unity_build_slice_size})")
    end

    if unity_sources
        target.sources_to_compile = unity_sources
    else
        target.sources_to_compile = target.sources.dup
    end

    # Generate a cache of all objects the objects to link
    target.objects_to_link |= target.sources_to_compile.map{|src|target_src_to_obj(target, src)}
    target.objects_to_link |= target.depends_on_target.map{|t|t.objects_to_link}.flatten

    target.is_initialized = true

    # puts "sources_to_compile:"
    # puts "  ", target.sources_to_compile.join("\n  ")

    # puts "objects_to_link:"
    # puts "  ", target.objects_to_link.join("\n  ")

    debug(target, "Initialized")
end

# Define all the tasks for a given build target
# DO NOT mutate target after you called this.
def target_define_tasks(target)

    namespace target.name do

    task :init => target.depends_on_target.map{|t|"#{t.name}:init"} do
        return if target.is_initialized
        target_initialize(target)
    end

    # Since we do not have a "configure" step, it is important to make
    # sure the target is initialized each time we run this script.
    Rake::Task[:init].invoke

    # Declare a task per source we have to compile
    # (strictly for this target, this does not include target dependencies's sources, each target dependency declares its own tasks).
    target.sources_to_compile.each do |src|
        obj = target_src_to_obj(target, src)
        file obj => target_find_alldeps_from_obj(target, obj) do
            target_compile_file(target, src)
        end    
    end

    task :clean do
        # FileUtils.rm_rf was too slow, that's why we use:
        sh "rm", "-rf", target.build_dir
    end

    task :clobber do
        # Remove build dir and dist file
        sh "rm", "-rf", target.build_dir, target.binary
    end

    task :rebuild => [:clean, :build]


    target.assets.each_with_index do |file_pattern, i|

        src, dst    = file_pattern_split(file_pattern)
        asset_src   = src
        asset_dest  = "#{target.bin_dir}/#{dst}"

        file asset_src => asset_dest do
            file_copy_or_overwrite( asset_src, asset_dest )
        end

    end  

    case target.type
    when TARGET_TYPE_OBJECTS

        task :build => target.objects_to_link do
            update_llvm_json_compilation_database()
        end

    when TARGET_TYPE_EXECUTABLE

        file target.binary => target.objects_to_link do
            update_llvm_json_compilation_database()
            target_link(target)
        end

        task :link => target.binary 

        task :build => :link do
            log(target, "Build DONE")
        end
        
        task :run => :build do
            log(target, "Running ...")
            case PLATFORM
            when PLATFORM_WEB
                system("#{BINARY_EMRUN} --hostname #{HTTP_SERVER_HOSTNAME} --port #{HTTP_SERVER_PORT} #{target.binary}", exception: true)
            else
                system(target.binary, exception: true)
            end
            log(target, "Running DONE")
        end

        task :zip do

            # Copy binary and related files + assets into dist folder
            if WEB
                binary_and_additionnal_files = FileList[
                    "#{target.bin_dir}/#{File.basename(target.binary).ext(".*")}", # .html, .js, .wasm, etc.
                ];
                binary_and_additionnal_files.each do |each|;
                    file_copy_or_overwrite( each, "#{target.dist_dir}/#{File.basename(each)}" )
                end
            else
                # Copy the binary
                binary_filename = File.basename(target.binary)
                file_copy_or_overwrite( target.binary, "#{target.dist_dir}/#{binary_filename}" )
            end
            target.assets.each_with_index do |pattern, i|
                src, dst = file_pattern_split(pattern)
                file_copy_or_overwrite( src, "#{target.dist_dir}/#{dst}" )
            end

            # Zip all the files
            zip_filename = File.absolute_path "#{ZIPPED_DIST_DIR}/#{target.name}-#{PLATFORM}-#{CONFIG}.zip"
            
            FileUtils.rm zip_filename if File.exist? zip_filename
            
            FileUtils.mkdir_p File.dirname( zip_filename )
            
            level = 9 # zip compression level

            if BUILD_OS == BUILD_OS_WINDOWS
                # GitHub Runner OS has 7zip on windows
                sh "7z a -tzip -mx#{level} #{zip_filename} #{target.dist_dir}/*"
            else
                sh "cd #{target.dist_dir} && zip -r -#{level} #{zip_filename} ."
            end
        end
    end # case target.type
    end # namespace end
end

# Target API (end) ------------------------------------------------------------------------------------------------

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

def log(target, message)
    puts "[#{target.name}:#{PLATFORM}:#{CONFIG}] #{message}"
end

def debug(target, message)
    log(target,message) if VERBOSE
end

def print_help()

    # Print regular option parser help
    print @OPTIONS_PARSER.help    
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

def run_sudo_apt_install(packages)
    system("sudo apt install #{packages.join(" ")}", exception: true)
end

# Define task :clean to clean all targets
# All targets created with target() will be included
def define_clean_task()

    task :clean => [
        *@TARGETS.map{|t|"#{t.name}:clean"}
    ]

end


# Define task :clobber to clean all targets
# All targets created with target() will be included
def define_clobber_task()

    task :clobber do 
        #Using file utils was too slow
        system "rm", "-rf", BUILD_DIR
    end

end