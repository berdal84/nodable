require_relative "common"
#---------------------------------------------------------------------------
$whereami = new_project("whereami", "objects")
$whereami.sources |= FileList[
    "libs/whereami/src/whereami.c"
]
#---------------------------------------------------------------------------
$gl3w = new_project("gl3w", "objects")
$gl3w.sources |= FileList[
    "libs/gl3w/GL/gl3w.c"
]
#---------------------------------------------------------------------------
$lodepng = new_project("lodepng", "objects")
$lodepng.sources |= FileList[
    "libs/lodepng/lodepng.cpp"
]
#---------------------------------------------------------------------------
$imgui = new_project("imgui", "objects")
$imgui.sources |= FileList[
   "libs/imgui/imgui.cpp",
   "libs/imgui/imgui_demo.cpp",
   "libs/imgui/imgui_draw.cpp",
   "libs/imgui/imgui_tables.cpp",
   "libs/imgui/imgui_widgets.cpp",
   "libs/imgui/misc/freetype/imgui_freetype.cpp",
   "libs/imgui/backends/imgui_impl_sdl.cpp",
   "libs/imgui/backends/imgui_impl_opengl3.cpp",
]
#---------------------------------------------------------------------------
$text_editor = new_project("text_editor", "objects")
$text_editor.sources |= FileList[
    "libs/ImGuiColorTextEdit/TextEditor.cpp"
]
#---------------------------------------------------------------------------
task :libs => 'libs:build'
namespace :libs do

    namespace :gl3w do
        declare_project_tasks( $gl3w )
    end

    namespace :text_editor do
        declare_project_tasks( $text_editor )
    end

    namespace :lodepng do
        declare_project_tasks( $lodepng )    
    end
    
    namespace :imgui do
        declare_project_tasks( $imgui )
    end

    namespace :whereami do
        declare_project_tasks( $whereami )
    end

    #---------------------------------------------------------------------------
    task :build => [
        'nfd',
        'cpptrace',
        'sdl',
        'freetype',
        'gl3w:build',
        'imgui:build',
        'lodepng:build',
        'text_editor:build',
    ]
    #---------------------------------------------------------------------------
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
    #---------------------------------------------------------------------------
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
    #---------------------------------------------------------------------------
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
    #---------------------------------------------------------------------------
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
    #--------------------------------------------------------------------------
end # namespace libs
