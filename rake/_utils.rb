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
BIN_DIR            = "#{BUILD_DIR}/bin"
LINUX              = OS == "linux"
WINDOWS            = OS == "windows"
GITHUB_ACTIONS     = ENV["GITHUB_ACTIONS"]
HTTP_SERVER_HOSTNAME = "0.0.0.0"
HTTP_SERVER_PORT     = "8000"
HTTP_SERVER_URL      = "http://#{HTTP_SERVER_HOSTNAME}:#{HTTP_SERVER_PORT}/"
VCPKG_TRIPLET        = get_vcpkg_triplet()
VCPKG_INSTALLED      = "./vcpkg/#{OS}/#{VCPKG_TRIPLET}"
PKG_CONFIG_BIN       = WINDOWS ? "#{VCPKG_INSTALLED}/tools/pkgconf/pkgconf.exe" : "pkg-config"
PKG_CONFIG_ARGS      = "--with-path #{VCPKG_INSTALLED}/lib/pkgconfig"
PKG_CONFIG_CMD       = "#{PKG_CONFIG_BIN} #{PKG_CONFIG_ARGS}"

puts "------------------------------------------------------------------------------------------------------"
puts "RUBY version: ....... #{`ruby -v`}"
puts "HOST_OS: ............ #{HOST_OS}"
puts "OS:.................. #{OS}"
puts "ARCHITECTURE: ....... #{ARCHITECTURE}"
puts "TARGET: ............. #{TARGET}"
puts "BUILD_TYPE: ......... #{BUILD_TYPE}"
puts "VCPKG_TRIPLET: ...... #{VCPKG_TRIPLET}"
puts "HTTP_SERVER_HOSTNAME: #{HTTP_SERVER_HOSTNAME}"
puts "HTTP_SERVER_PORT:     #{HTTP_SERVER_PORT}"
puts "Dir.pwd: ............ #{Dir.pwd }"
puts "__FILE__: ........... #{File.dirname(__FILE__)}"
puts "------------------------------------------------------------------------------------------------------"

if DESKTOP
    $c_compiler   = "clang"
    $cxx_compiler = "clang++"
    $linker       = "clang"
elsif WEB
    $c_compiler   = "emcc"
    $cxx_compiler = "emcc"
    $linker       = "emcc"
else
    raise "Unexpected platform!"
end

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
    # :vcpkg, # list of vcpkg package names
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
    target.depends_on_target = []
    # target.vcpkg = []
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

def compile_file(src, target)
    
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
    command = "#{is_cpp ? $cxx_compiler : $c_compiler} #{args.join(" ")}"

    return system(command)
end

def link_binary( target )

    if (target.type != TARGET_TYPE_EXECUTABLE)
        raise "Target type is expected to be: '#{TARGET_TYPE_EXECUTABLE}', actual: #{target.type}"
    end

    # Prepare linker arguments
    args = []
    args += target.compiler_flags
    args += get_defines_flags(target)   
    args += ['-o',  get_binary_path(target)]
    args += get_objects__incl_deps(target)
    args += target.linker_flags

    FileUtils.mkdir_p File.dirname(get_binary_path(target))

    command = "#{$linker} #{args.join(" ")}"

    return system(command)
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
    if VERBOSE 
        puts content
    end
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
                system("./#{get_binary_path(target)}")
            elsif WEB
                system("emrun --hostname #{HTTP_SERVER_HOSTNAME} --port #{HTTP_SERVER_PORT} #{get_binary_path(target)}")
            end
        end
    end

    # Add a task per object to build (dependencies excluded)
    objects.each_with_index do |obj, index|
        src = obj_to_src( obj, target )
        file obj => src do |task|
            puts "#{target.name} | Compiling #{src} ..."
            compile_file( src, target) or raise "Unable to compile #{src}!"
        end
    end
end

def pkg_config(args)

    if WINDOWS and not File.exist?(PKG_CONFIG_BIN)
        print("Unable to find PKG_CONFIG_BIN (#{PKG_CONFIG_BIN})\n")
        return ""
    end

    command = "#{PKG_CONFIG_BIN} #{PKG_CONFIG_ARGS} #{args}"

    # print("Running: #{command}\n")

    result = `#{command}` || ""
    if result
        result = result.chomp("\n") # Make sure string does not contains "\r", "\n", or "\r\n"
    end

    # print(" --result: #{result}\n")

    result
end

task :pkgconf , [:arg] do |task, args|
    print pkg_config(args[:arg])
end
