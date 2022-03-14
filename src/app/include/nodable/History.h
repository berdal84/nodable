#pragma once

#include <deque>
#include <ctime>
#include <string>
#include <memory>

#include <ImGuiColorTextEdit/TextEditor.h>

#include <nodable/R.h>
#include <nodable/Component.h>
#include <nodable/Nodable.h>
#include <nodable/GraphNode.h>
#include <nodable/Wire.h>
#include <nodable/Member.h>
#include <nodable/Log.h>
#include <nodable/Command.h>

namespace Nodable
{
    // forward declarations
    class History;

	/* TextEditorBuffer is a class to handle TextEditor UndoRecords
	This class will catch these object using AddUndo method.
	*/
	class TextEditorBuffer : public TextEditor::IExternalUndoBuffer
    {
	public:
        void set_enable(bool _val){ m_enabled = _val; }
		void AddUndo(TextEditor::UndoRecord& _undoRecord) override;
		void set_history(History* _history) { m_history = _history; }
		void set_text_editor(TextEditor* aTextEditor) { m_Text_editor = aTextEditor;}
	private:
		TextEditor* m_Text_editor = nullptr;
		History*    m_history     = nullptr;
        bool        m_enabled     = false;
	};

	class History {
	public:
		explicit History(size_t _sizeMax = 100)
            : m_size_max(_sizeMax)
            , m_dirty(false)
            , m_commands_cursor(0)
        {}
		~History();

		/** Execute a command and add it to the history.
		If there are other commands after they will be erased from the history */
		void push_back_and_execute(std::shared_ptr<ICommand>);

        void enable_text_editor(bool _val) { m_text_editor_buffer.set_enable(_val); }

		/** Undo the current command  */
		void undo();

		/** Redo the next command */
		void redo();
		
		/** clear the undo history */
		void clear();

		/** To get the size of the history (command count)*/
		size_t get_size()const { return m_commands.size(); }

		/** To get the current command*/
		size_t get_cursor_pos()const { return m_commands_cursor; }
		void   set_cursor_pos(size_t _pos);

		std::string get_cmd_description_at(size_t _commandId);

		/** To get the special buffer for TextEditor */
		TextEditorBuffer* configure_text_editor_undo_buffer(TextEditor* _textEditor) {
            m_text_editor_buffer.set_text_editor(_textEditor);
            m_text_editor_buffer.set_history(this);
            m_text_editor_buffer.set_enable(true);
			return &m_text_editor_buffer;
		}

        bool is_dirty() const { return m_dirty; }
        void set_dirty(bool _dirty = true) { m_dirty = _dirty; }

		// Future: For command groups (ex: 5 commands that are atomic)
		// static BeginGroup();
		// static EndGroup()

	private:
        bool                m_dirty;
	    size_t              m_size_max;
		size_t           	m_commands_cursor;	/* Command history cursor (zero based index) */
		TextEditorBuffer    m_text_editor_buffer;
		std::deque<std::shared_ptr<ICommand>> m_commands;		/* Command history */
	};
}