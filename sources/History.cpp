#include "History.h"
using namespace Nodable;

History* History::global = nullptr;

History::~History()
{
	for (auto cmd : commands )
		delete cmd;
}

void History::addAndExecute(Cmd* _cmd)
{
	commands.push_back(_cmd);
	commandsCursor++;
	_cmd->execute();
}

void History::undo()
{
	if (commandsCursor > 0)
	{
		commandsCursor--;
		commands.at(commandsCursor)->undo();
	}
}

void History::redo()
{
	if (commandsCursor > commands.size())
	{
		commands.at(commandsCursor)->execute();
		commandsCursor++;
	}
}


