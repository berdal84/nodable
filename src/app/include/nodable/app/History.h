#pragma once

#include <deque>
#include <ctime>
#include <string>
#include <nodable/core/memory.h>

#include <ImGuiColorTextEdit/TextEditor.h>

#include <nodable/core/reflection/reflection>
#include <nodable/core/Component.h>
#include <nodable/app/types.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/Wire.h>
#include <nodable/core/Member.h>
#include <nodable/core/Log.h>
#include <nodable/app/Command.h>

namespace ndbl
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

    /**
     * The history is responsible for undo/redo commands.
     * It has two containers to store past/future commands
     *     (past)    (now)    (future)
     * |oooooooooooooo|-------------------|
     */
	class History {
	public:
		explicit History(const bool* _experimental_hybrid_history, size_t _sizeMax = 100)
            : m_size_max(_sizeMax)
            , m_dirty(false)
            , m_experimental_hybrid_history(_experimental_hybrid_history)
        {}
		~History();

		/**
		 * Push a command and execute it.
		 * In some cases the command may not be added to the history or executed, check definition.
		 * @param _from_text_editor should not be set except if command comes from TextEditor.
		 *                          This flag is here to handle legacy history mode (text based) and
		 *                          hybrid mode (Text/Graph).
		 */
		void push_command(s_ptr<ICommand>, bool _from_text_editor = false);

        void enable_text_editor(bool _val) { m_text_editor_buffer.set_enable(_val); }

		/** Undo the current command  */
		void undo();

		/** Redo the next command */
		void redo();
		
		/** clear the undo history */
		void clear();

		/** To get the size of the history (command count) */
		size_t get_size()const { return m_past.size() + m_future.size(); }

        /** Move time cursor to past (negative value) or future (positive value). */
		void   move_cursor(int _pos);

		std::string get_cmd_description_at(int _cmd_position);

		/** To get the special buffer for TextEditor */
		TextEditorBuffer* configure_text_editor_undo_buffer(TextEditor* _textEditor) {
            m_text_editor_buffer.set_text_editor(_textEditor);
            m_text_editor_buffer.set_history(this);
            m_text_editor_buffer.set_enable(true);
			return &m_text_editor_buffer;
		}

        bool is_dirty() const { return m_dirty; }
        void set_dirty(bool _dirty = true) { m_dirty = _dirty; }

        /** return the command position range.
         * ex: (-100, 20) if we have 100 commands to undo and 20 to redo */
        std::pair<int, int>  get_command_id_range();
    private:
        bool                m_dirty;
	    size_t              m_size_max;
		TextEditorBuffer    m_text_editor_buffer;
        const bool*         m_experimental_hybrid_history;
		std::deque<s_ptr<ICommand>> m_past;
		std::deque<s_ptr<ICommand>> m_future;
    };
}