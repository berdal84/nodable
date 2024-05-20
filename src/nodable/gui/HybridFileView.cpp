#include "HybridFileView.h"

#include "core/Graph.h"
#include "core/Node.h"
#include "core/VirtualMachine.h"
#include "core/language/Nodlang.h"
#include "core/NodeUtils.h"

#include "commands/Cmd_ReplaceText.h"
#include "commands/Cmd_WrappedTextEditorUndoRecord.h"
#include "Event.h"
#include "HybridFile.h"
#include "GraphView.h"
#include "NodeView.h"
#include "Config.h"

using namespace ndbl;
using namespace fw;

HybridFileView::HybridFileView(HybridFile& _file)
    : View()
    , m_text_editor()
    , m_focused_text_changed(false)
    , m_is_graph_dirty(false)
    , m_file(_file)
    , m_child1_size(0.3f)
    , m_child2_size(0.7f)
    , m_experimental_clipboard_auto_paste(false)
    , m_text_overlay_window_name(_file.name + "_text_overlay" )
    , m_graph_overlay_window_name(_file.name + "_graph_overlay" )
{
    m_graph_changed_observer.observe(_file.graph_changed, [this](Graph* _graph)
    {
        LOG_VERBOSE("FileView", "graph changed evt received\n")
        if ( !_graph->is_empty() )
        {
            LOG_VERBOSE("FileView", "graph is not empty\n")
            Node* root = _graph->get_root().get();

            NodeView* root_node_view = root->get_component<NodeView>().get();
            GraphView* graph_view = m_file.get_graph_view();

            // unfold graph (lot of updates) and frame all nodes
            if ( root_node_view && graph_view )
            {
                // visually unfold graph. Does not work super well...
                graph_view->unfold();

                // make sure views are outside viewable rectangle (to avoid flickering)
                auto views = NodeUtils::get_components<NodeView>( _graph->get_node_registry() );
                graph_view->translate_all(views, Vec2(-1000.f, -1000.0f), NodeViewFlag_NONE);

                // frame all (33ms delayed)
                EventManager::get_instance().dispatch_delayed<Event_FrameSelection>( 33, {FRAME_ALL} );
            }
        }
    });
}

void HybridFileView::init()
{
	static auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	m_text_editor.SetLanguageDefinition(lang);
	m_text_editor.SetImGuiChildIgnored(true);
	m_text_editor.SetPalette(Nodable::get_instance().config.ui_text_textEditorPalette);
}

