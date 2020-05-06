
#include <stdio.h>
#include <string>

#include <Application.h>
#include <Test.h>            // for RunAll()
#include <ApplicationView.h>

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
		if(auto view = nodable.getComponent<View>())
			view->draw();
	}

	nodable.shutdown();

	return 0;
}