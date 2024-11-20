require_relative "rakefile.config"

def new_project(_name)
	{
	   name:          _name,
       sources:       FileList[],
       objects:       FileList[],
       includes:      FileList[
           "src",
           "src/gui",
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
           "libs/nativefiledialog-extended/src/include"
       ],
       defines: {
           "IMGUI_USER_CONFIG": "\"tools/gui/ImGuiExConfig.h\"" # extend ImVec2 and ImVec4 constructors & more..
       },
       c_compiler:   "clang",
       cxx_compiler: "clang++"
    }
end

def configure_project(project)

    check_os_support()

    # Generate object list
    project[:objects] = project[:sources].map{|src| src_to_obj(src)}

    # each obj depends on its src tasks
    project[:objects].each_with_index do |obj, index|
        src = project[:sources][index]
        file obj => src do
            compile_file( src, obj, project)
        end
    end

    project[:objects]
end

def src_to_obj(src_file)
	"#{OBJ_DIR}/#{src_file.ext("o")}"
end

def compile_file(src_file, obj_file, project)
	puts "Compiling #{src_file} ..."
	FileUtils.mkdir_p File.dirname(obj_file)

    includes = project[:includes].map{|path| "-I#{path}"}.join(" ")
    defines  = project[:defines].map{|key,value| "-D#{key}='#{value}'" }.join(" ") # we add '' because "" are removed by the terminal

    if ( project[:c_compiler] != "clang" )
        raise "Not implemented"
    end
    if ( project[:cxx_compiler] != "clang++" )
        raise "Not implemented"
    end

    flags = [
        "-c", # obj only
        "-Wfatal-errors",
        #"-pedantic"
    ].join(" ")

    flags_cxx    = [
        "--std=c++20",
        "-fno-char8_t"
    ].join(" ")

    has_cxx_extension = [
        ".cpp",
        ".cc"
    ].include? File.extname( src_file )

    if has_cxx_extension
        system "#{project[:cxx_compiler]} #{flags} #{flags_cxx} #{defines} #{includes} -o #{obj_file} #{src_file}" or raise("Unable to compile file!")
    else
        system "#{project[:c_compiler]} #{flags} #{defines} #{includes} -o #{obj_file} #{src_file}" or raise("Unable to compile file!")
    end
end

def is_linux
    TARGET_OS == TARGET_OS_LINUX_GNU
end

def is_windows
    TARGET_OS == TARGET_OS_MINGW32
end

def is_mac
    TARGET_OS == TARGET_OS_DARWIN22
end

def check_os_support()
    supported_target_os = [
        TARGET_OS_DARWIN22,
        TARGET_OS_MINGW32,
        TARGET_OS_LINUX_GNU,
    ]
    if !supported_target_os.include?( TARGET_OS )
        raise "OS Compatibility: #{TARGET_OS} is not compatible, we do: #{supported_target_os.join(", ")}."
    end
end