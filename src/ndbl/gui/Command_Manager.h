#pragma once

#include <deque>
#include <ctime>
#include <string>
#include "ImGuiColorTextEdit/TextEditor.h"
#include "ndbl/gui/Command.h"

namespace ndbl
{
    // forward declarations
    class Command_Manager;

	/* TextEditorBuffer is a class to handle TextEditor UndoRecords
	This class will catch these object using AddUndo method.
	*/
	class Text_Editor_Undo_Buffer : public TextEditor::IExternalUndoBuffer
    {
	public:
		void AddUndo(TextEditor::UndoRecord&) override;

		TextEditor*      text_editor     = nullptr;
		Command_Manager* command_manager = nullptr;
        bool             enabled         = false;
	};

    /**
     * The history is responsible for undo/redo commands.
     * It has_flags two containers to store past/future commands
     *     (past)    (now)    (future)
     * |oooooooooooooo|-------------------|
     */
	struct Command_Manager
	{
        using Commands = std::deque<Command>;

        bool 			  		is_dirty = false;
		Command_Flags           next_command_flags;
		Text_Editor_Undo_Buffer text_editor_undo_buffer;
		Commands          		past;
		Commands          		future;
    };

	Command_Manager* command_manager_init();
	void             command_manager_shutdown();
	Command_Manager* command_manager();

	/**
	* Push a command and execute it.
	* In some cases the command may not be added to the history or executed, check definition.
	* @param _from_text_editor should not be set except if command comes from TextEditor.
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
	void                command_manager_move_cursor(int _pos); /** Move time cursor to past (negative value) or future (positive value). */
	std::string         command_manager_get_cmd_description_at(int _cmd_position);
	Text_Editor_Undo_Buffer*  command_manager_configure_text_editor_undo_buffer(TextEditor* _textEditor); /** To get the special buffer for TextEditor */
	std::pair<int, int> command_manager_get_command_id_range(); /** return the command position range. Ex: (-100, 20) if we have 100 commands to undo and 20 to redo */

}