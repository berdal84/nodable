require_relative "rakefile.config"

def new_project(_name)
	{
	   name:          _name,
	   includes:      FileList[],
       sources:       FileList[],
       objects:       FileList[],
    }
end

def declare_project_tasks(project)

    # Generate object list
    project[:objects] = project[:sources].map{|src| src_to_obj(src)}

    # each obj depends on its src tasks
    project[:objects].each_with_index do |obj, index|
        src = project[:sources][index]
        file obj => src do
            compile_file( src, obj, project)
        end
    end

    # main task
    desc "Build project #{project[:name]}"
    multitask :build => project[:objects]
end

def src_to_obj(src_file)
	"#{OBJ_DIR}/#{src_file.ext("o")}"
end

def compile_file(src_file, obj_file, project)
	puts "Compiling #{src_file} ..."
	FileUtils.mkdir_p File.dirname(obj_file)
	case TARGET_OS
        when "linux-gnu"
            includes = project[:includes].join(" -I")
	        sh "gcc -I#{includes} -c -o #{obj_file} #{src_file}"
        else
            raise "Unhandled OS: #{TARGET_OS}"
    end
end