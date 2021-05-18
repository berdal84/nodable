
#include <string>

#include <nodable/Application.h>
#include <nodable/ApplicationView.h>
#include <nodable/Config.h>

using namespace Nodable;

int main(int argc, char* argv[])
{

//    Log::SetVerbosityLevel("GraphTraversal", Log::Verbosity::Verbose);
//    Log::SetVerbosityLevel("Parser", Log::Verbosity::Verbose);
//    Log::SetVerbosityLevel("GraphNode", Log::Verbosity::Verbose);
//    Log::SetVerbosityLevel("Runner", Log::Verbosity::Verbose);

	Application nodable("Nodable " NODABLE_VERSION_EXTENDED );
	nodable.init();
    auto startupFilePath = nodable.getAssetPath("startup.txt");
	nodable.openFile(startupFilePath); // Init and open a startup file

	try {
        while ( nodable.update() )
        {
            if (ApplicationView* view = nodable.getView() )
            {
                view->draw();
            }
        }
    } catch (std::exception& err) {
        LOG_ERROR("main", "Application crashes: %s\n", err.what() );
	}
	nodable.shutdown();
    LOG_FLUSH();
	return 0;
}

