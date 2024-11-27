
CMakeTarget = Struct.new(
    :name,
    :path,
    keyword_init: true # If the optional keyword_init keyword argument is set to true, .new takes keyword arguments instead of normal arguments.
)

def tasks_for_cmake_target( target )
    build_dir   = "#{BUILD_DIR}/#{target.name}"

    task :rebuild => [:clean, :build]

    task :clean do
        FileUtils.rm_f build_dir
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
         directory build_dir # ensure folder exists
         sh "cmake -S #{target.path} -B #{build_dir}" # configure
         # TODO: we should precise which target to install
         sh "cmake --build #{build_dir} --config #{config}"
    end

    # not needed, we link from BUILD_DIR
    #
    # task :install => :build do
    #     # TODO: we should precise which target to install
    #     cmd = "cmake --install #{build_dir}"
    #     if BUILD_OS_LINUX or BUILD_OS_MACOS
    #         sh "sudo #{cmd}" 
    #     else
    #         sh cmd
    #     end
    # end
end
