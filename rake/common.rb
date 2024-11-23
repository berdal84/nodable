require_relative 'environment'

def new_project(name, type)

    sources  = FileList[]
    depends_on = FileList[]
    includes = FileList[
        "src",
        "src/ndbl",
        "src/tools",
        "libs",
        "libs/whereami/src",
        "libs/imgui",
        "libs/imgui",
        "libs/glm",
        "libs/gl3w/GL",
        "libs/gl3w",
        "libs/SDL/include",
        "libs/IconFontCppHeaders",
        "libs/nativefiledialog-extended/src/include",
        "libs/cpptrace",
        "libs/freetype/include",
        "/usr/include/X11/mesa/GL"
    ]

    c_flags  = [
    ]
    cxx_flags = [
        # "-stdlib=platform", # ‘libc++’ (with extensions), ‘libstdc++’ (standard), or ‘platform’ (default).
        "--std=c++20",
        "-fno-char8_t"
    ]
    linker_flags = [
        "-L#{LIB_DIR}",
        "-Llibs/nativefiledialog-extended/build/src",        
        "`pkg-config --cflags --libs freetype2`",
        "`sdl2-config --cflags --libs`",
        "-lnfd", # Native File Dialog
        "-lGL",
        "-lcpptrace -ldwarf -lz -lzstd -ldl", # https://github.com/jeremy-rifkin/cpptrace?tab=readme-ov-file#use-without-cmake
    ]
    if BUILD_OS_LINUX
        linker_flags |= [
            "`pkg-config --cflags --libs gtk+-3.0`",
        ]
    end

    asset_folder_path = "assets" # a single folder

    # OS specific
    if BUILD_OS_MACOS
        linker_flags |= [
            "-framework CoreFoundation",
            "-framework Cocoa"
        ] 
    end


    defines = [
        "IMGUI_USER_CONFIG=\\\"tools/gui/ImGuiExConfig.h\\\"",
        "NDBL_APP_ASSETS_DIR=\\\"#{asset_folder_path}\\\"",
        "NDBL_APP_NAME=\\\"#{name}\\\"",
        "NDBL_BUILD_REF=\\\"local\\\"",
    ]
    
    if BUILD_TYPE_RELEASE
        compiler_flags = [
            "-O3"
        ] 
    elsif BUILD_TYPE_DEBUG
        compiler_flags = [
            "-g", # generates symbols
            "-O0", # no optim
            "-Wfatal-errors",
            "-pedantic"
        ]
    end

    # project
    {
        name:     name,
        type:     type,
        sources:  sources,
        depends_on: depends_on,
        # objects: objects, they are generated, see get_objects()
        includes: includes,
        defines: defines,
        compiler_flags: compiler_flags,
        c_flags: c_flags,
        cxx_flags: cxx_flags,
        linker_flags: linker_flags,
        asset_folder_path: asset_folder_path # a single folder
    }
end

def src_to_obj( obj )
    "#{OBJ_DIR}/#{ obj.ext(".o")}"
end

def src_to_dep( src )
    "#{DEP_DIR}/#{src.ext(".d")}"
end

def obj_to_src( obj, _project)
    stem = obj.sub("#{OBJ_DIR}/", "").ext("")
    _project[:sources].detect{|src| src.ext("") == stem } or raise "unable to find #{obj}'s source (stem: #{stem})"
end

def to_objects( sources )
    sources.map{|src| src_to_obj(src) };
end

def get_objects( project )
    if not project[:objects]
        project[:objects] = to_objects( project[:sources] )
    end
    project[:objects]
end

def get_all_objects( project )
    objects = get_objects( project )
    project[:depends_on].each do |other_project|
        objects |= get_all_objects( other_project )
    end    
    objects
end

