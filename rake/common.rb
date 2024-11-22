
C_COMPILER   = "clang"
CXX_COMPILER = "clang++"

def new_project(name)

    sources  = FileList[]
    objects  = FileList[]
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
        "-c", # obj only
        "-Wfatal-errors",
        "-pedantic"
    ]
    cxx_flags = [
        "--std=c++17",
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

    defines = {
        "NDBL_APP_ASSETS_DIR": "\\\"#{asset_folder_path}\\\"",
        "NDBL_APP_NAME": "\\\"nodable\\\"",
        "NDBL_BUILD_REF": "\\\"local\\\"",
    }
    if BUILD_OS_WINDOWS
        defines.merge!({"IMGUI_USER_CONFIG": "\"<tools/gui/ImGuiExConfig.h>\""})
    else
        defines.merge!({"IMGUI_USER_CONFIG": "'<tools/gui/ImGuiExConfig.h>'"})
    end
    
    if BUILD_TYPE_RELEASE
        c_flags |= ["-O3"] 
    elsif BUILD_TYPE_DEBUG
        c_flags |= ["-g", "-O0"]
    end

    # project
    {
        name:     name,
        sources:  sources,
        objects: objects,
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

def obj_to_src( obj, _project)
    stem = obj.sub("#{OBJ_DIR}/", "").ext("")
    _project[:sources].detect{|src| src.ext("") == stem } or raise "unable to find #{obj}'s source (stem: #{stem})"
end

def cook_project( project )
    objects = project[:sources].map{|src| src_to_obj(src) };

    # stringify arrays
    project[:str_includes]     = project[:includes].map{|path| "-I#{path}"}.join(" ")
    project[:str_cxx_flags]    = project[:cxx_flags].join(" ")
    project[:str_c_flags]      = project[:c_flags].join(" ")
    project[:str_defines]      = project[:defines].map{|key,value| "-D#{key}=#{value}" }.join(" ")
    project[:str_linker_flags] = project[:linker_flags].join(" ")

    objects
end

def declare_project_tasks(project)

    system "mkdir -p #{BUILD_DIR}"
    system "mkdir -p #{OBJ_DIR}"

    objects = cook_project(project)

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

    multitask :compile => objects

    objects.each do |obj|
        src = obj_to_src( obj, project )
        # desc "Compiles #{src}"
        file obj => src do |task|
            compile_file( task.source, task.name, project)
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

    objects        = project[:objects].join(" ")
    binary         = "#{BUILD_DIR}/#{project[:name]}"
    linker_flags   = project[:str_linker_flags]

    system "#{CXX_COMPILER} -o #{binary} #{objects} #{linker_flags} -v"
end

def compile_file(src, obj, project)

	puts "Compiling #{src} ..."
	FileUtils.mkdir_p File.dirname(obj)

    includes  = project[:str_includes]
    cxx_flags = project[:str_cxx_flags]
    c_flags   = project[:str_c_flags]
    defines   = project[:str_defines]

    if File.extname( src ) == ".cpp"
       cmd = "#{CXX_COMPILER} #{c_flags} #{cxx_flags} #{defines} #{includes} -o #{obj} #{src}"
    else
       cmd = "#{C_COMPILER} #{c_flags} #{defines} #{includes} -o #{obj} #{src}"
    end

    system cmd or raise "#{cmd} FAILED!"
end
