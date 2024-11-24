require_relative 'base'
require_relative 'tools'

#---------------------------------------------------------------------------
ndbl_core = new_target_from_base("ndbl_core", TargetType::OBJECTS)
ndbl_core.sources |= FileList[
    "src/ndbl/core/language/Nodlang.cpp",
    "src/ndbl/core/language/Nodlang_biology.cpp",
    "src/ndbl/core/language/Nodlang_math.cpp",
    "src/ndbl/core/Code.cpp",
    "src/ndbl/core/Compiler.cpp",
    "src/ndbl/core/ComponentFactory.cpp",
    "src/ndbl/core/DirectedEdge.cpp",
    "src/ndbl/core/ForLoopNode.cpp",
    "src/ndbl/core/FunctionNode.cpp",
    "src/ndbl/core/Graph.cpp",
    "src/ndbl/core/IfNode.cpp",
    "src/ndbl/core/Instruction.cpp",
    "src/ndbl/core/Interpreter.cpp",
    "src/ndbl/core/LiteralNode.cpp",
    "src/ndbl/core/NodableHeadless.cpp",
    "src/ndbl/core/Node.cpp",
    "src/ndbl/core/NodeComponent.cpp",
    "src/ndbl/core/NodeFactory.cpp",
    "src/ndbl/core/Property.cpp",
    "src/ndbl/core/PropertyBag.cpp",
    "src/ndbl/core/Scope.cpp",
    "src/ndbl/core/Slot.cpp",
    "src/ndbl/core/SwitchBehavior.cpp",
    "src/ndbl/core/Token.cpp",
    #"src/ndbl/core/Token.specs.cpp",
    "src/ndbl/core/TokenRibbon.cpp",
    "src/ndbl/core/Utils.cpp",
    "src/ndbl/core/VariableNode.cpp",
    "src/ndbl/core/WhileLoopNode.cpp",
]
#---------------------------------------------------------------------------
ndbl_gui = new_target_from_base("ndbl_gui", TargetType::OBJECTS)
ndbl_gui.sources |= FileList[
    "src/ndbl/gui/CreateNodeCtxMenu.cpp",
    "src/ndbl/gui/GraphView.cpp",
    "src/ndbl/gui/Nodable.cpp",
    #"src/ndbl/gui/benchmark.cpp",
    "src/ndbl/gui/Config.cpp",
    "src/ndbl/gui/File.cpp",
    "src/ndbl/gui/FileView.cpp",
    "src/ndbl/gui/History.cpp",
    "src/ndbl/gui/NodableView.cpp",
    "src/ndbl/gui/NodeView.cpp",
    "src/ndbl/gui/Physics.cpp",
    "src/ndbl/gui/PropertyView.cpp",
    "src/ndbl/gui/ScopeView.cpp",
    "src/ndbl/gui/SlotView.cpp",
]
#---------------------------------------------------------------------------
ndbl_app = new_target_from_base("nodable", TargetType::EXECUTABLE)
ndbl_app.sources |= FileList[
    "src/ndbl/app/main.cpp",
]
ndbl_app.link_library |= [
    $tools_gui,
    $tools_core,
    $text_editor,
    ndbl_core,
    ndbl_gui
]

#---------------------------------------------------------------------------
task :ndbl => 'ndbl:build'
namespace :ndbl do

    task :build => ['core:build', 'gui:build', 'app:build']
    task :pack  => ['app:pack']

    namespace :core do
        tasks_for_target( ndbl_core )
    end
    
    namespace :gui do        
        tasks_for_target( ndbl_gui )
    end

    namespace :app do
        tasks_for_target( ndbl_app )
    end
end
#---------------------------------------------------------------------------