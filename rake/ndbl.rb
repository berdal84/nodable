require_relative 'tools'
#---------------------------------------------------------------------------
core = new_project("ndbl_core", "objects")
core[:sources] |= FileList[
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
gui = new_project("ndbl_gui", "objects")
gui[:sources] |= FileList[
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
app = new_project("nodable", "executable")
app[:sources] |= FileList[
    "src/ndbl/app/main.cpp",
]
app[:depends_on] |= [
    $tools_core, $tools_gui,
    $text_editor,
    core, gui
]
#---------------------------------------------------------------------------
task :ndbl => 'ndbl:build'
namespace :ndbl do

    task :build => ['core:build', 'gui:build', 'app:build']
    task :pack  => ['app:pack']

    namespace :core do
        declare_project_tasks( core )
    end
    
    namespace :gui do        
        declare_project_tasks( gui )
    end

    namespace :app do
        declare_project_tasks( app )
    end
end
#---------------------------------------------------------------------------