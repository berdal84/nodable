#pragma once

#include <vector>
#include <ctime>
#include <string>
#include <nodable/R.h>
#include <ImGuiColorTextEdit/TextEditor.h>

#include <nodable/Component.h> // base class
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
		void AddUndo(TextEditor::UndoRecord& _undoRecord) override;
		void setHistory(History* _history) { m_history = _history; }
		void setTextEditor(TextEditor* aTextEditor) { m_Text_editor = aTextEditor;}
	private:
		TextEditor* m_Text_editor = nullptr;
		History*    m_history     = nullptr;
	};

	class History {
	public:
		explicit History(size_t _sizeMax = 100): m_size_max(_sizeMax), m_dirty(false), m_commands_cursor(0) {}
		~History();

		/** Execute a command and add it to the history.
		If there are other commands after they will be erased from the history */
		void addAndExecute(ICommand*);

		/** Undo the current command  */
		void undo();

		/** Redo the next command */
		void redo();
		
		/** clear the undo history */
		void clear();

		/** To get the size of the history (command count)*/
		size_t getSize()const { return m_commands.size(); }

		/** To get the current command*/
		size_t getCursorPosition()const { return m_commands_cursor; }
		void setCursorPosition(size_t _pos);

		std::string getCommandDescriptionAtPosition(size_t _commandId);

		/** To get the special buffer for TextEditor */
		TextEditorBuffer* getUndoBuffer(TextEditor* _textEditor) {
			m_text_editor_buffer.setTextEditor(_textEditor);
			m_text_editor_buffer.setHistory(this);
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
		std::vector<ICommand*>	m_commands;		/* Command history */
		size_t           	m_commands_cursor;	/* Command history cursor (zero based index) */
		TextEditorBuffer    m_text_editor_buffer;
	};
}