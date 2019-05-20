
#include <stdio.h>
#include <string>
#include "Application.h"
#include "ApplicationView.h"
#include "Test.h"            // for RunAll()

int main(int argc, char* argv[])

{
	// Build a new Nodable application
	std::string appName = "Nodable " NODABLE_VERSION;
	Nodable::Application nodable(appName.c_str());

	NODABLE_VERIFY(Nodable::Test::RunAll());
	NODABLE_VERIFY(nodable.init());

	while (nodable.update())
	{
		if( nodable.hasComponent("view"))
			((Nodable::ApplicationView*)nodable.getComponent("view"))->draw();
	}

	nodable.shutdown();

	return 0;
}