
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

int main(int argc, char* argv[])
{

    LOG_MESSAGE( "main", "%s\n", ghc::filesystem::path(argv[0]).c_str() );

    Log::SetVerbosityLevel("File", Log::Verbosity::Verbose);
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
