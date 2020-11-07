#pragma once

// std
#include <vector>
#include <ctime>
#include <string>

// extern
#include "mirror.h"
#include "ImGuiColorTextEdit/TextEditor.h"

// Nodable
#include "Component.h" // base class
#include "Nodable.h"
#include "Container.h"
#include "Wire.h"
#include "WireView.h"
#include "Member.h"
#include "Log.h"
#include "NodeTraversal.h"

namespace Nodable
{
	class Cmd;
	class Cmd_TextEditor_InsertText;

	/* TextEditorBuffer is a class to handle TextEditor UndoRecords
	This class will catch these object using AddUndo method.
	*/
	class TextEditorBuffer : public TextEditor::ExternalUndoBufferInterface {

	public:
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
		TextEditorBuffer* createTextEditorUndoBuffer(TextEditor* _textEditor) {

			textEditorBuffer = std::make_unique<TextEditorBuffer>();
			textEditorBuffer->setTextEditor(_textEditor);
			textEditorBuffer->setHistory(this);

			return textEditorBuffer.get();
		}

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
		Cmd_ConnectWire(Member* _source, Member* _target)
		{			
			this->source     = _source;
			this->target     = _target;

			// Title
			description.append("Connect Wire\n");

			// Details
			description.append( "\"" + _source->getName() + "\" ---> \"" + _target->getName() + "\"\n");

			// Time
			std::time_t time = std::time(nullptr);
			auto localTime   = std::localtime(&time);
			std::string timeString( std::asctime(localTime) );
			description.append(timeString);

		};

		~Cmd_ConnectWire(){};

		void execute()
		{
			target->setInputMember(source);
			auto targetNode = target->getOwner()->as<Node>();
			auto sourceNode = source->getOwner()->as<Node>();

			// Link wire to members
			auto sourceContainer = sourceNode->getParentContainer();

			if (sourceContainer != nullptr)
				this->wire = sourceContainer->newWire();
			else
				this->wire = std::make_shared<Wire>();

			wire->setSource(source);
			wire->setTarget(target);

			// Add the wire pointer to the Entitis instance to speed up drawing process.
			targetNode->addWire(wire);
			sourceNode->addWire(wire);

			NodeTraversal::SetDirty(targetNode);
		}

		void redo() {
			execute();
		}

		void undo()
		{
            auto targetNode = target->getOwner()->as<Node>();
            auto sourceNode = source->getOwner()->as<Node>();

            target->setInputMember(nullptr);
            NodeTraversal::SetDirty(targetNode);

			// Link Members
			wire->setSource(nullptr);
			wire->setTarget(nullptr);

			// Add the wire pointer to the Node instance to speed up drawing process.
            targetNode->removeWire(wire);
            sourceNode->removeWire(wire);
		}

		std::shared_ptr<Wire> getWire() { return wire; }
	private:
		std::shared_ptr<Wire> wire;
		Member*    source        = nullptr;
		Member*    target        = nullptr;
	};



	/*
		Command to wraps a TextEditor UndoRecord
	*/

	class Cmd_TextEditor_InsertText : public Cmd
	{
	public:
		Cmd_TextEditor_InsertText(
			TextEditor::UndoRecord& _undoRecord,
			TextEditor* _textEditor): 
			undoRecord(_undoRecord),
			textEditor(_textEditor)
		{
			this->description.append("Cmd_TextEditor_InsertText\n" );
			this->description.append("removed : " + undoRecord.mRemoved + "\n");
			this->description.append("added : " + undoRecord.mAdded + "\n");
		}

		~Cmd_TextEditor_InsertText() {}

		void execute() {}


		void redo() {
			undoRecord.Redo(textEditor);
		}

		void undo()
		{
			undoRecord.Undo(textEditor);
		}

	private:


		TextEditor::UndoRecord undoRecord;
		TextEditor*             textEditor;
	};
}