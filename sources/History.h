#pragma once
#include "Component.h" // base class
#include "Nodable.h"
#include "Container.h"
#include "Wire.h"
#include "WireView.h"
#include "Member.h"
#include <vector>
#include <time.h>
#include "ImGuiColorTextEdit/TextEditor.h"
#include "Log.h"

namespace Nodable
{
	class Cmd;
	class Cmd_TextEditor;

	class TextEditorBuffer : public TextEditor::ExternalUndoBufferInterface {

	public:
		void setHistory(History* _history) {
			history = _history;
		}

		void AddUndo(TextEditor::UndoRecord& _undoRecord, TextEditor& _textEditor);

	private: History* history;
	};

	class History : public Component {
	public:
		COMPONENT_CONSTRUCTOR(History);
		~History();

		/* Execute a command and add it to the history.
		If there are other commands after they will be erased from the history */
		void addAndExecute(Cmd*);

		/* Undo the current (in the history) command  */
		void undo();

		/* Redo the next (in the history) command */
		void redo();
		
		/* clear the undo history */
		void clear();

		/* To get the size of the history (command count)*/
		int getSize() { return commands.size(); }

		/* To get the current command*/
		int getCursorPosition() { return commandsCursor; }
		void setCursorPosition(int _pos);

		const char* getCommandDescriptionAtPosition(size_t _commandId);

		/* To get the special buffer for TextEditor */
		TextEditorBuffer* getTextEditorUndoBuffer() {

			if (textEditorBuffer == nullptr) {
				textEditorBuffer = new TextEditorBuffer();
				textEditorBuffer->setHistory(this);
			}

			return textEditorBuffer;

		}

		// Future: For command groups (ex: 5 commands that are atomic)
		// static BeginGroup();
		// static EndGroup()

		static History*     global;
	private:
		std::vector<Cmd*>	commands = std::vector<Cmd*>();		/* Command history */
		size_t           	commandsCursor = 0;	/* Command history cursor (zero based index) */
		TextEditorBuffer* textEditorBuffer = nullptr;
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
		Cmd_ConnectWire(Wire* _wire, Member* _source, Member* _target)
		{
			this->wire		 = _wire;
			this->source     = _source;
			this->target     = _target;

			// Generate a text description

				// Title
				description.append("Connect Wire\n");

				// Details
				description.append( "\"" + _source->getName() + "\" ---> \"" + _target->getName() + "\"\n");

				// Time
				time_t t = time(NULL);
				struct tm tm;
				char timeAsStringBuffer[26];
				localtime_s(&tm, &t);
				asctime_s(timeAsStringBuffer, sizeof timeAsStringBuffer, &tm);
				description.append(timeAsStringBuffer);

		};

		~Cmd_ConnectWire(){};

		void execute()
		{
			target->setInputMember(source);
			target->getOwner()->getAs<Entity*>()->setDirty();

			// Link wire to members
			wire->setSource(source);
			wire->setTarget(target);

			// Add the wire pointer to the Entitis instance to speed up drawing process.
			target->getOwner()->getAs<Entity*>()->addWire(wire);
			source->getOwner()->getAs<Entity*>()->addWire(wire);
		}

		void redo() {
			execute();
		}

		void undo()
		{
			target->setInputMember(nullptr);
			target->getOwner()->getAs<Entity*>()->setDirty();

			// Link Members
			wire->setSource(nullptr);
			wire->setTarget(nullptr);

			// Add the wire pointer to the Entity instance to speed up drawing process.
			target->getOwner()->getAs<Entity*>()->removeWire(wire);
			source->getOwner()->getAs<Entity*>()->removeWire(wire);
		}

	private:
		Wire*      wire          = nullptr;
		Member*    source        = nullptr;
		Member*    target        = nullptr;
	};



	/*
		Command to wraps a TextEditor UndoRecord
	*/

	class Cmd_TextEditor : public Cmd
	{
	public:
		Cmd_TextEditor(
			TextEditor::UndoRecord& _undoRecord,
			TextEditor& _textEditor): 
			undoRecord(_undoRecord),
			textEditor(_textEditor)
		{
		}

		~Cmd_TextEditor() {};

		void execute() {}

		void redo() {
			undoRecord.Redo(&textEditor);
		}

		void undo()
		{
			undoRecord.Undo(&textEditor);
		}

	private:
		TextEditor::UndoRecord undoRecord;
		TextEditor&             textEditor;
	};
}