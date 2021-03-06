#include <nodable/FileView.h>

#include <nodable/NodeView.h>
#include <nodable/File.h>
#include <nodable/GraphNode.h>
#include <nodable/GraphNodeView.h>
#include <nodable/Settings.h>
#include <nodable/ProgramNode.h>

using namespace Nodable;

FileView::FileView(File *_file)
        : m_textEditor()
        , m_hasChanged(false)
        , m_file(_file)
{
    m_observer.observe( _file->m_onExpressionParsedIntoGraph, [](ProgramNode* program)
    {
        if ( program )
        {
            NodeView* programView = program->getComponent<NodeView>();
            NodeView* graphView   = program->getParentGraph()->getComponent<NodeView>();
            if ( programView )
            {
                if ( graphView )
                {
                    auto graphViewRect = graphView->getVisibleRect();
                    auto newPos = graphViewRect.GetTL();
                    newPos.x += graphViewRect.GetSize().x * 0.33f;
                    newPos.y += programView->getSize().y;
                    programView->setPosition( newPos );
                }

            }
        }
    });
}

void FileView::init()
{
	static auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	m_textEditor.SetLanguageDefinition(lang);
	m_textEditor.SetImGuiChildIgnored(true);
	m_textEditor.SetPalette(Settings::GetCurrent()->ui.text.textEditorPalette);
}

bool FileView::draw()
{
    const ImVec2 margin(10.0f, 0.0f);
    auto availSize = ImGui::GetContentRegionAvail() - margin;

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
    rect.Translate(ImGuiEx::ToScreenPosOffset());
    ImGui::SplitterBehavior( rect, ImGui::GetID("file_splitter"), ImGuiAxis_X, &m_childSize1, &m_childSize2, 20.0f, 20.0f);

     // TEXT EDITOR
    //------------

    ImGui::BeginChild("file", ImVec2(m_childSize1, availSize.y), false);

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

    // listen to clipboard in background (disable by default)
    if (m_experimental_clipboard_auto_paste)
    {
        m_experimental_clipboard_curr = ImGui::GetClipboardText();
        if (!m_experimental_clipboard_curr.empty() && m_experimental_clipboard_curr != m_experimental_clipboard_prev)
        {
            if ( !m_experimental_clipboard_prev.empty() )
                m_textEditor.InsertText(m_experimental_clipboard_curr.c_str(), true);
            m_experimental_clipboard_prev = std::move(m_experimental_clipboard_curr);
        }
    }

    m_textEditor.Render("Text Editor Plugin", ImGui::GetContentRegionAvail());

    auto currentCursorPosition = m_textEditor.GetCursorPosition();
    auto currentSelectedText = m_textEditor.GetSelectedText();
    auto currentLineText = m_textEditor.GetCurrentLineText();

    auto isCurrentLineModified = currentLineText != previousLineText;
    auto isSelectedTextModified = previousSelectedText != currentSelectedText;

    m_hasChanged = isCurrentLineModified ||
                   m_textEditor.IsTextChanged() ||
                   isSelectedTextModified;

    if (m_textEditor.IsTextChanged())
        m_file->setModified();

    if (hasChanged()) {
        m_file->evaluateSelectedExpression();
    }

    ImGui::EndChild();

     // NODE EDITOR
    //-------------

    ImGui::SameLine();
    GraphNode* graph = m_file->getGraph();
    NODABLE_ASSERT(graph != nullptr);
    NodeView* graphNodeView = graph->getComponent<GraphNodeView>();
    NODABLE_ASSERT(graphNodeView != nullptr);
    graphNodeView->update();
    auto flags = (ImGuiWindowFlags_)(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    graphNodeView->drawAsChild("graph", ImVec2(m_childSize2, availSize.y), false, flags);

	return true;
}

std::string FileView::getText()const
{
	return m_textEditor.GetText();
}

void FileView::replaceSelectedText(const std::string &_val)
{
    if ( getSelectedText() != _val )
    {
        auto start = m_textEditor.GetCursorPosition();

        /* If there is no selection, selects current line */
        auto hasSelection = m_textEditor.HasSelection();
        auto selectionStart = m_textEditor.GetSelectionStart();
        auto selectionEnd = m_textEditor.GetSelectionEnd();

        // Select the whole line if no selection is set
        if (!hasSelection) {
            m_textEditor.MoveHome(false);
            m_textEditor.MoveEnd(true);
            m_textEditor.SetCursorPosition(TextEditor::Coordinates(start.mLine, 0));
        }

        /* insert text (and select it) */
        m_textEditor.InsertText(_val, true);

        auto end = m_textEditor.GetCursorPosition();
        if (!hasSelection && start.mLine == end.mLine) // no selection and insert text is still on the same line
        {
            m_textEditor.SetSelection(selectionStart, selectionEnd);
        }
        LOG_MESSAGE("FileView", "Selected text updated from graph.\n");
        LOG_VERBOSE("FileView", "%s \n", _val.c_str());
    }
}

void FileView::setText(const std::string& _content)
{
	m_textEditor.SetText(_content);
}

std::string FileView::getSelectedText()const
{
	return m_textEditor.HasSelection() ? m_textEditor.GetSelectedText() : m_textEditor.GetCurrentLineText();
}

void FileView::setUndoBuffer(TextEditor::ExternalUndoBufferInterface* _buffer ) {
	this->m_textEditor.SetExternalUndoBuffer(_buffer);
}

void FileView::drawFileInfo() const
{
    // Basic information
    ImGui::Text("Name: %s", m_file->getName().c_str());
    ImGui::Text("Path: %s", m_file->getPath().c_str());
    ImGui::NewLine();

    // Statistics
    ImGui::Text("Graph statistics:");
    ImGui::Indent();
    ImGui::Text("Node count: %u", m_file->getGraph()->getNodeRegistry().size());
    ImGui::Text("Wire count: %u", m_file->getGraph()->getWireRegistry().size());
    ImGui::Unindent();
    ImGui::NewLine();

    // Language browser (list functions/operators)
    if (ImGui::TreeNode("Language"))
    {
        const Language* language = m_file->getLanguage();
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
