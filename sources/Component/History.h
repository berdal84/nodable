#pragma once

#include <vector>
#include <ctime>
#include <string>

#include "mirror.h"
#include "ImGuiColorTextEdit/TextEditor.h"

#include "Component.h"
#include "Wire.h"

namespace Nodable
{
	class Cmd;
	class Cmd_TextEditor_InsertText;
    class History;

	/* TextEditorBuffer is a class to handle TextEditor UndoRecords
	This class will catch these object using AddUndo method.
	*/
	class TextEditorBuffer : public TextEditor::ExternalUndoBufferInterface {

	public:
	    TextEditorBuffer() = default;
	    ~TextEditorBuffer() = default;

		void AddUndo(TextEditor::UndoRecord& _undoRecord);

		void setHistory(History* _history) { history = _history; }
		void setTextEditor(TextEditor* aTextEditor) { mTextEditor = aTextEditor;}

	private:
		TextEditor* mTextEditor;
		History* history;
	};

	class History : public Component {
	public:
		History(size_t _sizeMax = 100):sizeMax(_sizeMax){}
		~History();

		/* Execute a command and add it to the history.
		If there are other commands after they will be erased from the history */
		void addAndExecute(std::unique_ptr<Cmd>);

		/* Undo the current (in the history) command  */
		void undo();

		/* Redo the next (in the history) command */
		void redo();
		
		/* clear the undo history */
		void clear();

		/* To get the size of the history (command count)*/
		size_t getSize()const { return commands.size(); }

		/* To get the current command*/
		size_t getCursorPosition()const { return commandsCursor; }
		void setCursorPosition(size_t _pos);

		std::string getCommandDescriptionAtPosition(size_t _commandId);

		/* To get the special buffer for TextEditor */
		TextEditorBuffer* createTextEditorUndoBuffer(TextEditor* _textEditor);

		bool dirty;

	private:
	    size_t sizeMax;
		std::vector<std::shared_ptr<Cmd>>	commands;		/* Command history */
		size_t commandsCursor = 0;	/* Command history cursor (zero based index) */
		std::unique_ptr<TextEditorBuffer> textEditorBuffer;

		MIRROR_CLASS(History)(
			MIRROR_PARENT(Component));
	};


	/*
		Base class for all commands
	*/

	class Cmd
	{
	public:
		Cmd(){};
		virtual ~Cmd(){};
		/* Call this to execute the command instance */
		virtual void execute() = 0;

		/* Call this to undo the execution of the command instance */
		virtual void undo() = 0;
		
		/* Call this to redo this command */
		virtual void redo() = 0;

		virtual const char* getDescription() { return description.c_str(); };
	protected:
		std::string description = "";
		bool done = false;	/* if set to true after do() has been called */		
	};


	/*
		Command to add a wire between two Members
	*/

	class Cmd_ConnectWire : public Cmd
	{
	public:
		Cmd_ConnectWire(std::shared_ptr<Member>& _source, std::shared_ptr<Member>& _target);;
		~Cmd_ConnectWire(){};
		void execute();
		void redo();
		void undo();
		std::shared_ptr<Wire> getWire() { return wire; }
	private:
		std::shared_ptr<Wire> wire;
        std::shared_ptr<Member> source;
        std::shared_ptr<Member> target;
	};

	/*
		Command to wraps a TextEditor UndoRecord
	*/

	class Cmd_TextEditor_InsertText : public Cmd
	{
	public:
		Cmd_TextEditor_InsertText(TextEditor::UndoRecord& _undoRecord, TextEditor* _textEditor);
		~Cmd_TextEditor_InsertText() = default;
		void execute() {}
		void redo();
		void undo();

	private:
		TextEditor::UndoRecord undoRecord;
		TextEditor*             textEditor;
	};
}