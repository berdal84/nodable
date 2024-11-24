require_relative "base"

#---------------------------------------------------------------------------
$whereami = new_target_from_base("whereami", TargetType::OBJECTS)
$whereami.sources |= FileList[
    "libs/whereami/src/whereami.c"
]
#---------------------------------------------------------------------------
$gl3w = new_target_from_base("gl3w", TargetType::OBJECTS)
$gl3w.sources |= FileList[
    "libs/gl3w/GL/gl3w.c"
]
#---------------------------------------------------------------------------
$lodepng = new_target_from_base("lodepng", TargetType::OBJECTS)
$lodepng.sources |= FileList[
    "libs/lodepng/lodepng.cpp"
]
#---------------------------------------------------------------------------
$imgui = new_target_from_base("imgui", TargetType::OBJECTS)
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
$text_editor = new_target_from_base("text_editor", TargetType::OBJECTS)
$text_editor.sources |= FileList[
    "libs/ImGuiColorTextEdit/TextEditor.cpp"
]
#---------------------------------------------------------------------------
task :libs => 'libs:build'
namespace :libs do

    namespace :gl3w do
        tasks_for_target( $gl3w )
    end

    namespace :text_editor do
        tasks_for_target( $text_editor )
    end

    namespace :lodepng do
        tasks_for_target( $lodepng )    
    end
    
    namespace :imgui do
        tasks_for_target( $imgui )
    end

    namespace :whereami do
        tasks_for_target( $whereami )
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
        'googletest',
    ]
    #---------------------------------------------------------------------------
    task :nfd => [] do
        commands = [
            "rm -rf libs/nativefiledialog-extended/build",
            'cd libs/nativefiledialog-extended',
            'mkdir -p build',
            'cd build',
            'cmake .. -DCMAKE_BUILD_TYPE=Release',
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
    #---------------------------------------------------------------------------
    task :googletest => [] do
        commands = [
            'cd libs/googletest',
            'mkdir -p build && cd build',
            'cmake ..',
            'make',
            'sudo make install',
        ]
        system commands.join(" && ")
    end
    #--------------------------------------------------------------------------
end # namespace libs
