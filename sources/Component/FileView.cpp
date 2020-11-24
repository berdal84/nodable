#include "FileView.h"
#include "View.h"
#include "NodeView.h"
#include "File.h"
#include "Container.h"

#include <ImGuiColorTextEdit/TextEditor.h>

using namespace Nodable;

void FileView::init()
{

	/*
		Configure ImGuiTextColorEdit
	*/

	m_textEditor = new TextEditor();
	static auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	m_textEditor->SetLanguageDefinition(lang);
	m_textEditor->SetImGuiChildIgnored(true);

	TextEditor::Palette palette = { {
		0xffffffff, // None
		0xffd69c56, // Keyword  
		0xff00ff00, // Number
		0xff7070e0, // String
		0xff70a0e0, // Char literal
		0xffffffff, // Punctuation
		0xff409090, // Preprocessor
		0xffaaaaaa, // Identifier
		0xff9bc64d, // Known identifier
		0xffc040a0, // Preproc identifier
		0xff909090, // Comment (single line)
		0xff909090, // Comment (multi line)
		0x30000000, // Background
		0xffe0e0e0, // Cursor
		0x40ffffff, // Selection
		0x800020ff, // ErrorMarker
		0x40f08000, // Breakpoint
		0x88909090, // Line number
		0x40000000, // Current line fill
		0x40808080, // Current line fill (inactive)
		0x40a0a0a0, // Current line edge
		} };

	m_textEditor->SetPalette(palette);
}

bool FileView::draw()
{

	/*
		TEXT EDITOR
	*/
	auto file = getFile();

	auto availSize = ImGui::GetContentRegionAvail();

	auto previousCursorPosition = m_textEditor->GetCursorPosition();
	auto previousSelectedText = m_textEditor->GetSelectedText();
	auto previousLineText = m_textEditor->GetCurrentLineText();

	auto allowkeyboard = !NodeView::IsANodeDragged() &&
		                  NodeView::GetSelected() == nullptr; // disable keyboard for text editor when a node is selected.

	auto allowMouse = !NodeView::IsANodeDragged() &&
		              !ImGui::IsAnyItemHovered() &&
		              !ImGui::IsAnyItemFocused();

	m_textEditor->SetHandleKeyboardInputs(allowkeyboard);
	m_textEditor->SetHandleMouseInputs(allowMouse);
	m_textEditor->Render("Text Editor Plugin", availSize);

	auto currentCursorPosition = m_textEditor->GetCursorPosition();
	auto currentSelectedText = m_textEditor->GetSelectedText();
	auto currentLineText = m_textEditor->GetCurrentLineText();

	auto isCurrentLineModified = currentLineText != previousLineText;
	auto isSelectedTextModified = previousSelectedText != currentSelectedText;

	m_hasChanged = isCurrentLineModified ||
		m_textEditor->IsTextChanged() ||
		isSelectedTextModified;

	if (m_textEditor->IsTextChanged())
		file->setModified();

	if ( hasChanged() )
		file->evaluateSelectedExpression();

	/*
		NODE EDITOR
	*/
	ImGui::SetCursorPos(ImVec2(0, 0));
	auto container = file->getInnerContainer();
	auto view      = container->getComponent<View>();

	view->setVisibleRect( this->visibleRect );
	view->draw();

	return true;
}

std::string FileView::getText()const
{
	return m_textEditor->GetText();
}

void FileView::replaceSelectedText(std::string _val)
{
	auto start = m_textEditor->GetCursorPosition();

	/* If there is no selection, selects current line */
	auto hasSelection    = m_textEditor->HasSelection();
	auto selectionStart  = m_textEditor->GetSelectionStart();
	auto selectionEnd    = m_textEditor->GetSelectionEnd();

	// Select the whole line if no selection is set
	if (!hasSelection)
	{
		m_textEditor->MoveHome(false);
		m_textEditor->MoveEnd(true);
		m_textEditor->SetCursorPosition(TextEditor::Coordinates(start.mLine, 0));
	}

	/* insert text (and select it) */
	m_textEditor->InsertText(_val, true);

	LOG_MESSAGE(Log::Verbosity::Normal, "Graph serialized: %s \n", _val.c_str());
}

void FileView::setText(const std::string& _content)
{
	m_textEditor->SetText(_content);
}

std::string FileView::getSelectedText()const
{
	return m_textEditor->HasSelection() ? m_textEditor->GetSelectedText() : m_textEditor->GetCurrentLineText();
}

File* FileView::getFile() {
	return getOwner()->as<File>();
}

void FileView::setUndoBuffer(TextEditor::ExternalUndoBufferInterface* _buffer ) {
	this->m_textEditor->SetExternalUndoBuffer(_buffer);
}