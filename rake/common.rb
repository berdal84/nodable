
C_COMPILER   = "clang"
CXX_COMPILER = "clang++"

def new_project(name)

    sources  = FileList[]
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
        "--std=c++20",
        "-fno-char8_t"
    ]
    linker_flags = [
        "-Llibs/nativefiledialog-extended/build/src",
        "`pkg-config --cflags --libs gtk+-3.0`",
        "`pkg-config --cflags --libs freetype2`",
        "`sdl2-config --cflags --libs`",
        "-l:libnfd.a",
        "-lGL",
        "-lcpptrace -ldwarf -lz -lzstd -ldl", # https://github.com/jeremy-rifkin/cpptrace?tab=readme-ov-file#use-without-cmake
    ]
    asset_folder_path = "assets" # a single folder

    # OS specific
    if BUILD_OS_MACOS
        flags |= [
            "-framework CoreFoundation",
            "-framework Cacoa"
        ] 
    end

    defines = []
    if BUILD_OS_WINDOWS
        defines |= [
            "IMGUI_USER_CONFIG=\"<tools/gui/ImGuiExConfig.h>\""
        ]
    else
        defines |= [
            "IMGUI_USER_CONFIG=\\\"tools/gui/ImGuiExConfig.h\\\"",
            "NDBL_APP_ASSETS_DIR=\\\"#{asset_folder_path}\\\"",
            "NDBL_APP_NAME=\\\"#{name}\\\"",
            "NDBL_BUILD_REF=\\\"local\\\"",
        ]
    end
    
    if BUILD_TYPE_RELEASE
        c_flags |= [
            "-O3"
        ] 
    elsif BUILD_TYPE_DEBUG
        c_flags |= [
            "-g", # generates symbols
            "-O0", # no optim
            "-Wfatal-errors",
            "-pedantic"
        ]
    end

    # project
    {
        name:     name,
        sources:  sources,
        # objects: objects, they are generated, see get_objects()
        includes: includes,
        defines: defines,
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
    "#{DEPS_DIR}/#{src.ext(".d")}"
end

def obj_to_src( obj, _project)
    stem = obj.sub("#{OBJ_DIR}/", "").ext("")
    _project[:sources].detect{|src| src.ext("") == stem } or raise "unable to find #{obj}'s source (stem: #{stem})"
end

def get_objects( project )
    if not project[:objects]
        project[:objects] = project[:sources].map{|src| src_to_obj(src) };
    end
    project[:objects]
end

def create_dirs( project ) 
    system "mkdir -p #{BUILD_DIR}"
    system "mkdir -p #{OBJ_DIR}"
end

def declare_project_tasks(project)

    desc "Copy in #{INSTALL_DIR} the files to distribute the software"
    task :pack do
        copy_build_to_install_dir(project)
    end

    desc "Compile project"
    task :build => :link

    desc "Link objects"
    task :link => [:compile, 'libs:build_all'] do
        build_executable_binary( project )
        copy_assets_to_build_dir( project )
    end    

    objects = get_objects( project )

    multitask :compile => objects

    objects.each do |obj|
        src = obj_to_src( obj, project )
        file obj => src do |task|
            compile_file( src, project)
		end
	end
end

def copy_assets_to_build_dir( project )
    source      = "#{project[:asset_folder_path]}/**"
    destination = "#{BUILD_DIR}/#{project[:asset_folder_path]}"
    puts "Copying assets from #{source} to #{destination}"
    commands = [
        "mkdir -p #{destination}",
        "cp -r #{source} #{destination}",
    ].join(" && ")
    system commands
    puts "Copying assets DONE"
end

def copy_build_to_install_dir( project )
    commands = [
        "mkdir -p #{INSTALL_DIR}",
        "cp -r #{BUILD_DIR}/#{project[:asset_folder_path]} #{BUILD_DIR}/#{project[:name]} #{INSTALL_DIR}", 
    ].join(" && ")
    system commands
end

def build_executable_binary( project )

    objects        = get_objects( project ).join(" ")
    binary         = "#{BUILD_DIR}/#{project[:name]}"
    linker_flags   = project[:linker_flags].join(" ")

    sh "#{CXX_COMPILER} -o #{binary} #{objects} #{linker_flags} -v", verbose: VERBOSE
end

def compile_file(src, project)

    # Generate stringified version of the flags
    # TODO: store this in a cache?
    includes     = project[:includes].map{|path| "-I#{path}"}.join(" ")
    cxx_flags    = project[:cxx_flags].join(" ")
    c_flags      = project[:c_flags].join(" ")
    defines      = project[:defines].map{|d| "-D\"#{d}\"" }.join(" ")
    linker_flags = project[:linker_flags].join(" ")

    if File.extname( src ) == ".cpp"
        # TODO: add a regular flags for both, and remove this c_flags below
       compiler = "#{CXX_COMPILER} #{cxx_flags}"
    else
       compiler = "#{C_COMPILER} #{c_flags}"
    end

    obj = src_to_obj( src )
    dep = src_to_dep( src )
    dep_flags = "-MD -MF#{dep}"

    
    puts "Compiling #{src} ..."
    if VERBOSE
        puts "-- obj: #{obj}"
        puts "-- dep: #{dep}"
    end

    FileUtils.mkdir_p File.dirname(obj)
    FileUtils.mkdir_p File.dirname(dep)

    sh "#{compiler} -c #{includes} #{defines} #{dep_flags} -o #{obj} #{src}", verbose: VERBOSE
end
