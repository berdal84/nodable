#include "History.h"
#include "commands/Cmd_WrappedTextEditorUndoRecord.h"

using namespace ndbl;

History::~History()
{
	m_past.clear();
	m_future.clear();
}

void History::push_command(std::shared_ptr<AbstractCommand> _cmd, bool _from_text_editor)
{
    // clear any future commands (when we undo, commands are moved from past to future)
    m_future.clear();

    // execute the command except if it concern text_editor.
    // since modification is already handled by text editor itself
    if ( !_from_text_editor )
    {
        _cmd->execute();
    }

    m_past.push_front(_cmd);


    /**
     * Ensure not to store too much undo commands.
     * We limit to a certain size, deleting first past commands, then future commands.
     */
    while ( m_past.size() > m_size_max )
    {
        m_past.pop_back();
    }
}

void History::undo()
{
	if ( !m_past.empty() )
	{
        std::shared_ptr<AbstractCommand> command_to_undo = m_past.front();
        command_to_undo->undo();
        m_past.pop_front();
        m_future.push_front(command_to_undo);
        is_dirty = true;
    }
}

void History::redo()
{
	if ( !m_future.empty() )
	{
        std::shared_ptr<AbstractCommand> command_to_redo = m_future.front();
        command_to_redo->redo();
        m_future.pop_front();
        m_past.push_front(command_to_redo);
        is_dirty = true;
	}
}

void History::clear()
{
	m_past.clear();
    m_future.clear();
}

void History::move_cursor(int _move)
{
	/* Do nothing if cursor is already well positioned */
	if (_move == 0 ) return;

	/* Undo or redo the required times to get the command cursor well positioned */
	while (_move != 0)
	{
		if (_move > 0)
        {
			redo();
            _move--;
        }
		else
        {
			undo();
            _move++;
        }
	}

    is_dirty = true;
}

std::string History::get_cmd_description_at(int _cmd_position)
{
	std::string result;

    if (_cmd_position <= -(int)m_past.size())
    {
        result.append("History Begin");
    }
    else if ( _cmd_position >= (int)m_future.size() )
    {
        result.append("History End");
    }
    else
	{
        std::shared_ptr<AbstractCommand> cmd;
        if (_cmd_position <= 0 )
        {
            cmd = m_past.at(abs(_cmd_position));
            result.append( cmd->get_description() );
        }
		else
        {
            cmd = m_future.at(_cmd_position-1); // index zero is m_past.front()
            result.append( cmd->get_description() );
        }
	}

	return result;
}

std::pair<int, int> History::get_command_id_range()
{
    // (begin index, end index)
    return std::make_pair(-(int)m_past.size(), (int)m_future.size());
}

void TextEditorBuffer::AddUndo(TextEditor::UndoRecord& _undoRecord)
{
    if ( m_enabled )
    {
	    auto cmd = std::make_shared<Cmd_WrappedTextEditorUndoRecord>(_undoRecord, m_Text_editor);
        m_history->push_command(cmd, true);
    }
}
