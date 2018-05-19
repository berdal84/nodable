#include <stdio.h>
#include <string>
#include "Node_Application.h"
#include "ApplicationView.h"
#include "Test.h"            // for RunAll()

int main(int, char**)
{

	// Run tests :
	if (!Nodable::Test::RunAll())
	{
		return -1;
	}

	// run the program
	Nodable::Node_Application nodable("Nodable");

	if(!nodable.init())
		return -1;

	while (nodable.update())
	{
		if( nodable.hasComponent("view"))
			((Nodable::ApplicationView*)nodable.getComponent("view"))->draw();
	}

	nodable.shutdown();

	return 0;
}