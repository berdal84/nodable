#pragma once

#include <ctime>

#include "bdc/Array.hpp"
#include "bdc/String.hpp"
#include "Command.h"
#include "texteditor/Text_Editor.h"

namespace ndbl
{
    /**
     * The history is responsible for undo/redo commands.
     * It has_flags two containers to store past/future commands
     *     (past)    (now)    (future)
     * |oooooooooooooo|-------------------|
     */
	struct Command_Manager
	{
        bool 			  			is_dirty = false;
		bool                    	push_text_editor_AddUndoRecord = true;
		Command_Flags           	next_command_flags;
		u32_t                       cursor; // ex: if cursor is 5, it means we have 5 commands in the past ("c c c c c <cursor> c ...")
		Resizable_Array<Command> 	commands;
    };

	Command_Manager* command_manager_init();
	void             command_manager_shutdown();
	Command_Manager* command_manager();

	/**
	* Push a command and execute it.
	* In some cases the command may not be added to the history or executed, check definition.
	* @param _from_text_editor should not be set except if command comes from Text_Editor.
	*                          This flag is here to state legacy history mode (text based) and
	*                          hybrid mode (Text/Graph).
	*/
	void    			command_manager_push_command(Command&, bool _from_text_editor = false);
	void 				command_manager_begin_transaction();
	void 				command_manager_end_transaction();
	void    			command_manager_enable_text_editor_undo_buffer(bool _val);
	void    			command_manager_undo();
	void    			command_manager_redo();
	void    			command_manager_clear();
	size_t  			command_manager_get_size(); /** To get the set_size of the history (command count) */
	void                command_manager_set_cursor(u32_t pos); /** undo/redo automatically to set the cursor at a given position */
	bdc::String         command_manager_get_cmd_description_at(u32_t pos);
	std::pair<int, int> command_manager_get_command_id_range(); /** return the command position range. Ex: (-100, 20) if we have 100 commands to undo and 20 to redo */
	void                command_manager_AddUndoHandler(Text_Editor& editor, Text_Editor::UndoRecord& undo_record);
}