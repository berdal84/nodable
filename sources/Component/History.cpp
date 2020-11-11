#include "History.h"
#include "Node.h"
#include "Container.h"

using namespace Nodable;

History::~History()
{
	commands.clear();
}

void History::addAndExecute( std::unique_ptr<Cmd> _cmd)
{	
	/* First clear commands after the cursor */
	while (commandsCursor < commands.size())
	{
        commands.pop_back();
    }

	/* Then execute and store cmd */
    _cmd->execute();
	commands.push_back( std::move(_cmd) );
	commandsCursor = commands.size();


	/* Delete command history in excess */
    while (commands.size() > sizeMax)
    {
        commands.erase(commands.begin());
        commandsCursor--;
    }
}

void History::undo()
{
	if (commandsCursor > 0)
	{
		commandsCursor--;
		commands.at(commandsCursor)->undo();
		dirty = true;
	}
}

void History::redo()
{
	if (commandsCursor < commands.size())
	{
		commands.at(commandsCursor)->redo();
		commandsCursor++;
		dirty = true;
	}
}

void Nodable::History::clear()
{
	commands.clear();
	commandsCursor = 0;
}

void History::setCursorPosition(size_t _pos)
{
	/* Do nothing if cursor is already well positioned */
	if (_pos == commandsCursor )
		return;

	/* Undo or redo the required times to get the command cursor well positioned */
	while (_pos != commandsCursor)
	{
		if (_pos > commandsCursor)
			redo();
		else
			undo();
	}

	dirty = true;
}

std::string Nodable::History::getCommandDescriptionAtPosition(size_t _commandId)
{
	const auto headId = commands.size();
	
	std::string result;

	if ( _commandId < headId )
	{
		result = commands.at(_commandId)->getDescription();
	}
	else
    {
		result = "History HEAD";
	}	

	return result;
}

TextEditorBuffer *History::createTextEditorUndoBuffer(TextEditor *_textEditor) {

    textEditorBuffer = std::make_unique<TextEditorBuffer>();
    textEditorBuffer->setTextEditor(_textEditor);
    textEditorBuffer->setHistory(this);

    return textEditorBuffer.get();
}

void TextEditorBuffer::AddUndo(TextEditor::UndoRecord& _undoRecord) {

	auto cmd = std::make_unique<Cmd_TextEditor_InsertText>(_undoRecord, mTextEditor);
	history->addAndExecute( std::move(cmd) );
}

Cmd_ConnectWire::Cmd_ConnectWire(std::shared_ptr<Member>& _source, std::shared_ptr<Member>& _target) {
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

}

void Cmd_ConnectWire::execute() {
    target->setInputConnectedMember(source);
    auto targetNode = std::dynamic_pointer_cast<Node>( target->getOwner() );
    auto sourceNode = std::dynamic_pointer_cast<Node>( source->getOwner() );

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

void Cmd_ConnectWire::redo() {
    execute();
}

void Cmd_ConnectWire::undo() {
    auto targetNode = std::dynamic_pointer_cast<Node>( target->getOwner() );
    auto sourceNode = std::dynamic_pointer_cast<Node>( source->getOwner() );

    target->resetInputConnectedMember();
    NodeTraversal::SetDirty(targetNode);

    // Link Members
    wire->getSource().reset();
    wire->getTarget().reset();

    // Add the wire pointer to the Node instance to speed up drawing process.
    targetNode->removeWire(wire);
    sourceNode->removeWire(wire);
}

Cmd_TextEditor_InsertText::Cmd_TextEditor_InsertText(TextEditor::UndoRecord &_undoRecord, TextEditor *_textEditor) :
        undoRecord(_undoRecord),
        textEditor(_textEditor)
{
    this->description.append("Cmd_TextEditor_InsertText\n" );
    this->description.append("removed : " + undoRecord.mRemoved + "\n");
    this->description.append("added : " + undoRecord.mAdded + "\n");
}

void Cmd_TextEditor_InsertText::redo() {
    undoRecord.Redo(textEditor);
}

void Cmd_TextEditor_InsertText::undo() {
    undoRecord.Undo(textEditor);
}