bool HybridFileView::onDraw()
{
    const Vec2 margin(10.0f, 0.0f);
    const Nodable &app       = Nodable::get_instance();
    Vec2 region_available    = (Vec2)ImGui::GetContentRegionAvail() - margin;
    Vec2 text_editor_size {m_child1_size, region_available.y};
    Vec2 graph_editor_size{m_child2_size, region_available.y};

     // Splitter
    //---------

    if (m_child1_size + m_child2_size != region_available.x )
    {
        float ratio = region_available.x / (m_child1_size + m_child2_size);
        m_child1_size *= ratio;
        m_child2_size *= ratio;
    }

    Rect splitter_rect{
        ImGui::GetCursorScreenPos(),
        (Vec2)ImGui::GetCursorScreenPos() + Vec2(4.0f, region_available.y)
    };
    splitter_rect.translate_x( m_child1_size + 2.0f );
    ImGui::SplitterBehavior(ImGuiEx::toImGui(splitter_rect), ImGui::GetID("file_splitter"), ImGuiAxis_X, &m_child1_size, &m_child2_size, 20.0f, 20.0f);

     // TEXT EDITOR
    //------------

    Vec2 text_editor_top_left_corner = ImGui::GetCursorPos();
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
        Rect overlay_rect = ImGuiEx::GetContentRegion( WORLD_SPACE );
        overlay_rect.expand( Vec2( -2.f * app.config.ui_overlay_margin ) ); // margin
        draw_overlay(m_text_overlay_window_name.c_str(), m_overlay_data[OverlayType_TEXT], overlay_rect, Vec2(0, 1));
        ImGuiEx::DebugRect( overlay_rect.min, overlay_rect.max, IM_COL32( 255, 255, 0, 127 ) );

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

        m_focused_text_changed = is_line_text_modified ||
                                 m_text_editor.IsTextChanged() ||
                                 (app.config.isolate_selection && is_selected_text_modified);

        if (m_text_editor.IsTextChanged())  m_file.is_content_dirty = true;
    }
    ImGui::EndChild();

     // NODE EDITOR
    //-------------

    Graph*     graph      = m_file.get_graph();
    GraphView* graph_view = m_file.get_graph_view();
    FW_ASSERT(graph);
    ImGui::SameLine();
    if ( graph_view )
    {
        LOG_VERBOSE("FileView", "graph_node_view->update()\n");
        ImGuiWindowFlags flags = (ImGuiWindowFlags_)(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        graph_view->update();
        Vec2 graph_editor_top_left_corner = ImGui::GetCursorPos();

        ImGui::BeginChild("graph", graph_editor_size, false, flags);
        {
            // Draw graph
            m_is_graph_dirty = graph_view->draw();

            // Draw overlay: shortcuts
            Rect overlay_rect = ImGuiEx::GetContentRegion( WORLD_SPACE );
            overlay_rect.expand( Vec2( -2.0f * app.config.ui_overlay_margin ) ); // margin
            draw_overlay(m_graph_overlay_window_name.c_str(), m_overlay_data[OverlayType_GRAPH], overlay_rect, Vec2(1, 1));
            ImGuiEx::DebugRect( overlay_rect.min, overlay_rect.max, IM_COL32( 255, 255, 0, 127 ) );

            // Draw overlay: isolation mode ON/OFF
            if( app.config.isolate_selection )
            {
                Vec2 cursor_pos = graph_editor_top_left_corner + Vec2(app.config.ui_overlay_margin);
                ImGui::SetCursorPos(cursor_pos);
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

    return changed();
}

std::string HybridFileView::get_text()const
{
	return m_text_editor.GetText();
}

void HybridFileView::replace_selected_text(const std::string &_val)
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

void HybridFileView::replace_text(const std::string& _content)
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

void HybridFileView::set_text(const std::string& _content)
{
	m_text_editor.SetText(_content);
}

std::string HybridFileView::get_selected_text()const
{
	return m_text_editor.HasSelection() ? m_text_editor.GetSelectedText() : m_text_editor.GetCurrentLineText();
}

void HybridFileView::set_undo_buffer(TextEditor::IExternalUndoBuffer* _buffer ) {
	this->m_text_editor.SetExternalUndoBuffer(_buffer);
}

void HybridFileView::draw_info_panel() const
{
    // Basic information
    ImGui::Text("Current file:");
    ImGui::Indent();
    ImGui::TextWrapped("path: %s", m_file.path.string().c_str());
    ImGui::TextWrapped("size: %0.3f KiB", float(m_file.size()) / 1000.0f );
    ImGui::Unindent();
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
            language.serialize_func_sig(name, each_fct->get_type());
            ImGui::Text("%s", name.c_str());
        }

        ImGui::TreePop();
    }
}

void  HybridFileView::experimental_clipboard_auto_paste(bool _enable)
{
    m_experimental_clipboard_auto_paste = _enable;
    if( _enable )
    {
        m_experimental_clipboard_prev = "";
    }
}

void HybridFileView::draw_overlay(const char* title, const std::vector<OverlayData>& overlay_data, Rect rect, Vec2 position)
{
    if( overlay_data.empty() ) return;

    const auto& app = Nodable::get_instance();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, app.config.ui_overlay_window_bg_golor);
    ImGui::PushStyleColor(ImGuiCol_Border, app.config.ui_overlay_border_color);
    ImGui::PushStyleColor(ImGuiCol_Text, app.config.ui_overlay_text_color);
    Vec2 win_position = rect.tl() + rect.size() * position;
    ImGui::SetNextWindowPos( win_position, ImGuiCond_Always, position);
    ImGui::SetNextWindowSize( rect.size(), ImGuiCond_Appearing);
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

void HybridFileView::clear_overlay()
{
    std::for_each(m_overlay_data.begin(), m_overlay_data.end(), [&](auto &vec) {
        vec.clear();
    });
}

void HybridFileView::push_overlay(OverlayData overlay_data, OverlayType overlay_type)
{
    m_overlay_data[overlay_type].push_back(overlay_data);
}

size_t HybridFileView::size() const
{
    return m_text_editor.Size();
}

void HybridFileView::refresh_overlay(Condition _condition )
{
    for (const IAction* _action: ActionManager::get_instance().get_actions())
    {
        if( ( _action->userdata & _condition) == _condition && (_action->userdata & Condition_HIGHLIGHTED) )
        {
            std::string  label        = _action->label.substr(0, 12);
            std::string  shortcut_str = _action->shortcut.to_string();
            OverlayType_ overlay_type = _action->userdata & Condition_HIGHLIGHTED_IN_TEXT_EDITOR ? OverlayType_TEXT
                                                                                                 : OverlayType_GRAPH;
            push_overlay({label, shortcut_str}, overlay_type);
        }
    }
}
