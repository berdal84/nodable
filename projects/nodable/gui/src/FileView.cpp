#include <ndbl/gui/FileView.h>

#include <ndbl/gui/Event.h>
#include <ndbl/gui/File.h>
#include <ndbl/gui/GraphNodeView.h>
#include <ndbl/gui/NodeView.h>
#include <ndbl/gui/Config.h>
#include <ndbl/gui/commands/Cmd_ReplaceText.h>
#include <ndbl/gui/commands/Cmd_WrappedTextEditorUndoRecord.h>
#include <ndbl/core/GraphNode.h>
#include <ndbl/core/Node.h>
#include <ndbl/core/VirtualMachine.h>
#include <ndbl/core/language/Nodlang.h>

using namespace ndbl;
using namespace fw;

FileView::FileView(File& _file)
    : fw::View()
    , m_text_editor()
    , m_text_has_changed(false)
    , m_file(_file)
    , m_child1_size(0.3f)
    , m_child2_size(0.7f)
    , m_experimental_clipboard_auto_paste(false)
    , m_text_overlay_window_name(_file.name + "_text_overlay" )
    , m_graph_overlay_window_name(_file.name + "_graph_overlay" )
{
    m_graph_change_obs.observe(_file.event_graph_changed, [](GraphNode* _graph)
    {
        LOG_VERBOSE("FileView", "graph changed evt received\n")
        if ( !_graph->is_empty() )
        {
            LOG_VERBOSE("FileView", "graph is not empty\n")
            Node* root = _graph->get_root();

            auto* root_node_view = root->get<NodeView>();
            auto* graph_view = root->get_parent_graph()->get<GraphNodeView>();

            // unfold graph (lot of updates) and frame all nodes
            if ( root_node_view && graph_view )
            {
                // unfold graph (simulate 1000 updates). Does not work well,
                graph_view->update( 100000.0f, 100);

                // make sure views are outside viewable rectangle (to avoid flickering)
                std::vector<NodeView*> views;
                Node::get_components<NodeView>( _graph->get_node_registry(), views );
                graph_view->translate_all( ImVec2(-10000.0f), views);

                // frame all (delay to next frame via event system)
                fw::EventManager::get_instance().push(EventType_frame_all_node_views);
            }
        }
    });
}

void FileView::init()
{
	static auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	m_text_editor.SetLanguageDefinition(lang);
	m_text_editor.SetImGuiChildIgnored(true);
	m_text_editor.SetPalette(Nodable::get_instance().config.ui_text_textEditorPalette);
}

