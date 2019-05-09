#include "Command.h"
using namespace Nodable;

History::~History()
{
	for (auto cmd : history )
		delete cmd;
}

void History::addAndExecute(Cmd*)
{

}

void History::undo()
{

}

void History::redo()
{

}


