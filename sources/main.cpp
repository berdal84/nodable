
// Standard includes
#include <stdio.h>
#include <string>
#include <iostream>

// Nodable includes
#include "Application.h"
#include "Test.h"

using namespace Nodable;

int main(int argc, char* argv[])
{	
	// If --test is passed as first arg, we run tests
	if( argc == 2 && std::string(argv[1]) == "--run-tests")
	{
		if ( !Test_RunAll() ) {
			std::cout << "Tests failed, press a key to exit program" << std::endl;
			std::cin.get();
			return 1;
		}
		return 0;
	}

	// Here the tests are all OK, we can instantiate, init and run nodable loop.

	Application nodable("Nodable " NODABLE_VERSION);

	nodable.init();

	while (nodable.update())
	{
		if(auto view = nodable.getComponent<View>())
			view->draw();
	}

	nodable.shutdown();

	return 0;
}

