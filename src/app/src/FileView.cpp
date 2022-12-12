#include <nodable/app/FileView.h>

#include <nodable/app/File.h>
#include <nodable/app/GraphNodeView.h>
#include <nodable/app/IAppCtx.h>
#include <nodable/app/NodeView.h>
#include <nodable/app/Settings.h>

#include <nodable/core/GraphNode.h>
#include <nodable/core/ISerializer.h>
#include <nodable/core/Node.h>
#include <nodable/core/VirtualMachine.h>
#include "nodable/app/commands/Cmd_WrappedTextEditorUndoRecord.h"
#include "nodable/app/commands/Cmd_ReplaceText.h"

using namespace ndbl;

FileView::FileView(IAppCtx& _ctx, File& _file)
    : View(_ctx)
    , m_text_editor()
    , m_text_has_changed(false)
    , m_file(_file)
    , m_child1_size(0.3f)
    , m_child2_size(0.7f)
    , m_experimental_clipboard_auto_paste(false)
{
    m_graph_change_obs.observe(_file.m_on_graph_changed_evt, [](GraphNode* _graph)
    {
        LOG_VERBOSE("FileView", "graph changed evt received\n")
        if ( !_graph->is_empty() )
        {
            LOG_VERBOSE("FileView", "graph is not empty\n")
            Node* root = _graph->get_root();

            NodeView* root_node_view = root->get<NodeView>();
            View*     graph_view   = root->get_parent_graph()->get<GraphNodeView>();

            if ( root_node_view && graph_view )
            {
                LOG_VERBOSE("FileView", "constraint root node view to be visible\n")
                ImRect graphViewRect = graph_view->get_visible_rect();
                vec2 newPos = graphViewRect.GetTL();
                newPos.x += graphViewRect.GetSize().x * 0.33f;
                newPos.y += root_node_view->get_size().y;
                root_node_view->set_position(newPos);
            }
        }
    });
}

void FileView::init()
{
	static auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	m_text_editor.SetLanguageDefinition(lang);
	m_text_editor.SetImGuiChildIgnored(true);
	m_text_editor.SetPalette(m_ctx.settings().ui_text_textEditorPalette);
}

