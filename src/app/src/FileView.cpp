#include <nodable/app/FileView.h>

#include <nodable/app/NodeView.h>
#include <nodable/app/File.h>
#include <nodable/core/GraphNode.h>
#include <nodable/app/GraphNodeView.h>
#include <nodable/app/Settings.h>
#include <nodable/core/Node.h>
#include <nodable/app/AppContext.h>
#include <nodable/core/VM.h>

using namespace Nodable;

FileView::FileView(AppContext* _ctx, File *_file)
        : View(_ctx)
        , m_textEditor()
        , m_hasChanged(false)
        , m_file(_file)
{
    m_observer.observe(_file->m_on_graph_changed_evt, [](GraphNode* _graph)
    {
        if ( !_graph->is_empty() )
        {
            Node* root = _graph->get_root();

            NodeView* root_node_view = root->get<NodeView>();
            View*     graph_view   = root->get_parent_graph()->get<GraphNodeView>();

            if ( root_node_view && graph_view )
            {
                ImRect graphViewRect = graph_view->getVisibleRect();
                vec2 newPos = graphViewRect.GetTL();
                newPos.x += graphViewRect.GetSize().x * 0.33f;
                newPos.y += root_node_view->getSize().y;
                root_node_view->setPosition(newPos );
            }
        }
    });
}

void FileView::init()
{
	static auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	m_textEditor.SetLanguageDefinition(lang);
	m_textEditor.SetImGuiChildIgnored(true);
	m_textEditor.SetPalette(m_file->get_context()->settings->ui_text_textEditorPalette);
}

bool FileView::draw()
{
    const vec2 margin(10.0f, 0.0f);
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

    ImGui::BeginChild("file", vec2(m_childSize1, availSize.y), false);

    auto previousCursorPosition = m_textEditor.GetCursorPosition();
    auto previousSelectedText = m_textEditor.GetSelectedText();
    auto previousLineText = m_textEditor.GetCurrentLineText();

    bool is_vm_running = m_context->vm->is_program_running();
    auto allowkeyboard = !is_vm_running &&
                         !NodeView::IsAnyDragged() &&
                         !NodeView::GetSelected(); // disable keyboard for text editor when a node is selected.

    auto allowMouse = !is_vm_running &&
                       !NodeView::IsAnyDragged() &&
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


    m_file->getHistory()->enable_text_editor(true); // ensure to begin to record history
    m_textEditor.Render("Text Editor Plugin", ImGui::GetContentRegionAvail());
    if ( m_context->settings->experimental_hybrid_history )
    {
        m_file->getHistory()->enable_text_editor(false); // avoid recording events caused by graph serialisation
    }

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
        m_file->update_graph();
    }

    ImGui::EndChild();

     // NODE EDITOR
    //-------------

    ImGui::SameLine();
    GraphNode* graph = m_file->getGraph();
    NODABLE_ASSERT(graph);
    auto graph_node_view = graph->get<GraphNodeView>();

    if ( graph_node_view )
    {
        graph_node_view->update();
        auto flags = (ImGuiWindowFlags_)(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        bool changed = graph_node_view->drawAsChild("graph", vec2(m_childSize2, availSize.y), false, flags);
    }
    else
    {
        ImGui::TextColored(ImColor(255,0,0), "ERROR: Unable to graw Graph View");
        LOG_ERROR("FileView", "graphNodeView is null\n");
    }

	return m_hasChanged;
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
        LOG_MESSAGE("FileView", "Selected text updated from graph.\n")
        LOG_VERBOSE("FileView", "%s \n", _val.c_str())
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

void FileView::setUndoBuffer(TextEditor::IExternalUndoBuffer* _buffer ) {
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
    ImGui::Text("Node count: %lu", m_file->getGraph()->get_node_registry().size());
    ImGui::Text("Wire count: %lu", m_file->getGraph()->get_wire_registry().size());
    ImGui::Unindent();
    ImGui::NewLine();

    // Language browser (list functions/operators)
    if (ImGui::TreeNode("Language"))
    {
        const auto&       functions  = m_context->language->getAllFunctions();
        const Serializer* serializer = m_context->language->getSerializer();

        ImGui::Columns(1);
        for(const auto& each_fct : functions )
        {
            std::string name;
            serializer->serialize(name, each_fct->get_signature());
            ImGui::Text("%s", name.c_str());
        }

        ImGui::TreePop();
    }
}

void  FileView::experimental_clipboard_auto_paste(bool _enable)
{
    m_experimental_clipboard_auto_paste = _enable;
    if( _enable )
    {
        m_experimental_clipboard_prev = "";
    }
}