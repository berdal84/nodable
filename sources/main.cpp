#include <stdio.h>
#include <string>
#include "Node_Application.h"

int main(int, char**)
{

	Nodable::Node_Application nodable("Nodable");

	if(!nodable.init())
		return -1;

	while (nodable.update())
	{
		nodable.draw();
	}

	nodable.shutdown();

	return 0;
}
