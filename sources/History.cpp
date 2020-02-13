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
	/* First clear commands after the cursor */
	while (commandsCursor > commands.size())
		commands.erase(commands.end());

	/* Then add and execute the new command */
	commands.push_back(_cmd);
	commandsCursor = commands.size();
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
	if (commandsCursor < commands.size())
	{
		commands.at(commandsCursor)->execute();
		commandsCursor++;
	}
}

void Nodable::History::clear()
{
	commands.clear();
	commandsCursor = 0;
}

void History::setCursorPosition(int _pos)
{
	while (_pos != commandsCursor)
	{
		if (_pos > commandsCursor)
			redo();
		else
			undo();
	}
}

const char* Nodable::History::getCommandDescriptionAtPosition(size_t _commandId)
{
	return commands.at(_commandId)->getDescription();
}

void TextEditorBuffer::AddUndo(TextEditor::UndoRecord& _undoRecord, TextEditor& _textEditor) {

	auto cmd = new Cmd_TextEditor(_undoRecord, _textEditor);
	history->addAndExecute(cmd);
}
