require_relative 'base'

# declare here the external libraries we need to build as OBJECTS

#---------------------------------------------------------------------------

$text_editor = new_target_from_base("text_editor", TARGET_TYPE_OBJECTS)
$text_editor.sources |= FileList[
    "extern/ImGuiColorTextEdit/TextEditor.cpp"
]

$imgui = new_target_from_base("imgui", TARGET_TYPE_OBJECTS)
$imgui.sources |= FileList[
   "extern/imgui/imgui.cpp",
   "extern/imgui/imgui_demo.cpp",
   "extern/imgui/imgui_draw.cpp",
   "extern/imgui/imgui_tables.cpp",
   "extern/imgui/imgui_widgets.cpp",
   "extern/imgui/misc/freetype/imgui_freetype.cpp",
   "extern/imgui/backends/imgui_impl_sdl2.cpp",
   "extern/imgui/backends/imgui_impl_opengl3.cpp",
]

$whereami = new_target_from_base("whereami", TARGET_TYPE_OBJECTS)
$whereami.sources |= FileList[
    "extern/whereami/src/whereami.c"
]

#---------------------------------------------------------------------------
namespace :libs do

    if PLATFORM_DESKTOP
        task :build => [
            'whereami:build'
        ]
    end

    task :build => [
        'text_editor:build',
        'imgui:build'
    ]

    namespace :text_editor do
        tasks_for_target( $text_editor )
    end

    namespace :imgui do
        tasks_for_target( $imgui )
    end

    namespace :whereami do
        tasks_for_target( $whereami )
    end
end