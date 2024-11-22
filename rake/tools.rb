
namespace :tools do
    $tools_core = new_project("tools-core")
    $tools_core[:sources] |= FileList[
        "src/tools/core/reflection/qword.cpp",
        "src/tools/core/reflection/Type.cpp",
        "src/tools/core/reflection/TypeRegister.cpp",
        "src/tools/core/reflection/variant.cpp",
        #"src/tools/core/memory/pointers.cpp",
        "src/tools/core/EventManager.cpp",
        "src/tools/core/FileSystem.cpp",
        "src/tools/core/format.cpp",
        "src/tools/core/log.cpp",
        "src/tools/core/StateMachine.cpp",
        "src/tools/core/System.cpp",
        "src/tools/core/TaskManager.cpp",
        "libs/whereami/src/whereami.c"
    ]
    $gl3w = new_project("gl3w")
    $gl3w[:sources] |= FileList[
        "libs/gl3w/GL/gl3w.c"
    ]

    $texteditor = new_project("texteditor")
    $texteditor[:sources] |= FileList[
        "libs/ImGuiColorTextEdit/TextEditor.cpp"
    ]

    $lodepng = new_project("lodepng")
    $lodepng[:sources] |= FileList[
        "libs/lodepng/lodepng.cpp"
    ]

    $imgui = new_project("imgui")
    $imgui[:sources] |= FileList[
       "libs/imgui/imgui.cpp",
       "libs/imgui/imgui_demo.cpp",
       "libs/imgui/imgui_draw.cpp",
       "libs/imgui/imgui_tables.cpp",
       "libs/imgui/imgui_widgets.cpp",
       "libs/imgui/misc/freetype/imgui_freetype.cpp",
       "libs/imgui/backends/imgui_impl_sdl.cpp",
       "libs/imgui/backends/imgui_impl_opengl3.cpp",
    ]
    $imgui[:sources] |= Dir.glob("libs/freetype/src/**.{c,cpp}")

    $tools_gui = new_project("tools_gui")
    $tools_gui[:sources] |= FileList[
        "src/tools/gui/geometry/BezierCurveSegment2D.cpp",
        "src/tools/gui/geometry/BoxShape2D.cpp",
        "src/tools/gui/geometry/Rect.cpp",
        "src/tools/gui/geometry/SpatialNode2D.cpp",
        "src/tools/gui/geometry/TRSTransform2D.cpp",
        "src/tools/gui/Action.cpp",
        "src/tools/gui/ActionManager.cpp",
        "src/tools/gui/ActionManagerView.cpp",
        "src/tools/gui/App.cpp",
        "src/tools/gui/AppView.cpp",
        "src/tools/gui/Config.cpp",
        "src/tools/gui/FontManager.cpp",
        "src/tools/gui/ImGuiEx.cpp",
        "src/tools/gui/ViewState.cpp",
        "src/tools/gui/TextureManager.cpp",
    ]

    tools_gui_example = new_project("tools-gui-example")
    tools_gui_example[:sources] |= FileList[
        "src/tools/gui-example/AppExample.cpp",
        "src/tools/gui-example/AppExampleView.cpp",
        "src/tools/gui-example/main.cpp"
    ]

    tools_gui_example[:sources] |= $tools_core[:sources]
    tools_gui_example[:sources] |= $tools_gui[:sources]
    tools_gui_example[:sources] |= $gl3w[:sources]
    tools_gui_example[:sources] |= $lodepng[:sources]
    tools_gui_example[:sources] |= $imgui[:sources]

    tools_gui_example[:includes] |= $tools_core[:includes]
    tools_gui_example[:includes] |= $tools_gui[:includes]

    namespace :core do
        declare_project_tasks( $tools_core )
    end
    namespace :gui do
        declare_project_tasks( $tools_gui )
    end
    namespace :gui_example do
        declare_project_tasks( tools_gui_example )
    end

    desc "Build tools"
    task :build => ['core:build', 'gui:build', 'gui_example:build']
end