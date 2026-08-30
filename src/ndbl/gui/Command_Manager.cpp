#include "Command_Manager.h"
#include "bdc/String_Builder.hpp"
#include "core/Flags.h"
#include "gui/Nodable.h"
#include "tools/core/Asserts.h"
#include "ndbl/gui/Config.h"
#include "ndbl/gui/Command.h"

// private
namespace ndbl
{
    static Command_Manager* g_command_manager = nullptr;
}

#define VERIFY_COMMAND_MANAGER_IS_INITIALIZED() VERIFY( ndbl::g_command_manager != nullptr, "g_command_manager is null, did you call command_manager_init() ?")

ndbl::Command_Manager* ndbl::command_manager_init()
{
    VERIFY(g_command_manager == nullptr, "Cannot call init twice!");
    g_command_manager = bdc::memory_new<Command_Manager>();
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
    bdc::memory_delete(g_command_manager);
    g_command_manager = nullptr;
}

void ndbl::command_manager_begin_transaction()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();
    g_command_manager->next_command_flags = Command_Flags_TRANSACTION_BEGIN;
}

void ndbl::command_manager_end_transaction()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();
    VERIFY(!HAS_FLAGS(g_command_manager->past.back().flags, Command_Flags_TRANSACTION_BEGIN), "Cannot end a transaction on the same command that transaction begins");
    g_command_manager->past.back().flags |= Command_Flags_TRANSACTION_END;
}

void ndbl::command_manager_push_command(Command& command, bool _from_text_editor)
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

    Config* cfg = config();

    // Set flags
    command.flags = g_command_manager->next_command_flags;
    g_command_manager->next_command_flags = 0;

    // clear any future commands (when we undo, commands are moved from past to future)
    g_command_manager->future.clear();

    // execute the command except if it concern text_editor.
    // since modification is already handled by text editor itself
    if ( !_from_text_editor )
    {
        command_do(&command);
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
        command_undo(&command_to_undo);
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
        Command& command_to_redo = g_command_manager->future.front();
        command_redo(&command_to_redo);
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

bdc::String ndbl::command_manager_get_cmd_description_at(int _cmd_position)
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

	bdc::String_Builder sb{};

    if (_cmd_position <= -(int)g_command_manager->past.size())
    {
        bdc::string_builder_append(sb, "History Begin");
    }
    else if ( _cmd_position >= (int)g_command_manager->future.size() )
    {
        bdc::string_builder_append(sb, "History End");
    }
    else
	{
        if (_cmd_position <= 0 )
        {
            Command& cmd = g_command_manager->past.at(abs(_cmd_position));
            bdc::string_builder_append(sb, cmd.description );
        }
        else
        {
            Command& cmd = g_command_manager->future.at(_cmd_position-1); // index zero is m_past.front()
            bdc::string_builder_append(sb, cmd.description );
        }
	}

	return bdc::string_builder_build_string(sb);
}

std::pair<int, int> ndbl::command_manager_get_command_id_range()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

    // (begin index, end index)
    return std::make_pair(-(int)g_command_manager->past.size(), (int)g_command_manager->future.size());
}

ndbl::Text_Editor_Undo_Buffer* ndbl::command_manager_configure_text_editor_undo_buffer( TextEditor* _text_editor )
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

void ndbl::Text_Editor_Undo_Buffer::AddUndo(TextEditor::UndoRecord& undo_record)
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();

    if ( enabled )
    {
        auto* undo_record_copy = bdc::memory_new<TextEditor::UndoRecord>();
        *undo_record_copy = undo_record;
	    Command cmd = command_text_undo_record({ undo_record_copy, text_editor});
        command_manager_push_command(cmd, true);
    }
}
