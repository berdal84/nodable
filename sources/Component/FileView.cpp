#include "FileView.h"
#include "View.h"
#include "NodeView.h"
#include "File.h"
#include "GraphNode.h"
#include "GraphNodeView.h"
#include "Settings.h"

#include <ImGuiColorTextEdit/TextEditor.h>

using namespace Nodable;

void FileView::init()
{
	static auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	m_textEditor.SetLanguageDefinition(lang);
	m_textEditor.SetImGuiChildIgnored(true);
	m_textEditor.SetPalette(Settings::GetCurrent()->ui.text.textEditorPalette);
}

bool FileView::draw()
{
    auto availSize = ImGui::GetContentRegionAvail();

     // Splitter
    //---------

    if ( m_childSize1 + m_childSize2 != availSize.x )
    {
        float ratio = availSize.x / (m_childSize1 + m_childSize2);
        m_childSize1 *= ratio;
        m_childSize2 *= ratio;
    }

    ImRect rect;
    rect.Max.y = availSize.y;
    rect.Max.x = 4.0f;
    rect.TranslateX(m_childSize1 + 2.0f);
    rect.Translate(View::ToScreenPosOffset());
    ImGui::SplitterBehavior( rect, ImGui::GetID("file_splitter"), ImGuiAxis_X, &m_childSize1, &m_childSize2, 20.0f, 20.0f);

     // TEXT EDITOR
    //------------

    ImGui::BeginChild("file", ImVec2(m_childSize1, availSize.y));

    auto file = getFile();
    auto previousCursorPosition = m_textEditor.GetCursorPosition();
    auto previousSelectedText = m_textEditor.GetSelectedText();
    auto previousLineText = m_textEditor.GetCurrentLineText();

    auto allowkeyboard = !NodeView::IsAnyDragged() &&
                         NodeView::GetSelected() ==
                         nullptr; // disable keyboard for text editor when a node is selected.

    auto allowMouse = !NodeView::IsAnyDragged() &&
                      !ImGui::IsAnyItemHovered() &&
                      !ImGui::IsAnyItemFocused();

    m_textEditor.SetHandleKeyboardInputs(allowkeyboard);
    m_textEditor.SetHandleMouseInputs(allowMouse);

    m_textEditor.Render("Text Editor Plugin", ImGui::GetContentRegionAvail(), false);

    auto currentCursorPosition = m_textEditor.GetCursorPosition();
    auto currentSelectedText = m_textEditor.GetSelectedText();
    auto currentLineText = m_textEditor.GetCurrentLineText();

    auto isCurrentLineModified = currentLineText != previousLineText;
    auto isSelectedTextModified = previousSelectedText != currentSelectedText;

    m_hasChanged = isCurrentLineModified ||
                   m_textEditor.IsTextChanged() ||
                   isSelectedTextModified;

    if (m_textEditor.IsTextChanged())
        file->setModified();

    if (hasChanged()) {
        file->evaluateSelectedExpression();
    }
    ImGui::EndChild();

     // NODE EDITOR
    //-------------

    ImGui::SameLine();
    auto graphNodeView = file->getInnerGraph()->getComponent<GraphNodeView>();
    graphNodeView->update();
    graphNodeView->drawAsChild("graph", ImVec2(m_childSize2, availSize.y), false);

	return true;
}

std::string FileView::getText()const
{
	return m_textEditor.GetText();
}

void FileView::replaceSelectedText(std::string _val)
{
	auto start = m_textEditor.GetCursorPosition();

	/* If there is no selection, selects current line */
	auto hasSelection    = m_textEditor.HasSelection();
	auto selectionStart  = m_textEditor.GetSelectionStart();
	auto selectionEnd    = m_textEditor.GetSelectionEnd();

	// Select the whole line if no selection is set
	if (!hasSelection)
	{
		m_textEditor.MoveHome(false);
		m_textEditor.MoveEnd(true);
		m_textEditor.SetCursorPosition(TextEditor::Coordinates(start.mLine, 0));
	}

	/* insert text (and select it) */
	m_textEditor.InsertText(_val, true);
    auto end = m_textEditor.GetCursorPosition();
	if (!hasSelection && start.mLine == end.mLine ) // no selection and insert text is still on the same line
    {
        m_textEditor.SetSelection(selectionStart, selectionEnd);
    }

	LOG_MESSAGE( "FileView", "Graph serialized: %s \n", _val.c_str());
}

void FileView::setText(const std::string& _content)
{
	m_textEditor.SetText(_content);
}

std::string FileView::getSelectedText()const
{
	return m_textEditor.HasSelection() ? m_textEditor.GetSelectedText() : m_textEditor.GetCurrentLineText();
}

File* FileView::getFile() {
	return getOwner()->as<File>();
}

void FileView::setUndoBuffer(TextEditor::ExternalUndoBufferInterface* _buffer ) {
	this->m_textEditor.SetExternalUndoBuffer(_buffer);
}

void FileView::drawFileInfo()
{
    File* file = getFile();

    // Basic information
    ImGui::Text("Name: %s", file->getName().c_str());
    ImGui::Text("Path: %s", file->getPath().c_str());
    ImGui::NewLine();

    // Statistics
    ImGui::Text("Graph statistics:");
    ImGui::Indent();
    GraphNode* graph = file->getInnerGraph();
    ImGui::Text("Node count: %lu", graph->getNodeRegistry().size());
    ImGui::Text("Wire count: %lu", graph->getWireRegistry().size());
    ImGui::Unindent();
    ImGui::NewLine();

    // Language browser (list functions/operators)
    if (ImGui::TreeNode("Language"))
    {
        const Language* language = file->getLanguage();
        const auto& functions = language->getAllFunctions();
        const Serializer* serializer = language->getSerializer();

        ImGui::Columns(1);
        for(const auto& each_fct : functions )
        {
            std::string name;
            serializer->serialize(name, each_fct.signature);
            ImGui::Text("%s", name.c_str());
        }

        ImGui::TreePop();
    }
}
