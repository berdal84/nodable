#include "Command_Manager.h"

#include "Asserts.h"
#include "bdc/String_Builder.hpp"
#include "Command.h"
#include "Config.h"
#include "Flags.h"

namespace ndbl
{

static Command_Manager* g_command_manager = nullptr;

void _command_manager_release_range(u32_t begin, u32_t end);

#define VERIFY_COMMAND_MANAGER_IS_INITIALIZED() NDBL_ASSERT( g_command_manager != nullptr, "g_command_manager is null, did you call command_manager_init() ?")

Command_Manager* command_manager_init()
{
    NDBL_ASSERT(g_command_manager == nullptr, "Cannot call init twice!");
    g_command_manager = bdc::memory_new<Command_Manager>();
    return g_command_manager;
}

Command_Manager* command_manager()
{
    VERIFY_COMMAND_MANAGER_IS_INITIALIZED();
    return g_command_manager;
}

void command_manager_shutdown()
{
    auto* manager = command_manager();
    command_manager_clear();
	array_release(manager->commands);
    bdc::memory_delete(manager);
    g_command_manager = nullptr;
}

void command_manager_begin_transaction()
{
    auto* manager = command_manager();
    manager->next_command_flags = Command_Flags_TRANSACTION_BEGIN;
}

void command_manager_end_transaction()
{
    auto* manager      = command_manager();
    auto& last_command = array_back(manager->commands);

    NDBL_ASSERT(!HAS_FLAGS( last_command.flags, Command_Flags_TRANSACTION_BEGIN), "Cannot end a transaction on the same command that transaction begins");
    last_command.flags |= Command_Flags_TRANSACTION_END;
}

void command_manager_push_command(Command& command, bool skip_command_execution)
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
    if ( !skip_command_execution )
    {
        command_do(&command);
    }

    array_append(manager->commands, command);

    // 
    // Ensure not to store too much undo commands.
    // We limit to a certain set_size, deleting first past commands, then future commands.
    // 
    // Note: I could use a ring buffer for this, but I actually want to be able to have an "infinite" buffer.
    // 
    u32_t command_in_memory_max_count = cfg->ui_history_size_max * 2;
    if ( manager->commands.size >= command_in_memory_max_count)
    {
        u32_t command_index_begin  = 0;
        u32_t command_index__end   = manager->commands.size-cfg->ui_history_size_max;
        _command_manager_release_range(command_index_begin, command_index__end);
        array_remove_range_ordered(manager->commands, command_index_begin, command_index__end);
    }

    manager->cursor = manager->commands.size;
}

void _command_manager_release_range(u32_t begin, u32_t end)
{
    NDBL_ASSERT( begin <= end );
    auto* manager = command_manager();
    for(u32_t i = begin; i < end; ++i)
    {
        auto& command = manager->commands[i];
        command_release(&command);
    }
}

void command_manager_undo()
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

void command_manager_redo()
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

void command_manager_clear()
{
    auto* manager = command_manager();
    _command_manager_release_range(0, manager->commands.size);
    array_clear(manager->commands);
}

void command_manager_set_cursor(u32_t pos)
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

bdc::String command_manager_get_cmd_description_at(u32_t pos)
{
    auto* manager = command_manager();

    if ( pos <= 0 )               return "History Begin";
    if ( pos >= manager->cursor ) return "History End";

    Command& cmd = manager->commands[pos];
    return cmd.description;
}

void command_manager_enable_text_editor_undo_buffer( bool enabled )
{
    auto* manager = command_manager();
    manager->push_text_editor_AddUndoRecord = enabled;
}

void command_manager_AddUndoHandler(Text_Editor& editor, Text_Editor::UndoRecord& undo_record)
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

} // namespace ndbl