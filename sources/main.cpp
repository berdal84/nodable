
#include <stdio.h>
#include <string>
#include "Application.h"
#include "ApplicationView.h"
#include "Test.h"            // for RunAll()

using namespace Nodable;

int main(int argc, char* argv[])

{
	// Build a new Nodable application
	std::string appName = "Nodable " NODABLE_VERSION;
	Application nodable(appName.c_str());
	Test::RunAll();
	nodable.init();

	while (nodable.update())
	{
		if( nodable.hasComponent("view"))
			nodable.getComponent<View>("view")->draw();
	}

	nodable.shutdown();

	return 0;
}