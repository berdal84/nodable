require "rbconfig"
require 'json'
require 'date' # To add date in .clang export

def get_architecture()
    host_cpu = RbConfig::CONFIG['host_cpu']

    if host_cpu != "x86_64" and host_cpu != "x64"
        raise "This script is not compatible with #{host_cpu} architecture!"
    end

    return "x64"
end

def get_os()
    
    build_os = RbConfig::CONFIG['build_os']

    if build_os.include?("linux")
        return "linux"
    elsif build_os.include?("mingw32") # Ruby is built on mingw32 (w32 does not stands for 32bits)
        return "windows"
    end

    raise "This script is not compatible with #{build_os}!"
end

def get_vcpkg_triplet()
    triplet = "#{ARCHITECTURE}-#{OS}"

    if OS == "windows"
        triplet += "-static" # windows convention is different than linux, dynamic by default
    end

    triplet
end

def get_target()
    target = (ENV["TARGET"] || "desktop").downcase

    if target != "desktop" and target != "web"
        raise "Unexpected target: #{target}! (expecting web|desktop)"
    end

    return target
end

def get_build_type()
    build_type = (ENV["BUILD_TYPE"] || "release").downcase

    if build_type != "release" and build_type != "debug"
        raise "Unexpected build_type: #{build_type}! (expecting release|debug)"
    end

    return build_type
end

def get_pkgconf_binary()
  if WINDOWS 
    return "#{VCPKG}/tools/pkgconf/pkgconf.exe"
  elsif LINUX
    return "#{VCPKG}/tools/pkgconf/pkgconf"  
  end
end

ARCHITECTURE       = get_architecture()      # Must match with vcpkg convention
OS                 = get_os()                # Must match with vcpkg convention
VERBOSE            = !!ENV["VERBOSE"]
HOST_OS            = RbConfig::CONFIG['host_os']
TARGET             = get_target()
DESKTOP            = TARGET == "desktop"
WEB                = TARGET == "web"
BUILD_TYPE         = get_build_type()
RELEASE            = BUILD_TYPE == "release"
DEBUG              = BUILD_TYPE == "debug"
BUILD_DIR          = ENV["BUILD_DIR"] || "build-#{TARGET}-#{ARCHITECTURE}-#{OS}-#{BUILD_TYPE}"
OBJ_DIR            = "#{BUILD_DIR}/obj"
DEP_DIR            = "#{BUILD_DIR}/dep"
BIN_DIR            = "#{BUILD_DIR}/bin" # TODO: ambigous, we also consider this folder as dist/, FIXME
LINUX              = OS == "linux"
WINDOWS            = OS == "windows"
GITHUB_ACTIONS     = ENV["GITHUB_ACTIONS"]
HTTP_SERVER_HOSTNAME = "0.0.0.0"
HTTP_SERVER_PORT     = "8000"
HTTP_SERVER_URL      = "http://#{HTTP_SERVER_HOSTNAME}:#{HTTP_SERVER_PORT}/"
VCPKG_TRIPLET        = get_vcpkg_triplet()
VCPKG                = "./vcpkg/#{OS}/#{VCPKG_TRIPLET}"
PKGCONF_BINARY       = get_pkgconf_binary()
PKGCONF              = "#{PKGCONF_BINARY} --with-path #{VCPKG}/lib/pkgconfig"
COMPILER             = DESKTOP ? "clang" : "emcc"
LINKER               = DESKTOP ? "clang" : "emcc"

if VERBOSE
puts "------------------------------------------------------------------------------------------------------"
puts "RUBY version: ....... #{`ruby -v`}"
puts "HOST_OS: ............ #{HOST_OS}"
puts "OS:.................. #{OS}"
puts "ARCHITECTURE: ....... #{ARCHITECTURE}"
puts "TARGET: ............. #{TARGET}"
puts "BUILD_TYPE: ......... #{BUILD_TYPE}"
puts "VCPKG: .............. #{VCPKG}"
puts "VCPKG_TRIPLET: ...... #{VCPKG_TRIPLET}"
puts "HTTP_SERVER_HOSTNAME: #{HTTP_SERVER_HOSTNAME}"
puts "HTTP_SERVER_PORT:     #{HTTP_SERVER_PORT}"
puts "PKGCONF_BINARY: ..... #{PKGCONF_BINARY}"
puts "PKGCONF: ............ #{PKGCONF}"
puts "COMPILER: ........... #{COMPILER}"
puts "LINKER: ............. #{LINKER}"
puts "Dir.pwd: ............ #{Dir.pwd }"
puts "__FILE__: ........... #{File.dirname(__FILE__)}"
puts "------------------------------------------------------------------------------------------------------"
end # if VERBOSE

