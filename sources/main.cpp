
// Standard includes
#include <stdio.h>
#include <string>
#include <iostream>
#include <filesystem>

// Nodable includes
#include "Application.h"
#include "Config.h"

using namespace Nodable;

int main(int argc, char* argv[])
{

//    Log::SetVerbosityLevel("NodeTraversal", Log::Verbosity::Verbose);
//    Log::SetVerbosityLevel("Parser", Log::Verbosity::Verbose);
//    Log::SetVerbosityLevel("GraphNode", Log::Verbosity::Verbose);

	Application nodable("Nodable " NODABLE_VERSION_EXTENDED );
	nodable.init();
    auto startupFilePath = nodable.getAssetPath("startup.txt");
	nodable.openFile(startupFilePath); // Init and open a startup file

	while ( nodable.update() != UpdateResult::Stopped )
	{
		if(auto view = nodable.getComponent<View>())
        {
            view->draw();
        }
	}

	nodable.shutdown();

	return 0;
}

