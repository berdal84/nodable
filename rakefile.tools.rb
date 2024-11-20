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
    ]

    project[:includes] |= FileList[
        "src/",
        "libs/",
        "libs/whereami/src"
    ]

    declare_project_tasks( project )

end # namespace :core
end # namespace :tools
