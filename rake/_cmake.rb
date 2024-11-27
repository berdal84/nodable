
CMakeTarget = Struct.new(
    :name,
    :path,
    keyword_init: true # If the optional keyword_init keyword argument is set to true, .new takes keyword arguments instead of normal arguments.
)


def get_cmake_install_flags( target )
    # We try a single folder c: on windows
    if BUILD_OS_MINGW
        "--prefix #{CMAKE_INSTALL_PREFIX_MINGW}"
    else
        ""
    end
end

def tasks_for_cmake_target( target )
    build_dir = "#{BUILD_DIR}/#{target.name}"

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
         sh "cmake -S #{target.path} -B #{build_dir} -DCMAKE_OSX_DEPLOYMENT_TARGET=#{MACOSX_VERSION_MIN}" # configure
         sh "cmake --build #{build_dir} --config #{config}"
    end

    task :install => :build do
         cmd = "cmake --install #{build_dir} #{get_cmake_install_flags(target)}"
         if BUILD_OS_LINUX or BUILD_OS_MACOS
             sh "sudo #{cmd}" 
        else
             sh "#{cmd}"
         end
    end
end
