require_relative "rakefile.common"

namespace :tools do
namespace :core do

    _project = default_project("tools_core")

    _project[:source_files] |= FileList[
        "src/tools/core/EventManager.cpp",
        "src/tools/core/FileSystem.cpp",
        "src/tools/core/format.cpp",
        "src/tools/core/log.cpp",
        "src/tools/core/StateMachine.cpp",
        "src/tools/core/System.cpp",
        "src/tools/core/TaskManager.cpp",
    ]

    multitask :build => object_files(_project)

    objects = object_files(_project)
    objects.each do |_object|
        file _object => object_dependencies(_object, _project) do |_task|
            compile(_task.name, _project)
        end
    end

end # namespace :core

namespace :gui do

    _project = default_project("tools_gui")

    _project[:source_files] |= FileList[
      "src/tools/core/EventManager.cpp",
      "src/tools/core/FileSystem.cpp",
      "src/tools/core/format.cpp",
      "src/tools/core/log.cpp",
      "src/tools/core/StateMachine.cpp",
      "src/tools/core/System.cpp",
      "src/tools/core/TaskManager.cpp",
    ]

    multitask :build => object_files(_project)

    obj = object_files(_project)
    obj.each do |_obj|
      file _obj => object_dependencies(_obj, _project) do |_task|
        compile(_task.name, _project)
      end
    end

end # namespace :gui
end # namespace :tools
