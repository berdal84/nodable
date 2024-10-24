#include "FileView.h"

#include "tools/gui/ImGuiTypeConvert.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/Interpreter.h"
#include "ndbl/core/language/Nodlang.h"
#include "ndbl/core/Utils.h"


#include "Config.h"
#include "Event.h"
#include "File.h"
#include "GraphView.h"
#include "NodeView.h"
#include "commands/Cmd_ReplaceText.h"
#include "commands/Cmd_WrappedTextEditorUndoRecord.h"
#include "Physics.h"

using namespace ndbl;
using namespace tools;

FileView::FileView()
    : m_text_editor()
    , is_graph_dirty(false)
    , is_text_dirty(false)
    , m_child1_size(0.3f)
    , m_child2_size(0.7f)
    , m_file(nullptr)
    , m_experimental_clipboard_auto_paste(false)
{
}

void FileView::init(File& _file)
{
    Config* cfg = get_config();

    m_file = &_file;
    std::string overlay_basename{_file.filename()};
    m_text_overlay_window_name  = overlay_basename + "_text_overlay";
    m_graph_overlay_window_name = overlay_basename + "_graph_overlay";

    static auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	m_text_editor.SetLanguageDefinition(lang);
	m_text_editor.SetImGuiChildIgnored(true);
	m_text_editor.SetPalette( cfg->ui_text_textEditorPalette );
}

bool FileView::draw()
{
    Config* cfg = get_config();
    const Vec2 margin(10.0f, 0.0f);
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
    ImGui::SplitterBehavior(toImGui(splitter_rect), ImGui::GetID("file_splitter"), ImGuiAxis_X, &m_child1_size, &m_child2_size, 20.0f, 20.0f);

     // TEXT EDITOR
    //------------

    Vec2 text_editor_top_left_corner = ImGui::GetCursorPos();
    ImGui::BeginChild("text_editor", text_editor_size, false);
    {
        auto old_cursor_position = m_text_editor.GetCursorPosition();
        auto old_selected_text = m_text_editor.GetSelectedText();
        auto old_line_text = m_text_editor.GetCurrentLineText();

        bool is_running = get_interpreter()->is_program_running();
        GraphView* graphview = m_file->graph().view();
        auto allow_keyboard = !is_running &&
                              !graphview->has_an_active_tool();

        auto allow_mouse = !is_running &&
                           !graphview->has_an_active_tool() &&
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

        m_file->history.enable_text_editor(true); // ensure to begin to record history

        // render text editor
        m_text_editor.Render("Text Editor Plugin", ImGui::GetContentRegionAvail());

        // overlay
        Rect overlay_rect = ImGuiEx::GetContentRegion(WORLD_SPACE );
        overlay_rect.expand( Vec2( -2.f * cfg->ui_overlay_margin ) ); // margin
        draw_overlay(m_text_overlay_window_name.c_str(), m_overlay_data[OverlayType_TEXT], overlay_rect, Vec2(0, 1));
        ImGuiEx::DebugRect( overlay_rect.min, overlay_rect.max, IM_COL32( 255, 255, 0, 127 ) );

        if ( cfg->flags & ConfigFlag_EXPERIMENTAL_MULTI_SELECTION )
        {
            m_file->history.enable_text_editor(false); // avoid recording events caused by graph serialisation
        }

        auto new_cursor_position = m_text_editor.GetCursorPosition();
        auto new_selected_text   = m_text_editor.GetSelectedText();
        auto new_line_text       = m_text_editor.GetCurrentLineText();

        auto is_line_text_modified = new_line_text != old_line_text &&
                                     new_cursor_position.mLine == old_cursor_position.mLine;
        auto is_selected_text_modified = new_cursor_position != old_cursor_position;

        is_graph_dirty |= is_line_text_modified;
        is_graph_dirty |= m_text_editor.IsTextChanged();
        is_graph_dirty |= ( cfg->isolation && is_selected_text_modified);
    }
    ImGui::EndChild();

     // NodeViewItem EDITOR
    //-------------

    Graph&     graph      = m_file->graph();
    GraphView* graph_view = graph.view();

    ASSERT(graph_view);

    ImGui::SameLine();
    LOG_VERBOSE("FileView", "graph_node_view->update_world_matrix()\n");
    ImGuiWindowFlags flags = (ImGuiWindowFlags_)(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    Vec2 graph_editor_top_left_corner = ImGui::GetCursorPos();

    ImGui::BeginChild("graph", graph_editor_size, false, flags);
    {
        // Draw graph
        is_text_dirty |= graph_view->draw();

        // Draw overlay: shortcuts
        Rect overlay_rect = ImGuiEx::GetContentRegion(WORLD_SPACE );
        overlay_rect.expand( Vec2( -2.0f * cfg->ui_overlay_margin ) ); // margin
        draw_overlay(m_graph_overlay_window_name.c_str(), m_overlay_data[OverlayType_GRAPH], overlay_rect, Vec2(1, 1));
        ImGuiEx::DebugRect( overlay_rect.min, overlay_rect.max, IM_COL32( 255, 255, 0, 127 ) );

        // Draw overlay: isolation mode ON/OFF
        if( cfg->isolation )
        {
            Vec2 cursor_pos = graph_editor_top_left_corner + Vec2( cfg->ui_overlay_margin);
            ImGui::SetCursorPos(cursor_pos);
            ImGui::Text("Isolation mode ON");
        }
    }
    ImGui::EndChild();

    return is_graph_dirty || is_text_dirty;
}

std::string FileView::get_text( Isolation mode )const
{
    if ( mode == Isolation_OFF )
    {
        return m_text_editor.GetText();
    }

    if ( m_text_editor.HasSelection() )
    {
        return m_text_editor.GetSelectedText();
    }

    return m_text_editor.GetCurrentLineText(); // By default, we consider the current line as the selection
}

void FileView::set_text(const std::string& text, Isolation mode)
{
    if ( get_text(mode) == text )
    {
        return;
    }

    if( mode == Isolation_ON )
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
        m_text_editor.InsertText( text, true);

        auto end = m_text_editor.GetCursorPosition();
        if (!hasSelection && start.mLine == end.mLine) // no selection and insert text is still on the same line
        {
            m_text_editor.SetSelection(selectionStart, selectionEnd);
        }
        LOG_MESSAGE("FileView", "Selected text updated from graph.\n");
        LOG_VERBOSE("FileView", "%s \n", text.c_str());
    }
    else
    {
        m_text_editor.SetText(text);
        // auto cmd = std::make_shared<Cmd_ReplaceText>(current_content, text, &m_text_editor);
        // m_file->get_history()->push_command(cmd);

        LOG_MESSAGE("FileView", "Whole text updated from graph.\n");
        LOG_VERBOSE("FileView", "%s \n", text.c_str());
    }
}

