
CMakeTarget = Struct.new(
    :name,
    :path,
    keyword_init: true # If the optional keyword_init keyword argument is set to true, .new takes keyword arguments instead of normal arguments.
)

def tasks_for_cmake_target( target )
    build_dir   = "#{BUILD_DIR}/#{target.path}"
    install_dir = "#{INSTALL_DIR}/#{target.name}"

    task :rebuild => [:clean, :build]

    task :clean do
        FileUtils.rm_f build_dir
        FileUtils.rm_f install_dir
    end

    task :build do
         if Dir.exist? build_dir
             FileUtils.rm_rf build_dir
         end
         if BUILD_TYPE == "release"
            config = "Release"
         else
            config = "Debug"
         end
         FileUtils.mkdir_p build_dir # ensure folder exists
         sh "cmake -S #{target.path} -B #{build_dir}" # configure
         sh "cmake --build #{build_dir} --config #{config}"
    end

    task :install => :build do
        FileUtils.mkdir_p install_dir
        sh "cmake --install #{build_dir} --prefix #{install_dir}"
    end
end
