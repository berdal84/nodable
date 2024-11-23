
desc "Build tools"
task :tools => 'tools:build'

namespace :tools do

    desc "Build tools"
    task :build => ['core:build', 'gui:build', 'app:build']

    namespace :core do
        core = new_project("tools-core", "static")
        core[:sources] |= FileList[
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
        declare_project_tasks( core )
    end

    namespace :gui do
        gui = new_project("tools_gui", "static")
        gui[:sources] |= FileList[
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

        declare_project_tasks( gui )
    end

    namespace :app do
        app = new_project("tools-gui-example", "executable")
        app[:sources] |= FileList[
            "src/tools/gui-example/AppExample.cpp",
            "src/tools/gui-example/AppExampleView.cpp",
            "src/tools/gui-example/main.cpp"
        ]

        declare_project_tasks( app )
    end

end