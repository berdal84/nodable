require_relative "rakefile.common"

namespace :tools do

    namespace :core do

        project = new_project("tools_core")

        project[:sources] |= FileList[
            "src/tools/core/reflection/qword.cpp",
            "src/tools/core/reflection/Type.cpp",
            "src/tools/core/reflection/TypeRegister.cpp",
            "src/tools/core/reflection/variant.cpp",
            "src/tools/core/memory/pointers.cpp",
            "src/tools/core/EventManager.cpp",
            "src/tools/core/FileSystem.cpp",
            "src/tools/core/format.cpp",
            "src/tools/core/log.cpp",
            "src/tools/core/StateMachine.cpp",
            "src/tools/core/System.cpp",
            "src/tools/core/TaskManager.cpp",
            "libs/whereami/src/whereami.c"
        ]

        multitask :build => configure_project( project )

    end # namespace :core

    namespace :gui do

        project = new_project("tools_gui")

        project[:sources] |= FileList[
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

        multitask :build => configure_project( project )

    end # namespace :gui

    task :build => ['core:build', 'gui:build']

end # namespace :tools
