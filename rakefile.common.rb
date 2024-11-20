require_relative "rakefile.config"

def default_project(_name)
	{
	   name:         _name,
       source_files: FileList[]
    }
end

def object_files(_project)
	_project[:source_files].clone.map{|_s| "#{BUILD_DIR}/#{_s.ext("o")}"}
end


def src_to_obj(_s, _project)
	_s.ext("cpp").prepend("#{BUILD_DIR}/")
end

def obj_to_src(_s, _project)
	stem = _s.sub("#{BUILD_DIR}/", "")
	_project[:source_files].detect{|_f| _f.ext("") == stem.ext("")}
end

def object_dependencies(_object_path, _project)
	src_file = obj_to_src(_object_path, _project)
	src_file
end

def compile(_target, _project)
	src_file = obj_to_src(_target, _project)
	system "echo Compiling #{src_file}..."
	case TARGET_OS
        when "linux-gnu"
	        system "gcc -c -o #{_target} #{src_file}"
        else
            raise "Unhandled OS: #{TARGET_OS}"
    end
end