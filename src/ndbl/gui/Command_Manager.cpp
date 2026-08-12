#include "Command_Manager.h"
#include "tools/core/Asserts.h"
#include "ndbl/gui/Config.h"
#include "ndbl/gui/Command.h"

using namespace ndbl;

#define VERIFY_COMMAND_MANAGER_IS_INITIALIZED() VERIFY( ndbl::g_command_manager != nullptr, "g_command_manager is null, did you call command_manager_init() ?")

// private
namespace ndbl
{
    static Command_Manager* g_command_manager = nullptr;
}

ndbl::Command_Manager* ndbl::command_manager_init()
{
    VERIFY(g_command_manager == nullptr, "Cannot call init twice!");
    g_command_manager = new Command_Manager();
    return g_command_manager;
}

ndbl::Command_Manager* ndbl::command_manager()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();
    return g_command_manager;
}

void ndbl::command_manager_shutdown()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();
	g_command_manager->past.clear();
	g_command_manager->future.clear();
}

ndbl::Context make_context()
{
    Context ctx;
    ctx.text_editor = g_command_manager->text_editor_undo_buffer.text_editor;
    return ctx;
}

void ndbl::command_manager_push_command(Command& command, bool _from_text_editor)
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

    Config* cfg = get_config();

    // clear any future commands (when we undo, commands are moved from past to future)
    g_command_manager->future.clear();

    // execute the command except if it concern text_editor.
    // since modification is already handled by text editor itself
    if ( !_from_text_editor )
    {
        Context context = make_context();
        command_do(&command, &context);
    }

    g_command_manager->past.push_front(command);

    /**
     * Ensure not to store too much undo commands.
     * We limit to a certain set_size, deleting first past commands, then future commands.
     */
    while ( g_command_manager->past.size() > cfg->ui_history_size_max )
    {
        g_command_manager->past.pop_back();
    }
}

void ndbl::command_manager_undo()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

	if ( !g_command_manager->past.empty() )
	{
        Command& command_to_undo = g_command_manager->past.front();
        Context context = make_context();
        command_undo(&command_to_undo, &context);
        g_command_manager->past.pop_front();
        g_command_manager->future.push_front(command_to_undo);
        g_command_manager->is_dirty = true;
    }
}

void ndbl::command_manager_redo()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

	if ( !g_command_manager->future.empty() )
	{
        Context context = make_context();
        Command& command_to_redo = g_command_manager->future.front();
        command_redo(&command_to_redo, &context);
        g_command_manager->future.pop_front();
        g_command_manager->past.push_front(command_to_redo);
        g_command_manager->is_dirty = true;
	}
}

void ndbl::command_manager_clear()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

	g_command_manager->past.clear();
    g_command_manager->future.clear();
}

void ndbl::command_manager_move_cursor(int delta)
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

	/* Do nothing if cursor is already well positioned */
	if (delta == 0 ) return;

	/* Undo or redo the required times to get the command cursor well positioned */
	while (delta != 0)
	{
		if (delta > 0)
        {
			command_manager_redo();
            delta--;
        }
		else
        {
			command_manager_undo();
            delta++;
        }
	}

    g_command_manager->is_dirty = true;
}

std::string ndbl::command_manager_get_cmd_description_at(int _cmd_position)
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

	std::string result;

    if (_cmd_position <= -(int)g_command_manager->past.size())
    {
        result.append("History Begin");
    }
    else if ( _cmd_position >= (int)g_command_manager->future.size() )
    {
        result.append("History End");
    }
    else
	{
        if (_cmd_position <= 0 )
        {
            Command& cmd = g_command_manager->past.at(abs(_cmd_position));
            result.append( cmd.description );
        }
        else
        {
            Command& cmd = g_command_manager->future.at(_cmd_position-1); // index zero is m_past.front()
            result.append( cmd.description );
        }
	}

	return result;
}

std::pair<int, int> ndbl::command_manager_get_command_id_range()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

    // (begin index, end index)
    return std::make_pair(-(int)g_command_manager->past.size(), (int)g_command_manager->future.size());
}

Text_Editor_Undo_Buffer* ndbl::command_manager_configure_text_editor_undo_buffer( TextEditor* _text_editor )
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

    g_command_manager->text_editor_undo_buffer.text_editor = _text_editor;
    g_command_manager->text_editor_undo_buffer.enabled     = true;
    return &g_command_manager->text_editor_undo_buffer;
}

size_t ndbl::command_manager_get_size()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

    return g_command_manager->past.size() + g_command_manager->future.size();
}

void ndbl::command_manager_enable_text_editor_undo_buffer( bool enabled )
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

    g_command_manager->text_editor_undo_buffer.enabled = enabled;
}

void Text_Editor_Undo_Buffer::AddUndo(TextEditor::UndoRecord& undo_record)
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

    if ( enabled )
    {
	    Command cmd = command_text_undo_record({ new TextEditor::UndoRecord(undo_record), text_editor});
        command_manager_push_command(cmd, true);
    }
}
