#include "Command_Manager.h"

#include "Asserts.h"
#include "bdc/String_Builder.hpp"
#include "Command.h"
#include "Config.h"
#include "Flags.h"

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
    array_init(g_command_manager->commands);
    return g_command_manager;
}

ndbl::Command_Manager* ndbl::command_manager()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();
    return g_command_manager;
}

void ndbl::command_manager_shutdown()
{
    auto* manager = command_manager();
	array_release(manager->commands);
    bdc::memory_delete(manager);
    g_command_manager = nullptr;
}

void ndbl::command_manager_begin_transaction()
{
    auto* manager = command_manager();
    manager->next_command_flags = Command_Flags_TRANSACTION_BEGIN;
}

void ndbl::command_manager_end_transaction()
{
    auto* manager      = command_manager();
    auto& last_command = array_back(manager->commands);

    VERIFY(!HAS_FLAGS( last_command.flags, Command_Flags_TRANSACTION_BEGIN), "Cannot end a transaction on the same command that transaction begins");
    last_command.flags |= Command_Flags_TRANSACTION_END;
}

void ndbl::command_manager_push_command(Command& command, bool _from_text_editor)
{
    Config* cfg     = config();
    auto*   manager = command_manager();

    // Set flags
    command.flags = manager->next_command_flags;
    manager->next_command_flags = 0;

    // clear any future commands (when we undo, commands are moved from past to future)
    if( manager->cursor < manager->commands.size )
    {
        array_resize(manager->commands, manager->cursor);
    }

    // execute the command except if it concern text_editor.
    // since modification is already handled by text editor itself
    if ( !_from_text_editor )
    {
        command_do(&command);
    }

    array_append(manager->commands, command);

    /**
     * Ensure not to store too much undo commands.
     * We limit to a certain set_size, deleting first past commands, then future commands.
     */
    if ( manager->commands.size >= cfg->ui_history_size_max * 2 )
    {
        array_remove_range_ordered(manager->commands, 0, manager->commands.size-cfg->ui_history_size_max);
    }

    manager->cursor = manager->commands.size;
}

void ndbl::command_manager_undo()
{
    auto* manager = command_manager();

	if ( manager->cursor > 0 )
	{
        Command& command_to_undo = manager->commands[manager->cursor-1];
        command_undo(&command_to_undo);
        --manager->cursor;
        manager->is_dirty = true;
    }
}

void ndbl::command_manager_redo()
{
    auto* manager = command_manager();

	if ( manager->cursor < manager->commands.size )
	{
        Command& command_to_redo = manager->commands[manager->cursor];
        command_redo(&command_to_redo);
        ++manager->cursor;
        manager->is_dirty = true;
	}
}

void ndbl::command_manager_clear()
{
    auto* manager = command_manager();
    array_clear(manager->commands);
}

void ndbl::command_manager_set_cursor(u32_t pos)
{
    auto* manager = command_manager();

	while ( manager->cursor != pos )
	{
		if ( manager->cursor < pos )
        {
			command_manager_redo();
        }
		else
        {
			command_manager_undo();
        }
	}
}

bdc::String ndbl::command_manager_get_cmd_description_at(u32_t pos)
{
    auto* manager = command_manager();

    if ( pos <= 0 )               return "History Begin";
    if ( pos >= manager->cursor ) return "History End";

    Command& cmd = manager->commands[pos];
    return cmd.description;
}

std::pair<int, int> ndbl::command_manager_get_command_id_range()
{
    auto* manager = command_manager();
    return { 0, manager->cursor };
}

size_t ndbl::command_manager_get_size()
{
    auto* manager = command_manager();
    return manager->commands.size;
}

void ndbl::command_manager_enable_text_editor_undo_buffer( bool enabled )
{
    auto* manager = command_manager();
    manager->push_text_editor_AddUndoRecord = enabled;
}

void ndbl::command_manager_AddUndoHandler(Text_Editor& editor, Text_Editor::UndoRecord& undo_record)
{
    auto* manager = command_manager();

    if ( manager->push_text_editor_AddUndoRecord )
    {
        auto* undo_record_copy = bdc::memory_new<Text_Editor::UndoRecord>();
        *undo_record_copy = undo_record;
	    Command cmd = command_text_undo_record({ undo_record_copy, &editor});
        command_manager_push_command(cmd, true);
    }
}