bool FileView::draw()
{
    const vec2 margin(10.0f, 0.0f);
    auto availSize = ImGui::GetContentRegionAvail() - margin;

     // Splitter
    //---------

    if (m_child1_size + m_child2_size != availSize.x )
    {
        float ratio = availSize.x / (m_child1_size + m_child2_size);
        m_child1_size *= ratio;
        m_child2_size *= ratio;
    }

    ImRect rect;
    rect.Max.y = availSize.y;
    rect.Max.x = 4.0f;
    rect.TranslateX(m_child1_size + 2.0f);
    rect.Translate(ImGuiEx::ToScreenPosOffset());
    ImGui::SplitterBehavior(rect, ImGui::GetID("file_splitter"), ImGuiAxis_X, &m_child1_size, &m_child2_size, 20.0f, 20.0f);

     // TEXT EDITOR
    //------------

    ImGui::BeginChild("file", vec2(m_child1_size, availSize.y), false);

    auto previousCursorPosition = m_text_editor.GetCursorPosition();
    auto previousSelectedText = m_text_editor.GetSelectedText();
    auto previousLineText = m_text_editor.GetCurrentLineText();

    bool is_running = m_ctx.virtual_machine().is_program_running();
    auto allowkeyboard = !is_running &&
                         !NodeView::is_any_dragged() &&
                         !NodeView::get_selected(); // disable keyboard for text editor when a node is selected.

    auto allowMouse = !is_running &&
                      !NodeView::is_any_dragged() &&
                      !ImGui::IsAnyItemHovered() &&
                      !ImGui::IsAnyItemFocused();

    m_text_editor.SetHandleKeyboardInputs(allowkeyboard);
    m_text_editor.SetHandleMouseInputs(allowMouse);

    // listen to clipboard in background (disable by default)
    if (m_experimental_clipboard_auto_paste)
    {
        m_experimental_clipboard_curr = ImGui::GetClipboardText();
        if (!m_experimental_clipboard_curr.empty() && m_experimental_clipboard_curr != m_experimental_clipboard_prev)
        {
            if ( !m_experimental_clipboard_prev.empty() )
                m_text_editor.InsertText(m_experimental_clipboard_curr.c_str(), true);
            m_experimental_clipboard_prev = std::move(m_experimental_clipboard_curr);
        }
    }


    m_file.get_history()->enable_text_editor(true); // ensure to begin to record history
    m_text_editor.Render("Text Editor Plugin", ImGui::GetContentRegionAvail());
    if (m_ctx.settings().experimental_hybrid_history )
    {
        m_file.get_history()->enable_text_editor(false); // avoid recording events caused by graph serialisation
    }

    auto currentCursorPosition = m_text_editor.GetCursorPosition();
    auto currentSelectedText = m_text_editor.GetSelectedText();
    auto currentLineText = m_text_editor.GetCurrentLineText();

    auto isCurrentLineModified = currentLineText != previousLineText &&
                                 currentCursorPosition.mLine == previousCursorPosition.mLine;
    auto isSelectedTextModified = previousSelectedText != currentSelectedText;

    m_text_has_changed = isCurrentLineModified ||
                         m_text_editor.IsTextChanged() /* ||
                         isSelectedTextModified */;

    if (m_text_editor.IsTextChanged())
    {
        m_file.set_changed_flag();
    }

    if ( m_text_has_changed )
    {
        m_file.update_graph();
    }

    ImGui::EndChild();

     // NODE EDITOR
    //-------------

    GraphNode* graph = m_file.get_graph();
    NODABLE_ASSERT(graph);
    auto graph_node_view = graph->get<GraphNodeView>();
    ImGui::SameLine();
    if ( graph_node_view )
    {
        LOG_VERBOSE("FileView", "graph_node_view->update()\n");
        ImGuiWindowFlags flags = (ImGuiWindowFlags_)(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        graph_node_view->update();
        vec2 graph_view_TL = ImGui::GetCursorPos();
        bool changed = graph_node_view->draw_as_child("graph", vec2(m_child2_size, availSize.y), false, flags);
        if( changed )
        {
            graph->set_dirty();
        }
        ImRect overlay_rect(graph_view_TL, graph_view_TL + graph_node_view->get_visible_rect().GetSize());
        overlay_rect.Expand(vec2(-20)); // margin
        overlay_rect.Translate( ImGuiEx::CursorPosToScreenPos(vec2()));
        draw_overlay(overlay_rect);
    }
    else
    {
        ImGui::TextColored(ImColor(255,0,0), "ERROR: Unable to graw Graph View");
        LOG_ERROR("FileView", "graphNodeView is null\n");
    }

	return m_text_has_changed;
}

std::string FileView::get_text()const
{
	return m_text_editor.GetText();
}

void FileView::replace_selected_text(const std::string &_val)
{
    if (get_selected_text() != _val )
    {
        auto start = m_text_editor.GetCursorPosition();

        /* If there is no selection, selects current line */
        auto hasSelection = m_text_editor.HasSelection();
        auto selectionStart = m_text_editor.GetSelectionStart();
        auto selectionEnd = m_text_editor.GetSelectionEnd();

        // Select the whole line if no selection is set
        if (!hasSelection) {
            m_text_editor.MoveHome(false);
            m_text_editor.MoveEnd(true);
            m_text_editor.SetCursorPosition(TextEditor::Coordinates(start.mLine, 0));
        }

        /* insert text (and select it) */
        m_text_editor.InsertText(_val, true);

        auto end = m_text_editor.GetCursorPosition();
        if (!hasSelection && start.mLine == end.mLine) // no selection and insert text is still on the same line
        {
            m_text_editor.SetSelection(selectionStart, selectionEnd);
        }
        LOG_MESSAGE("FileView", "Selected text updated from graph.\n")
        LOG_VERBOSE("FileView", "%s \n", _val.c_str())
    }
}

void FileView::replace_text(const std::string& _content)
{
    const std::string current_content = get_text();
    if (current_content != _content )
    {
        return set_text(_content);
        auto cmd = std::make_shared<Cmd_ReplaceText>(current_content, _content, &m_text_editor);
        m_file.get_history()->push_command(cmd);

        LOG_MESSAGE("FileView", "Selected text updated from graph.\n")
        LOG_VERBOSE("FileView", "%s \n", _content.c_str())
    }
}

void FileView::set_text(const std::string& _content)
{
	m_text_editor.SetText(_content);
}

std::string FileView::get_selected_text()const
{
	return m_text_editor.HasSelection() ? m_text_editor.GetSelectedText() : m_text_editor.GetCurrentLineText();
}

void FileView::set_undo_buffer(TextEditor::IExternalUndoBuffer* _buffer ) {
	this->m_text_editor.SetExternalUndoBuffer(_buffer);
}

void FileView::draw_info() const
{
    // Basic information
    ImGui::Text("Name: %s", m_file.get_name().c_str());
    ImGui::Text("Path: %s", m_file.get_path().c_str());
    ImGui::NewLine();

    // Statistics
    ImGui::Text("Graph statistics:");
    ImGui::Indent();
    ImGui::Text("Node count: %lu", m_file.get_graph()->get_node_registry().size());
    ImGui::Text("Wire count: %lu", m_file.get_graph()->get_wire_registry().size());
    ImGui::Unindent();
    ImGui::NewLine();

    // Language browser (list functions/operators)
    if (ImGui::TreeNode("Language"))
    {
        ILanguage&         language   = m_ctx.language();
        const auto&       functions  = language.get_api();
        const ISerializer& serializer = language.get_serializer();

        ImGui::Columns(1);
        for(const auto& each_fct : functions )
        {
            std::string name;
            serializer.serialize(name, &each_fct->get_type());
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

void FileView::draw_overlay(ImRect rect)
{
    if( m_overlay_data.empty() ) return;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, vec4(0,0,0,0.1f));
    ImGui::PushStyleColor(ImGuiCol_Border, vec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_Text, vec4(0,0,0,0.5f));
    ImGui::SetNextWindowPos( rect.GetBL(), ImGuiCond_Always, vec2(0,1)); // bottom-left corner aligned
    ImGui::SetNextWindowSize(vec2(rect.GetSize()), ImGuiCond_Appearing);
    bool show = true;
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Quick Help:", &show, flags);

    ImGui::Indent(5);
    std::for_each(m_overlay_data.begin(), m_overlay_data.end(), [](const OverlayData& _data) {
        ImGui::Text("%s:", _data.label.c_str());
        ImGui::SameLine(150);
        ImGui::Text("%s", _data.description.c_str());
    });
    ImGui::PopStyleColor(3);
    ImGui::End();
}