TARGET_TYPE_OBJECTS    = "objects"
TARGET_TYPE_EXECUTABLE = "executable"

#---------------------------------------------------------------------------

Target = Struct.new(
    :name,
    :type, # TARGET_TYPE_XXX
    :sources, # list of .c|.cpp files
    :depends_on_target, # list of other targets to link with (their compiled *.o will be linked)
    :includes, # list of path dir to include
    :defines,
    :compiler_flags,
    :c_flags,
    :cxx_flags,
    :linker_flags,
    :assets, # List of patterns like: "<source>" or "<source>:<destination>"
    :vcpkg, # list of vcpkg package names
    :is_ready_to_compile_and_link, # is ready to compile (e.g. pkg-config was run)
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
    target.is_ready_to_compile_and_link = false;
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

def generate_vcpkg_flags( target )

    puts "#{target.name} | Generate vcpkg flags .."

    target.vcpkg.each do |vcpkg_name|

        # we use |= to make sure there is no duplicates
        target.cxx_flags    |= [get_library_cflags(vcpkg_name)]
        target.linker_flags |= [get_library_linker_flags(vcpkg_name, "static")]
    
    end

    puts "#{target.name} | Generate vcpkg flags DONE"
end

$mutex = Mutex.new

def ensure_is_ready_to_compile_and_link(target)

    # Let's check if ready first (we don't need to sync threads to read)
    if target.is_ready_to_compile_and_link
        return
    end

    # Since multiple task may run this in parrallel, we need to lock this portion
    $mutex.synchronize {

        # Might have changed
        if target.is_ready_to_compile_and_link
            return
        end
        puts "#{target.name} | Prepare .."
    
        generate_vcpkg_flags(target)
        target.is_ready_to_compile_and_link = true

        puts "#{target.name} | Prepare DONE"
    }    
end

def compile_file(src, target)

    ensure_is_ready_to_compile_and_link(target)
    
    # Ensure target folders exist
    FileUtils.mkdir_p File.dirname( src_to_obj( src ) )
    FileUtils.mkdir_p File.dirname( src_to_dep( src ) )

    # Prepare compiler arguments
    is_cpp = File.extname( src ) == ".cpp"
    args = []
    args += target.compiler_flags
    args += is_cpp ? target.cxx_flags : target.c_flags
    args += ['-c'] # no linking
    args += get_includes_flags(target)
    args += get_defines_flags(target)
    args += ["-MD", "-MF#{src_to_dep(src)}"]
    args += ["-MJ", src_to_obj(src).ext("o.json")] # Write a compilation database entry per input, see https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-MJ-arg
    args += ["-o",  src_to_obj(src)]
    args += [src]

    # print(args.join(" "))

    # Run the command
    command = "#{COMPILER} #{args.join(" ")}"

    system(command, exception: true)
end

def link_binary( target )

    if (target.type != TARGET_TYPE_EXECUTABLE)
        raise "Target type is expected to be: '#{TARGET_TYPE_EXECUTABLE}', actual: #{target.type}"
    end

    ensure_is_ready_to_compile_and_link(target)

    # Prepare linker arguments
    args = []
    args += target.compiler_flags
    args += get_defines_flags(target)   
    args += ['-o',  get_binary_path(target)]
    args += get_objects__incl_deps(target)
    args += target.linker_flags

    FileUtils.mkdir_p File.dirname(get_binary_path(target))

    command = "#{LINKER} #{args.join(" ")}"

    system(command, exception: true)
end

def get_defines_flags(target)
    target.defines.map{|d| "-D\"#{d}\"" }
end

def get_includes_flags(target)
    target.includes.map{|f| "--include-directory=#{File.absolute_path(f)}"}
end

def get_assets_src(target)
    assets = []
    target.assets.each do |pattern|
        source, destination = split_asset_pattern(pattern)
        assets.append( source )
    end
    assets
end

def get_assets_dest(target)
    assets = []
    target.assets.each do |pattern|
        source, destination =  split_asset_pattern(pattern)
        assets.append( destination )
    end
    assets
end

def split_asset_pattern(pattern)  # pattern: "<source>:<destination>" (destination is optional)
    arr = pattern.split(':')

    # Source is required
    source = arr[0] or raise ("Wrong pattern: #{pattern}")

    # Destination is optional, by default we copy relative to repository root
    destination = "#{BIN_DIR}/#{arr[1] || source}";

    [source, destination]
end

def copy_asset(source, destination)
    FileUtils.rm_f destination
    FileUtils.mkdir_p File.dirname(destination)
    FileUtils.copy_file( source, destination )
    puts "  Copy asset: #{source} => #{destination}"
end

def update_compile_commands_json()
    # Generate compile_commands.json for clangd
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

    # debug log
    #if VERBOSE 
    #    puts content
    #end
end

def tasks_for_target(target)

    objects = get_self_objects(target)
    objects_with_deps = get_objects__incl_deps(target)

    desc "Clean intermediate files (#{target.name})"
    task :clean do
        FileUtils.rm_f objects
    end

    desc "Clean intermediate files including dependencies (#{target.name})"
    task :clean_all do
        FileUtils.rm_f objects_with_deps
    end

    desc "Clean and build (#{target.name})"
    task :rebuild => [:clean, :build]

    desc "Clean and build all (#{target.name})"
    task :rebuild_all => [:clean_all, :build]

    if target.type == TARGET_TYPE_OBJECTS

        desc "Compile individual objects (#{target.name})"
        multitask :build => objects_with_deps do
            puts "#{target.name} | Build DONE"
        end

    elsif target.type == TARGET_TYPE_EXECUTABLE

        desc "Compile and link binary (#{target.name})"
        task :build => [:link, :copy_assets] do
            puts "#{target.name} | Build DONE"
        end

        # assets
        assets_src = get_assets_src(target)
        assets_dst = get_assets_dest(target)
        multitask :copy_assets => assets_dst do 
            puts "Assets copy DONE"
        end
        target.assets.each_with_index do |_, i|
            file assets_dst[i] => assets_src[i] do
                copy_asset(assets_src[i], assets_dst[i])
            end
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

        desc "Run #{target.name}"
        task :run => :build do

            if DESKTOP
                system("./#{get_binary_path(target)}", exception: true)
            elsif WEB
                system("emrun --hostname #{HTTP_SERVER_HOSTNAME} --port #{HTTP_SERVER_PORT} #{get_binary_path(target)}", exception: true)
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

def get_library_cflags(libname)

    pkg_config_flags = ['--cflags']
    
    # try to get pkg-config flags first
    has_pkg_config = system("#{PKGCONF} --exists #{libname}")

    if has_pkg_config
        flags = `#{PKGCONF} #{pkg_config_flags.join(" ")} #{libname}`.chomp
    else
        flags = "-I#{VCPKG}/include" # try this..
    end

    puts "get_library_cflags(#{libname}) => #{flags}"

    return flags

end

def get_library_linker_flags(libname, type)

    is_static = type == "static"
    
    is_static or raise "type not supported: #{type}"
    
    pkg_config_flags = ['--libs']
    
    if is_static
        pkg_config_flags.append("--static")
    end

    # try to get pkg-config flags first
    has_pkg_config = system("#{PKGCONF} --exists #{libname}")

    if has_pkg_config
        flags = `#{PKGCONF} #{pkg_config_flags.join(" ")} #{libname}`.chomp
    else
        flags = "-l#{libname}" 
    end

    puts "get_library_linker_flags(#{libname}, #{type}) => #{flags}"

    return flags
end
