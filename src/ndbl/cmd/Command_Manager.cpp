#include "Command_Manager.h"

#include "ndbl/Asserts.h"
#include "bdc/String_Builder.hpp"
#include "ndbl/Config.h"
#include "ndbl/Flags.h"

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

void begin_command_group()
{
    auto* manager = command_manager();
    manager->next_command_flags = Cmd_Flags_GROUP_BEGIN;
}

void end_command_group()
{
    auto* manager      = command_manager();
    auto& last_command = array_back(manager->commands);

    NDBL_ASSERT(!HAS_FLAGS( last_command.flags, Cmd_Flags_GROUP_BEGIN), "Cannot end a transaction on the same command that transaction begins");
    last_command.flags |= Cmd_Flags_GROUP_END;
}

Command* push_command(const Command& command_to_push, const Cmd_Push_Params& params)
{
    NDBL_ASSERT( command_to_push.proc_do );
    NDBL_ASSERT( command_to_push.proc_undo );

    Config* cfg     = config();
    auto*   manager = command_manager();

    // clear any future commands (when we undo, commands are moved from past to future)
    if( manager->cursor < manager->commands.size )
    {
        array_resize(manager->commands, manager->cursor);
    }

    Command& command = array_append(manager->commands, command_to_push);

    // take flags
    command.flags = manager->next_command_flags; 
    manager->next_command_flags = 0;

    // execute the command except if it concern text_editor.
    // since modification is already handled by text editor itself
    if ( !params.skip_command_do_at_push )
    {
        command_do(&command);
    }
    

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

    return &command;
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

    if( manager->cursor == 0)
    {
        return;
    }

    Command* cmd = &manager->commands[manager->cursor-1];

    if( cmd->flags & Cmd_Flags_GROUP_END )
    {
        while ( cmd )
        {
            command_undo(cmd);
            --cmd;
            --manager->cursor;
            
            NDBL_ASSERT((cmd->flags & Cmd_Flags_GROUP_END) == 0, "nesting not allowed!");

            if( cmd->flags & Cmd_Flags_GROUP_BEGIN )
            {
                break;
            }
        }
    }
    else
    {
        command_undo(cmd);
        --manager->cursor;        
    }
    manager->is_dirty = true;
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

    if ( pos <= 0 )               return string("History Begin");
    if ( pos >= manager->cursor ) return string("History End");

    Command& cmd = manager->commands[pos];
    return string(cmd.description);
}

void command_manager_enable_text_editor_undo_buffer( bool enabled )
{
    auto* manager = command_manager();
    manager->push_text_editor_AddUndoRecord = enabled;
}

void command_manager_handle_undo_record(Text_Editor& editor, Text_Editor::UndoRecord& undo_record)
{
    auto* manager = command_manager();

    if ( manager->push_text_editor_AddUndoRecord )
    {
	    Command cmd = make_cmd__text_undo_record({ undo_record, &editor});
        push_command(cmd, { .skip_command_do_at_push = true }); // I skip the command_do because the change was done already by the text editor.
    }
}

} // namespace ndbl