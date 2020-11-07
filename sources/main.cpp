
// Standard includes
#include <stdio.h>
#include <string>
#include <iostream>
#include <filesystem>

// Nodable includes
#include "Application.h"
#include "ApplicationView.h"
#include "Test.h"
#include "Config.h"

using namespace Nodable;

int main(int argc, char* argv[])
{	
	// If --test is passed as first arg, we run tests
	if( argc == 2 && std::string(argv[1]) == "--run-tests")
	{
	    Nodable::Log::SetVerbosityLevel(4u);

		if ( !Test_RunAll() )
		{
			std::cout << "Tests failed." << std::endl;
			return 1;
		}
		return 0;
	}

	// Here the tests are all OK, we can instantiate, init and run nodable loop.

	Application nodable("Nodable " NODABLE_VERSION);
    nodable.newComponent<ApplicationView>();

	nodable.init();
    auto startupFilePath = nodable.getAssetPath("startup.txt");
	std::cout << "Opening startup file: " << startupFilePath << std::endl;
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

