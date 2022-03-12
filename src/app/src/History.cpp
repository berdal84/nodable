#include <nodable/History.h>
#include <nodable/commands/Cmd_ReplaceText.h>

using namespace Nodable;

History::~History()
{
	for (auto cmd : m_commands )
		delete cmd;
}

void History::addAndExecute(ICommand* _cmd)
{	
	/* First clear commands after the cursor */
	while (m_commands_cursor < m_commands.size())
	{
        auto command = m_commands.back();
        delete command;
        m_commands.pop_back();
    }

	/* Then add and execute the new command */
	m_commands.push_back(_cmd);
    m_commands_cursor = m_commands.size();
	_cmd->execute();

	/* Delete command history in excess */
    while (m_commands.size() > m_size_max)
    {
        delete m_commands.front();
        m_commands.erase(m_commands.begin());
        m_commands_cursor--;
    }
}

void History::undo()
{
	if (m_commands_cursor > 0)
	{
		m_commands_cursor--;
        ICommand* command_to_undo = m_commands.at(m_commands_cursor);
        if ( command_to_undo->is_undoable() )
        {
            command_to_undo->undo();
            m_dirty = true;
        }
	}
}

void History::redo()
{
	if (m_commands_cursor < m_commands.size())
	{
		m_commands.at(m_commands_cursor)->redo();
		m_commands_cursor++;
        m_dirty = true;
	}
}

void History::clear()
{
	m_commands.clear();
    m_commands_cursor = 0;
}

void History::setCursorPosition(size_t _pos)
{
	/* Do nothing if cursor is already well positioned */
	if (_pos == m_commands_cursor )
		return;

	/* Undo or redo the required times to get the command cursor well positioned */
	while (_pos != m_commands_cursor)
	{
		if (_pos > m_commands_cursor)
			redo();
		else
			undo();
	}

    m_dirty = true;
}

std::string History::getCommandDescriptionAtPosition(size_t _commandId)
{
	const auto headId = m_commands.size();
	
	std::string result;

	if ( _commandId < headId )
	{
		result = m_commands.at(_commandId)->get_description();
	}
	else
    {
		result = "History HEAD";
	}	

	return result;
}

void TextEditorBuffer::AddUndo(TextEditor::UndoRecord& _undoRecord) {

	auto cmd = new Cmd_ReplaceText(_undoRecord, m_Text_editor);
	m_history->addAndExecute(cmd);
}
