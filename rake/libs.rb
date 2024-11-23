require_relative "common"

task :libs => 'libs:build'

namespace :libs do

    task :build => [
        'nfd',
        'cpptrace',
        'sdl',
        'freetype',
        'imgui:build',
        'lodepng:build',
        'texteditor:build',
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

    namespace :gl3w do
        gl3w = new_project("gl3w", "static")
        gl3w[:sources] |= FileList[
            "libs/gl3w/GL/gl3w.c"
        ]
        declare_project_tasks(gl3w)
    end

    namespace :texteditor do
        texteditor = new_project("texteditor", "static")
        texteditor[:sources] |= FileList[
            "libs/ImGuiColorTextEdit/TextEditor.cpp"
        ]
        declare_project_tasks( texteditor )
    end

    namespace :lodepng do
        lodepng = new_project("lodepng", "static")
        lodepng[:sources] |= FileList[
            "libs/lodepng/lodepng.cpp"
        ]
        declare_project_tasks( lodepng )    
    end
    
    namespace :imgui do
        imgui = new_project("imgui", "static")
        imgui[:sources] |= FileList[
           "libs/imgui/imgui.cpp",
           "libs/imgui/imgui_demo.cpp",
           "libs/imgui/imgui_draw.cpp",
           "libs/imgui/imgui_tables.cpp",
           "libs/imgui/imgui_widgets.cpp",
           "libs/imgui/misc/freetype/imgui_freetype.cpp",
           "libs/imgui/backends/imgui_impl_sdl.cpp",
           "libs/imgui/backends/imgui_impl_opengl3.cpp",
        ]
        declare_project_tasks( imgui )
    end

end # namespace libs
