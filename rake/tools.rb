require_relative 'libs'
#---------------------------------------------------------------------------
$tools_core = new_project("tools_core", "objects")
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
$tools_core.depends_on |= [$whereami] # this lib is only objects
#---------------------------------------------------------------------------
$tools_gui = new_project("tools_gui", "objects")
$tools_gui.sources |= FileList[
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
$tools_gui.depends_on |= [$gl3w, $lodepng, $imgui]
#---------------------------------------------------------------------------
app = new_project("tools-gui-example", "executable")
app.sources |= FileList[
    "src/tools/gui-example/AppExample.cpp",
    "src/tools/gui-example/AppExampleView.cpp",
    "src/tools/gui-example/main.cpp"
]
app.depends_on |= [$tools_core, $tools_gui]
#---------------------------------------------------------------------------
desc "Build tools"
task :tools => 'tools:build'
namespace :tools do

    task :build => :app

    task :core => 'core:build'
    namespace :core do
        declare_project_tasks( $tools_core )
    end

    task :gui => ['gui:build']
    namespace :gui do
        declare_project_tasks( $tools_gui )
    end

    task :app => ['app:build']
    namespace :app do
        declare_project_tasks( app )
    end
end
#---------------------------------------------------------------------------