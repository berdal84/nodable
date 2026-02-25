require_relative 'base'

#---------------------------------------------------------------------------
$imgui_sources = FileList[
   "extern/imgui/imgui.cpp",
   "extern/imgui/imgui_demo.cpp",
   "extern/imgui/imgui_draw.cpp",
   "extern/imgui/imgui_tables.cpp",
   "extern/imgui/imgui_widgets.cpp",
   "extern/imgui/misc/freetype/imgui_freetype.cpp",
   "extern/imgui/backends/imgui_impl_sdl2.cpp",
   "extern/imgui/backends/imgui_impl_opengl3.cpp",
   # not from imgui, but related to
   "extern/ImGuiColorTextEdit/TextEditor.cpp"
]
#---------------------------------------------------------------------------
$whereami_sources = FileList[
    "extern/whereami/src/whereami.c"
]
#---------------------------------------------------------------------------
$tools_core = new_target_from_base("tools_core", TARGET_TYPE_OBJECTS)

if DESKTOP
    $tools_core.sources |= $whereami_sources
end

$tools_core.sources |= FileList[
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
    "src/tools/core/TaskManager.cpp"
]

#---------------------------------------------------------------------------
$tools_gui = new_target_from_base("tools_gui", TARGET_TYPE_OBJECTS)

$tools_gui.sources |= $imgui_sources;

$tools_gui.sources |= FileList[
    "src/tools/gui/geometry/BezierCurveSegment2D.cpp",
    "src/tools/gui/geometry/BoxShape2D.cpp",
    "src/tools/gui/geometry/Rect.cpp",
    "src/tools/gui/geometry/SpatialNode.cpp",
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

#---------------------------------------------------------------------------

app = new_target_from_base("tools-gui-example", TARGET_TYPE_EXECUTABLE)
app.sources |= FileList[
    "src/tools/gui-example/AppExample.cpp",
    "src/tools/gui-example/AppExampleView.cpp",
    "src/tools/gui-example/main.cpp"
]
app.assets = FileList[
    # Fonts
    "fonts/CenturyGothic.ttf",
    "fonts/fa-solid-900.ttf",
    "fonts/JetBrainsMono-*.ttf", # 4 variants
]
app.depends_on_target |= [
    $tools_core,
    $tools_gui
]

#---------------------------------------------------------------------------

tools_test = new_target_from_base("tools-test", TARGET_TYPE_EXECUTABLE)
tools_test.sources |= FileList[
    "src/tools/core/Delegate.specs.cpp",
    "src/tools/core/string.specs.cpp",
    "src/tools/core/reflection/reflection.specs.cpp",
    "src/tools/gui/geometry/SpatialNode.specs.cpp",
    "src/tools/gui/geometry/Rect.specs.cpp",
    "src/tools/test/main.cpp",
]

tools_test.vcpkg |= [
    "gtest"
]

tools_test.depends_on_target |= [
    $tools_core,
    $tools_gui
]

#---------------------------------------------------------------------------
namespace :tools do

    task :clean => [
        'core:clean',
        'gui:clean',
        'app:clean'
    ]

    task :rebuild => [
        'clean',
        'build'
    ]

    task :build => [
        'core:build',
        'gui:build',
        'app:build',
    ]

    if DESKTOP
        task :clean => 'test:clean'
        task :build => 'test:build'
        task :test  => 'test:run'
    end

    namespace :core do
        tasks_for_target( $tools_core )
    end

    task :gui
    namespace :gui do
        tasks_for_target( $tools_gui )
    end

    task :app
    namespace :app do
        tasks_for_target( app )
    end

    namespace :test do
        tasks_for_target( tools_test )
    end

end
#---------------------------------------------------------------------------