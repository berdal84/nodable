namespace :tools_core do
    require_relative "common"

    def project()
        _project = default_project()
        _project[:source_files] |= FileList[
            "src/tools/core/EventManager.cpp",
            "src/tools/core/FileSystem.cpp",
            "src/tools/core/format.cpp",
            "src/tools/core/log.cpp",
            "src/tools/core/StateMachine.cpp",
            "src/tools/core/System.cpp",
            "src/tools/core/TaskManager.cpp",
        ]
        _project
    end

    def define_module_tasks()
        _project = project()

        desc "Build tools_core"
        multitask :build => object_files(_project)

        obj = object_files(_project)
        obj.each do |_obj|
            file _obj => object_dependencies(_obj, _project) do |_task|
                compile(_task.name, _project)
            end
        end
    end

end # namespace core