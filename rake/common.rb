require_relative "config"

def default_project()
	{
       source_files: FileList[]
    }
end

def object_files(_project)
	_project[:source_files].clone.map{|_s| "#{TARGET_BUILD_DIR}/#{_s.ext("o")}"}
end

def compile(_target, _project)
	src_file = obj_to_src(_target, _project)
	sh "echo Compiling #{src_file}...", verbose: false
	if HOST_OS == "linux-gnu"
	    sh "gcc -c -o #{_target} #{src_file}", verbose: false
    else
        raise "Unhandled HOST_OS (#{HOST_OS})"
    end
end

def src_to_obj(_s, _project)
	_s.ext("cpp").prepend("#{TARGET_BUILD_DIR}/")
end

def obj_to_src(_s, _project)
	stem = _s.sub("#{TARGET_BUILD_DIR}/", "")
	_project[:source_files].detect{|_f| _f.ext("") == stem.ext("")}
end

def object_dependencies(_object_path, _project)
	src_file = obj_to_src(_object_path, _project)
	src_file
end