#include "History.h"

using namespace Nodable;

History::~History()
{
	commands.clear();
}

void History::addAndExecute( std::unique_ptr<Cmd> _cmd)
{	
	/* First clear commands after the cursor */
	while (commandsCursor < commands.size())
	{
        commands.pop_back();
    }

	/* Then execute and store cmd */
    _cmd->execute();
	commands.push_back( std::move(_cmd) );
	commandsCursor = commands.size();


	/* Delete command history in excess */
    while (commands.size() > sizeMax)
    {
        commands.erase(commands.begin());
        commandsCursor--;
    }
}

void History::undo()
{
	if (commandsCursor > 0)
	{
		commandsCursor--;
		commands.at(commandsCursor)->undo();
		dirty = true;
	}
}

void History::redo()
{
	if (commandsCursor < commands.size())
	{
		commands.at(commandsCursor)->redo();
		commandsCursor++;
		dirty = true;
	}
}

void Nodable::History::clear()
{
	commands.clear();
	commandsCursor = 0;
}

void History::setCursorPosition(size_t _pos)
{
	/* Do nothing if cursor is already well positioned */
	if (_pos == commandsCursor )
		return;

	/* Undo or redo the required times to get the command cursor well positioned */
	while (_pos != commandsCursor)
	{
		if (_pos > commandsCursor)
			redo();
		else
			undo();
	}

	dirty = true;
}

std::string Nodable::History::getCommandDescriptionAtPosition(size_t _commandId)
{
	const auto headId = commands.size();
	
	std::string result;

	if ( _commandId < headId )
	{
		result = commands.at(_commandId)->getDescription();
	}
	else
    {
		result = "History HEAD";
	}	

	return result;
}

void TextEditorBuffer::AddUndo(TextEditor::UndoRecord& _undoRecord) {

	auto cmd = std::make_unique<Cmd_TextEditor_InsertText>(_undoRecord, mTextEditor);
	history->addAndExecute( std::move(cmd) );
}
