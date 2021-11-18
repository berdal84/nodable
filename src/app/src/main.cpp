
#include <mirror/mirror.h>
#include <string>

#include <nodable/App.h>
#include <nodable/AppView.h>
#include <nodable/Config.h>
#include <nodable/GraphNodeView.h>
#include <nodable/FileView.h>
#include <nodable/ComputeBase.h>
#include <nodable/GraphNode.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/ComputeUnaryOperation.h>
#include <nodable/ComputeBinaryOperation.h>
#include <nodable/Settings.h>

using namespace Nodable;

MIRROR_INITIALIZER

// Declare reflected classes:
MIRROR_CLASS_DEFINITION(Settings);
// View and derived
MIRROR_CLASS_DEFINITION(View)
MIRROR_CLASS_DEFINITION(AppView)
MIRROR_CLASS_DEFINITION(FileView)
MIRROR_CLASS_DEFINITION(Component)
MIRROR_CLASS_DEFINITION(ComputeBase)
MIRROR_CLASS_DEFINITION(ComputeFunction)
MIRROR_CLASS_DEFINITION(ComputeUnaryOperation)
MIRROR_CLASS_DEFINITION(ComputeBinaryOperation)
MIRROR_CLASS_DEFINITION(NodeView) // inherits View and Component
MIRROR_CLASS_DEFINITION(GraphNodeView) // inherits NodeView
// Node and derived
MIRROR_CLASS_DEFINITION(Node)
MIRROR_CLASS_DEFINITION(GraphNode)
MIRROR_CLASS_DEFINITION(VariableNode)
MIRROR_CLASS_DEFINITION(LiteralNode)
MIRROR_CLASS_DEFINITION(InstructionNode)
MIRROR_CLASS_DEFINITION(AbstractCodeBlockNode)
MIRROR_CLASS_DEFINITION(CodeBlockNode)
MIRROR_CLASS_DEFINITION(ScopedCodeBlockNode)
MIRROR_CLASS_DEFINITION(ConditionalStructNode)


int main(int argc, char* argv[])
{

//    Log::SetVerbosityLevel("GraphTraversal", Log::Verbosity::Verbose);
//    Log::SetVerbosityLevel("Parser", Log::Verbosity::Verbose);
//    Log::SetVerbosityLevel("GraphNode", Log::Verbosity::Verbose);
//    Log::SetVerbosityLevel("Runner", Log::Verbosity::Verbose);

	App nodable("Nodable " NODABLE_VERSION_EXTENDED );
	nodable.init();
    auto startupFilePath = nodable.getAssetPath("txt/startup.txt");
	nodable.openFile(startupFilePath); // Init and open a startup file

	try {
        while ( nodable.update() )
        {
            if (AppView* view = nodable.getView() )
            {
                view->draw();
            }
        }
    } catch (std::exception& err) {
        LOG_ERROR("main", "Application crashes: %s\n", err.what() )
	}
	nodable.shutdown();
    LOG_FLUSH()
	return 0;
}
