#include "History.h"
#include "File.h"

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
	while (commandsCursor < commands.size())
		commands.pop_back(); // TODO: memory leak to fix


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
		commands.at(commandsCursor)->redo();
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
	/* Do nothing if cursor is already well positionned */
	if (_pos == commandsCursor )
		return;

	/* Undo or redo the required times to get the command cursor well positionned */
	while (_pos != commandsCursor)
	{
		if (_pos > commandsCursor)
			redo();
		else
			undo();
	}

	/* After moving history, we force the file editor to evaluate the selected expression */
	auto file = this->getOwner()->as<File*>();
	file->evaluateSelectedExpression();
}

const char* Nodable::History::getCommandDescriptionAtPosition(size_t _commandId)
{
	return commands.at(_commandId)->getDescription();
}

void TextEditorBuffer::AddUndo(TextEditor::UndoRecord& _undoRecord) {

	auto cmd = new Cmd_TextEditor_InsertText(_undoRecord, mTextEditor);
	history->addAndExecute(cmd);
}