bool FileView::draw_implem()
{
    const ImVec2 margin(10.0f, 0.0f);
    const Nodable &app       = Nodable::get_instance();
    ImVec2 region_available  = ImGui::GetContentRegionAvail() - margin;
    ImVec2 text_editor_size  = ImVec2(m_child1_size, region_available.y);
    ImVec2 graph_editor_size = ImVec2(m_child2_size, region_available.y);

     // Splitter
    //---------

    if (m_child1_size + m_child2_size != region_available.x )
    {
        float ratio = region_available.x / (m_child1_size + m_child2_size);
        m_child1_size *= ratio;
        m_child2_size *= ratio;
    }

    ImRect splitter_rect{ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos()};
    splitter_rect.Max += ImVec2(region_available.y, 4.0f);
    splitter_rect.TranslateX(m_child1_size + 2.0f);
    ImGui::SplitterBehavior(splitter_rect, ImGui::GetID("file_splitter"), ImGuiAxis_X, &m_child1_size, &m_child2_size, 20.0f, 20.0f);

     // TEXT EDITOR
    //------------

    ImVec2 text_editor_top_left_corner = ImGui::GetCursorPos();
    ImGui::BeginChild("text_editor", text_editor_size, false);
    {
        auto old_cursor_position = m_text_editor.GetCursorPosition();
        auto old_selected_text = m_text_editor.GetSelectedText();
        auto old_line_text = m_text_editor.GetCurrentLineText();

        bool is_running = app.virtual_machine.is_program_running();
        auto allow_keyboard = !is_running &&
                              !NodeView::is_any_dragged();

        auto allow_mouse = !is_running &&
                           !NodeView::is_any_dragged() &&
                           !ImGui::IsAnyItemHovered() &&
                           !ImGui::IsAnyItemFocused();

        m_text_editor.SetHandleKeyboardInputs(allow_keyboard);
        m_text_editor.SetHandleMouseInputs(allow_mouse);

        // listen to clipboard in background (disable by default)
        if (m_experimental_clipboard_auto_paste)
        {
            m_experimental_clipboard_curr = ImGui::GetClipboardText();
            if (!m_experimental_clipboard_curr.empty() &&
                m_experimental_clipboard_curr != m_experimental_clipboard_prev)
            {
                if (!m_experimental_clipboard_prev.empty())
                    m_text_editor.InsertText(m_experimental_clipboard_curr.c_str(), true);
                m_experimental_clipboard_prev = std::move(m_experimental_clipboard_curr);
            }
        }

        m_file.get_history()->enable_text_editor(true); // ensure to begin to record history

        // render text editor
        m_text_editor.Render("Text Editor Plugin", ImGui::GetContentRegionAvail());

        // overlay
        ImRect overlay_rect = fw::ImGuiEx::GetContentRegion(fw::Space_Screen);
        overlay_rect.Expand(ImVec2(-2.f * app.config.ui_overlay_margin)); // margin
        draw_overlay(m_text_overlay_window_name.c_str(), m_overlay_data[OverlayType_TEXT], overlay_rect, ImVec2(0, 1));
        fw::ImGuiEx::DebugRect( overlay_rect.Min, overlay_rect.Max, IM_COL32( 255, 255, 0, 127 ) );

        if (app.config.experimental_hybrid_history)
        {
            m_file.get_history()->enable_text_editor(false); // avoid recording events caused by graph serialisation
        }

        auto new_cursor_position = m_text_editor.GetCursorPosition();
        auto new_selected_text   = m_text_editor.GetSelectedText();
        auto new_line_text       = m_text_editor.GetCurrentLineText();

        auto is_line_text_modified = new_line_text != old_line_text &&
                                     new_cursor_position.mLine == old_cursor_position.mLine;
        auto is_selected_text_modified = new_cursor_position != old_cursor_position;

        m_text_has_changed = is_line_text_modified ||
                             m_text_editor.IsTextChanged() ||
                             (app.config.isolate_selection && is_selected_text_modified);

        if (m_text_editor.IsTextChanged())  m_file.changed = true;
        if (m_text_has_changed) m_file.update_graph();
    }
    ImGui::EndChild();

     // NODE EDITOR
    //-------------

    GraphNode* graph = m_file.get_graph();
    FW_ASSERT(graph);
    auto graph_node_view = graph->get<GraphNodeView>();
    ImGui::SameLine();
    if ( graph_node_view )
    {
        LOG_VERBOSE("FileView", "graph_node_view->update()\n");
        ImGuiWindowFlags flags = (ImGuiWindowFlags_)(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        graph_node_view->update();
        ImVec2 graph_editor_top_left_corner = ImGui::GetCursorPos();

        ImGui::BeginChild("graph", graph_editor_size, false, flags);
        {
            bool changed = graph_node_view->draw();
            if( changed )
            {
                graph->set_dirty();
            }

            // overlay (for shortcuts)
            ImRect overlay_rect = fw::ImGuiEx::GetContentRegion(fw::Space_Screen);
            overlay_rect.Expand(ImVec2(-2.0f * app.config.ui_overlay_margin)); // margin
            draw_overlay(m_graph_overlay_window_name.c_str(), m_overlay_data[OverlayType_GRAPH], overlay_rect, ImVec2(1, 1));
            fw::ImGuiEx::DebugRect( overlay_rect.Min, overlay_rect.Max, IM_COL32( 255, 255, 0, 127 ) );

            // overlay (for isolation mode status)
            if( app.config.isolate_selection )
            {
                ImGui::SetCursorPos(graph_editor_top_left_corner + app.config.ui_overlay_margin);
                ImGui::Text("Isolation mode ON");
            }
        }
        ImGui::EndChild();

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
        set_text(_content);
        // auto cmd = std::make_shared<Cmd_ReplaceText>(current_content, _content, &m_text_editor);
        // m_file.get_history()->push_command(cmd);

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
    ImGui::Text("Name: %s", m_file.name.c_str());
    ImGui::Text("Path: %s", m_file.path.string().c_str());
    ImGui::NewLine();

    // Statistics
    ImGui::Text("Graph statistics:");
    ImGui::Indent();
    ImGui::Text("Node count: %zu", m_file.get_graph()->get_node_registry().size());
    ImGui::Text("Edge count: %zu", m_file.get_graph()->get_edge_registry().size());
    ImGui::Unindent();
    ImGui::NewLine();

    // Language browser (list functions/operators)
    if (ImGui::TreeNode("Language"))
    {
        const Nodlang& language = Nodlang::get_instance();

        ImGui::Columns(1);
        for(const auto& each_fct : language.get_api() )
        {
            std::string name;
            language.serialize(name, &each_fct->get_type());
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

void FileView::draw_overlay(const char* title, const std::vector<OverlayData>& overlay_data, ImRect rect, ImVec2 position)
{
    if( overlay_data.empty() ) return;

    const auto& app = Nodable::get_instance();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, app.config.ui_overlay_window_bg_golor);
    ImGui::PushStyleColor(ImGuiCol_Border, app.config.ui_overlay_border_color);
    ImGui::PushStyleColor(ImGuiCol_Text, app.config.ui_overlay_text_color);
    ImGui::SetNextWindowPos( rect.GetTL() + rect.GetSize() * position, ImGuiCond_Always, position);
    ImGui::SetNextWindowSize(rect.GetSize(), ImGuiCond_Appearing);
    bool show = true;
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMouseInputs;

    if (ImGui::Begin(title, &show, flags) )
    {
        ImGui::Indent(app.config.ui_overlay_indent);
        std::for_each(overlay_data.begin(), overlay_data.end(), [](const OverlayData& _data) {
            ImGui::Text("%s:", _data.label.c_str());
            ImGui::SameLine(150);
            ImGui::Text("%s", _data.description.c_str());
        });
        ImGui::PopStyleColor(3);
    }
    ImGui::End();
}

void FileView::clear_overlay()
{
    std::for_each(m_overlay_data.begin(), m_overlay_data.end(), [&](auto &vec) {
        vec.clear();
    });
}

void FileView::push_overlay(OverlayData overlay_data, OverlayType overlay_type)
{
    m_overlay_data[overlay_type].push_back(overlay_data);
}
