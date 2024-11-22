
namespace :libs do

    task :build_all => [
        'nfd',
        'cpptrace',
        'sdl',
        'freetype'
    ]

    task :nfd => [] do
        commands = [
            "rm -rf libs/nativefiledialog-extended/build",
            'cd libs/nativefiledialog-extended',
            'mkdir -p build',
            'cd build',
            'cmake -DCMAKE_BUILD_TYPE=Release ..',
            'cmake --build .'
        ]
        system commands.join(" && ")
    end

    task :cpptrace => [] do
        commands = [
            "cd libs/cpptrace",
            "mkdir -p build && cd build",
            "cmake .. -DCMAKE_BUILD_TYPE=Release",
            "make -j",
            "sudo make install"
        ]
        system commands.join(" && ")
    end

    task :sdl => [] do
        commands = [
            'cd libs/SDL',
            'mkdir -p build',
            'cd build',
            '../configure',
            'make',
            'sudo make install'
        ]
        system commands.join(" && ")
    end

    task :freetype => [] do
        commands = [
            'cd libs/freetype',
            'mkdir -p build && cd build',
            'cmake ..',
            'make',
            'sudo make install'
        ]
        system commands.join(" && ")
    end
end # namespace libs