void FileView::set_undo_buffer(TextEditor::IExternalUndoBuffer* _buffer ) {
	this->m_text_editor.SetExternalUndoBuffer(_buffer);
}

void FileView::draw_info_panel() const
{
    // Basic information
    ImGui::Text("Current file:");
    ImGui::Indent();
    ImGui::TextWrapped("path: %s", m_file->path.string().c_str());
    ImGui::TextWrapped("set_size: %0.3f KiB", float(m_file->size()) / 1000.0f );
    ImGui::Unindent();
    ImGui::NewLine();

    // Statistics
    ImGui::Text("Graph statistics:");
    ImGui::Indent();
    ImGui::Text("Node count: %zu", m_file->graph().get_node_registry().size());
    ImGui::Text("Edge count: %zu", m_file->graph().get_edge_registry().size());
    ImGui::Unindent();
    ImGui::NewLine();

    // Language browser (list functions/operators)
    if (ImGui::TreeNode("Language"))
    {
        const Nodlang* language = get_language();

        ImGui::Columns(1);
        for(const IInvokable* invokable : language->get_api() )
        {
            std::string name;
            language->serialize_invokable_sig(name, invokable );
            ImGui::Text("%s", name.c_str());
        }

        ImGui::TreePop();
    }
}

void FileView::experimental_clipboard_auto_paste(bool _enable)
{
    m_experimental_clipboard_auto_paste = _enable;
    if( _enable )
    {
        m_experimental_clipboard_prev = "";
    }
}

void FileView::draw_overlay(const char* title, const std::vector<OverlayData>& overlay_data, const Rect& rect, const Vec2& position)
{
    if( overlay_data.empty() ) return;

    Config* cfg = get_config();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, cfg->ui_overlay_window_bg_golor);
    ImGui::PushStyleColor(ImGuiCol_Border, cfg->ui_overlay_border_color);
    ImGui::PushStyleColor(ImGuiCol_Text, cfg->ui_overlay_text_color);
    Vec2 win_position = rect.top_left() + rect.size() * position;
    ImGui::SetNextWindowPos( win_position, ImGuiCond_Always, position);
    ImGui::SetNextWindowSize( rect.size(), ImGuiCond_Appearing);
    bool show = true;
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMouseInputs;

    if (ImGui::Begin(title, &show, flags) )
    {
        ImGui::Indent( cfg->ui_overlay_indent);
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
    for(auto& vec : m_overlay_data )
        vec.clear();
}

void FileView::push_overlay(OverlayData overlay_data, OverlayType overlay_type)
{
    m_overlay_data[overlay_type].push_back(overlay_data);
}

size_t FileView::size() const
{
    return m_text_editor.Size();
}

void FileView::refresh_overlay(Condition _condition )
{
    for (const IAction* _action: get_action_manager()->get_actions())
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

void FileView::update(float dt)
{
    GraphView* graph_view = m_file->graph().view();
    ASSERT(graph_view != nullptr);
    graph_view->update(dt);

    if( is_text_dirty )
    {
        m_file->update_text_from_graph();
        is_text_dirty = false;

        // Refresh constraints
        auto physics_components = Utils::get_components<Physics>(m_file->graph().get_node_registry() );
        Physics::destroy_constraints( physics_components );
        Physics::create_constraints(m_file->graph().get_node_registry() );
    }
    else if( is_graph_dirty )
    {
        m_file->update_graph_from_text();
        is_graph_dirty = false;
    }
    is_text_dirty |= m_file->graph().update(); // ~ garbage collection
}
