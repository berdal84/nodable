#include <stdio.h>
#include <string>
#include <fstream> 			 // for ofstream
#include <iostream>          // for std::cout
#include "Application.h"
#include "ApplicationView.h"
#include "Test.h"            // for RunAll()

int main(int, char**)
{
	// Redirect standard output to a file
	freopen("nodable.log","w",stdout);

	// Run tests :
	NODABLE_VERIFY(Nodable::Test::RunAll());

	// Run the program
	std::string appName = "Nodable " NODABLE_VERSION;
	Nodable::Application nodable(appName.c_str());

	NODABLE_VERIFY(nodable.init());

	while (nodable.update())
	{
		if( nodable.hasComponent("view"))
			((Nodable::ApplicationView*)nodable.getComponent("view"))->draw();
	}

	nodable.shutdown();

	return 0;
}