def declare_project_tasks(project)

    desc "Clean intermediate *.o files"
    task :clean do
        get_objects(project).delete()
    end

    desc "Copy in #{INSTALL_DIR} the files to distribute the software"
    task :pack do
        _pack(project)
    end

    desc "Compile #{project[:type]}"
    task :build =>  :binary do
        puts "#{project[:name]} Copying assets ..."
        copy_assets_to_build_dir( project )
        puts "#{project[:name]} Copying assets OK"
    end

    desc "Build executable binary"
    task :binary => :compile do
        case project[:type] 
        when "executable"
            puts "#{project[:name]} Linking ..."
            build_executable_binary( project )
            puts "#{project[:name]} Linking OK"

        when "static"
            puts "#{project[:name]} Creating static library ..."
            build_static_library( project )
            puts "#{project[:name]} Static library OK"
            
        when "objects"
            puts "#{project[:name]} Objects OK"
        else
            raise "Unexpected project type: #{project[:type]}"
        end
    end    

    multitask :compile => get_all_objects( project )

    objects = get_objects( project )
    objects.each_with_index do |obj, index|
        src = obj_to_src( obj, project )
        
        # desc "#{project[:name]}: to build #{src}"
        file obj => src do |task|
            puts "#{project[:name]} | Compiling #{src} ..."
            compile_file( src, project)
		end
	end
end

def copy_assets_to_build_dir( project )
    source      = "#{project[:asset_folder_path]}/**"
    destination = "#{BUILD_DIR}/#{project[:asset_folder_path]}"
    puts "source: #{source}, destination: #{destination}"
    commands = [
        "mkdir -p #{destination}",
        "cp -r #{source} #{destination}",
    ].join(" && ")
    system commands or raise "Unable to copy assets"
end

def _pack( project )
    commands = [
        "mkdir -p #{INSTALL_DIR}",
        "cp -r #{BUILD_DIR}/#{project[:asset_folder_path]} #{BUILD_DIR}/#{project[:name]} #{INSTALL_DIR}", 
    ].join(" && ")
    system commands
end

def get_library_name( project )
    "#{LIB_DIR}/lib#{project[:name].ext(".a")}"
end

def build_static_library( project )

    objects        = get_all_objects( project ).join(" ")
    binary         = get_library_name( project )
    linker_flags   = project[:linker_flags].join(" ")

    FileUtils.mkdir_p File.dirname(binary)
    sh "llvm-ar r #{binary} #{objects}", verbose: VERBOSE
end

def get_binary_name( project )
    "#{BUILD_DIR}/#{project[:name]}"
end

def build_executable_binary( project )

    objects        = get_all_objects( project ).join(" ")
    binary         = get_binary_name( project )
    linker_flags   = project[:linker_flags].join(" ")

    FileUtils.mkdir_p File.dirname(binary)

    sh "#{CXX_COMPILER} -o #{binary} #{objects} #{linker_flags} -v", verbose: VERBOSE
end

def compile_file(src, project)

    # Generate stringified version of the flags
    # TODO: store this in a cache?
    includes     = project[:includes].map{|path| "-I#{path}"}.join(" ")
    cxx_flags    = project[:cxx_flags].join(" ")
    c_flags      = project[:c_flags].join(" ")
    defines      = project[:defines].map{|d| "-D\"#{d}\"" }.join(" ")
    linker_flags = project[:linker_flags].join(" -l")
    compiler_flags = project[:compiler_flags].join(" ")

    if File.extname( src ) == ".cpp"
        # TODO: add a regular flags for both, and remove this c_flags below
       compiler = "#{CXX_COMPILER} #{cxx_flags} #{compiler_flags}"
    else
       compiler = "#{C_COMPILER} #{c_flags} #{compiler_flags}"
    end

    obj = src_to_obj( src )
    dep = src_to_dep( src )
    dep_flags = "-MD -MF#{dep}"

    
    if VERBOSE
        puts "-- obj: #{obj}"
        puts "-- dep: #{dep}"
    end

    FileUtils.mkdir_p File.dirname(obj)
    FileUtils.mkdir_p File.dirname(dep)

    sh "#{compiler} -c #{includes} #{defines} #{dep_flags} -o #{obj} #{src}", verbose: VERBOSE
end
