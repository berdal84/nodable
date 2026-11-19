#pragma once

#include <ctime>

#include "bdc/Array.hpp"
#include "bdc/String.hpp"
#include "Command.h"
#include "ndbl/texteditor/Text_Editor.h"

namespace ndbl
{
    /**
     * The history is responsible for undo/redo commands.
     * It has a single commands container, with a cursor to identify the current history position.
     *     (past) (cursor) (future)
     * |oooooooooooooo|---------------|
     */
	struct Command_Manager
	{
        bool 			  			is_dirty = false;
		bool                    	push_text_editor_AddUndoRecord = true;
		Cmd_Flags           	    next_command_flags;
		u32_t                       cursor; // ex: if cursor is 5, it means we have 5 commands in the past ("c c c c c <cursor> c ...")
		bdc::Resizable_Array<Command>commands;
    };

	Command_Manager* 	command_manager_init();
	void             	command_manager_shutdown();
	Command_Manager* 	command_manager();
	void    			command_manager_enable_text_editor_undo_buffer(bool _val);
	void    			command_manager_undo();
	void    			command_manager_redo();
	void    			command_manager_clear();
	void                command_manager_set_cursor(u32_t pos); /** undo/redo automatically to set the cursor at a given position */
	bdc::String         command_manager_get_cmd_description_at(u32_t pos);
	void                command_manager_handle_undo_record(Text_Editor& editor, Text_Editor::UndoRecord& undo_record);

	Command*  			push_command(const Command&, const Cmd_Push_Params& = { .skip_command_do_at_push = false }); // set skip_command_execution = true in case command was already executed. When I write these lines, the text editor has its own undo/redo system, and Nodable get the UndoRecord after it has been executed, we don't want to run it twice.
	void 				begin_command_group();
	void 				end_command_group();
}