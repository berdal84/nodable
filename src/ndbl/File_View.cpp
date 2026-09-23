#include "File_View.h"

#include "Action_Manager.h"
#include "Command_Manager.h"
#include "Config.h"
#include "Event.h"
#include "File.h"
#include "Flags.h"
#include "Font_Manager.h"
#include "Graph_View.h"
#include "Graph.h"
#include "ImGuiEx.h"
#include "ImGuiTypeConvert.h"
#include "Node_View.h"
#include "Node.h"
#include "Parser.h"
#include "texteditor/Text_Editor_ImGui_Renderer.h"

namespace ndbl
{

void fileview_init(File_View* file_view, File* file)
{
    Config* cfg = config();

    file_view->file = file;
    file_view->text_overlay_window_name  = bdc::string_printf( "%s_text_overlay" , file->name.data );
    file_view->graph_overlay_window_name = bdc::string_printf( "%s_graph_overlay", file->name.data );

	file_view->text_editor_palette = cfg->ui_text_editor_palette;
    file_view->is_syntax_dirty     = true;

    ASSERT(file->graph->view);
    file_view->graph_view = file->graph->view;

    VERIFY( file_view->file->graph->view, "A Graph_View component is required by File_View" );
}

void fileview_deinit(File_View* file_view)
{
    string_release(file_view->text_overlay_window_name );
    string_release(file_view->graph_overlay_window_name);
}

void fileview_update(File_View* file_view, float dt)
{
    graphview_update(file_view->graph_view, dt);

    if( file_view->is_syntax_dirty )
    {
        file_view->is_syntax_dirty = false;
        
        size_t                      next_token_id       = 0;
        int                         colorize_char_count = 0;
        PaletteIndex                index               = PaletteIndex::Default;
        const std::vector<Token>&   tokens              = file_view->file->parser.pristine_tokens;

        for(auto& line : file_view->text_editor.mLines )
        {
            for(auto& glyph : line )
            {
                while( colorize_char_count <= 0 && next_token_id < tokens.size() )
                {
                    auto& tok = tokens[next_token_id];
                    colorize_char_count = tok.size();
                    switch ( tok.type)
                    {
                        case Token_Type_keyword_if:                                
                        case Token_Type_keyword_else:                                
                        case Token_Type_keyword_for:                                
                        case Token_Type_keyword_while:                                
                        case Token_Type_keyword_operator:                                
                        case Token_Type_keyword_return:                                
                        case Token_Type_keyword_string:                                
                        case Token_Type_keyword_double:                                
                        case Token_Type_keyword_int:                                
                        case Token_Type_keyword_i16:                                
                        case Token_Type_keyword_bool:                                
                        case Token_Type_keyword_any:                                
                        case Token_Type_keyword_unknown:
                        {
                            index = PaletteIndex::Keyword;
                            break;
                        }

                        case Token_Type_identifier:
                        {
                            index = PaletteIndex::KnownIdentifier;
                            break;
                        }

                        case Token_Type_literal_string:  
                        {
                            index = PaletteIndex::String;
                            break;
                        }                              
                        case Token_Type_literal_double:                                
                        case Token_Type_literal_int:                                
                        case Token_Type_literal_bool:                                
                        case Token_Type_literal_any:                                
                        case Token_Type_literal_unknown:
                        {
                            index = PaletteIndex::Number;
                            break;
                        }
                        case Token_Type_comment:
                        {
                            index = PaletteIndex::Comment;
                            break;
                        }
                        case Token_Type_multiline_comment:
                        {
                            index = PaletteIndex::MultiLineComment;
                            break;
                        }
                        case Token_Type_end_of_line:
                        {
                            colorize_char_count = 0;
                            break;
                        }
                        default:
                        {                            
                            index = PaletteIndex::Default;
                        }
                    }

                    ++next_token_id;
                }

                glyph.mColorIndex = index;
                colorize_char_count -= 1;                
            }
        }
        
    }
}

Condition_Flags fileview_calc_condition_flags(File_View* file_view)
{
    Condition_Flags result = 0;
    result |= Condition_Flags_ENABLE_IF_HAS_SELECTION * !file_view->graph_view->selection.empty();
    return result;
}

void fileview_draw(File_View* file_view, float dt)
{
    // Summary
    // 1) Draw History Bar
    // 2) Draw Text and Graph Editors

    fileview_clear_overlay(file_view);
    fileview_refresh_overlay( file_view );

    // 1)
    if (ImGui::IsMouseReleased(0))
    {
        file_view->is_history_dragged = false;
    }
    auto* cfg           = config();
    float btn_spacing   = cfg->ui_history_btn_spacing;
    float btn_height    = cfg->ui_history_btn_height;
    float btn_width_max = cfg->ui_history_btn_width_max;

    size_t history_size = command_manager_get_size();
    std::pair<int, int> history_range = command_manager_get_command_id_range();
    float avail_width = ImGui::GetContentRegionAvail().x;
    float btn_width = fmin(btn_width_max, avail_width / float(history_size + 1) - btn_spacing);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { btn_spacing, 0});

    for (int cmd_pos = history_range.first; cmd_pos <= history_range.second; cmd_pos++)
    {
        ImGui::SameLine();

        bdc::String label = bdc::string_tprintf("##%i", cmd_pos);

        // Draw a highlighted button for the current history position
        if (cmd_pos == 0) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            ImGui::Button(label.c_str(), {btn_width, btn_height});
            ImGui::PopStyleColor();
        }
        else // or a simple one for other history positions
        {
            ImGui::Button(label.c_str(), {btn_width, btn_height});
        }

        // Hovered item
        if (ImGui::IsItemHovered())
        {
            if (ImGui::IsMouseDown(0)) // hovered + mouse down
            {
                file_view->is_history_dragged = true;
            }

            // Draw command description
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, float(0.8));
            if (ImGuiEx::BeginTooltip())
            {
                ImGui::Text("%s", command_manager_get_cmd_description_at(cmd_pos).c_str());
                ImGuiEx::EndTooltip();
            }
            ImGui::PopStyleVar();
        }

        // When dragging history
        if (file_view->is_history_dragged &&
            ImGui::GetMousePos().x > ImGui::GetItemRectMin().x &&
            ImGui::GetMousePos().x < ImGui::GetItemRectMax().x)
        {
            command_manager_move_cursor(cmd_pos); // update history cursor position
        }


    }
    ImGui::PopStyleVar();

    // 2)
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.35f));
    ImGui::PushFont( font_manager_get_by_slot(Font_Slot_Code) );

    ImGui::BeginChild("File_View", ImGui::GetContentRegionAvail(), false, 0);
    {
        const Vec2 margin(10.0f, 0.0f);
        Vec2 region_available    = (Vec2)ImGui::GetContentRegionAvail() - margin;
        Vec2 text_editor_size {file_view->text_child_size, region_available.y};
        Vec2 graph_editor_size{file_view->graph_child_size, region_available.y};
        bool text_view_changed = false;
        bool graph_view_changed = false;

         // Splitter
        //---------

        if (file_view->text_child_size + file_view->graph_child_size != region_available.x )
        {
            float ratio = region_available.x / ( file_view->text_child_size + file_view->graph_child_size );
            file_view->text_child_size *= ratio;
            file_view->graph_child_size *= ratio;
        }

        Rect splitter_rect{
            ImGui::GetCursorScreenPos(),
            (Vec2)ImGui::GetCursorScreenPos() + Vec2(4.0f, region_available.y)
        };
        splitter_rect.translate_x( file_view->text_child_size + 2.0f );
        ImGui::SplitterBehavior(toImGui(splitter_rect), ImGui::GetID("file_splitter"), ImGuiAxis_X, &file_view->text_child_size, &file_view->graph_child_size, 20.0f, 20.0f);

         // TEXT EDITOR
        //------------

        Vec2 text_editor_top_left_corner = ImGui::GetCursorPos();
        ImGui::BeginChild("text_editor", text_editor_size, false);
        {
            auto old_cursor_position = file_view->text_editor.GetCursorPosition();
            auto old_selected_text = file_view->text_editor.GetSelectedText();
            auto old_line_text = file_view->text_editor.GetCurrentLineText();

            auto allow_keyboard = !graphview_has_an_active_tool(file_view->graph_view);

            auto allow_mouse = !graphview_has_an_active_tool(file_view->graph_view) &&
                               !ImGui::IsAnyItemHovered() &&
                               !ImGui::IsAnyItemFocused();

            file_view->text_editor.mHandleKeyboardInputs = allow_keyboard;
            file_view->text_editor.mHandleMouseInputs    = allow_mouse;

            // listen to clipboard in background (disable by default)
            if (file_view->experimental_clipboard_auto_paste)
            {
                bdc::String clipboard = ImGui::GetClipboardText();
                file_view->experimental_clipboard_curr = string_copy(clipboard);
                if (!file_view->experimental_clipboard_curr.size &&
                    file_view->experimental_clipboard_curr != file_view->experimental_clipboard_prev)
                {
                    if (!file_view->experimental_clipboard_prev.size )
                        file_view->text_editor.InsertText(file_view->experimental_clipboard_curr.c_str(), true);
                    file_view->experimental_clipboard_prev = std::move(file_view->experimental_clipboard_curr);
                }
            }

            command_manager_enable_text_editor_undo_buffer(true); // ensure to begin to record history

            // render text editor
            const bool nochild = true;
            const bool text_changed = text_editor_render(file_view->text_editor, "Text_Editor", file_view->text_editor_palette, ImGui::GetContentRegionAvail(), nochild);
            file_view->is_syntax_dirty |= text_changed;

            // overlay
            Rect overlay_rect = ImGuiEx::GetContentRegion(WORLD_SPACE );
            overlay_rect.expand( -cfg->ui_textview_padding );
            fileview_draw_overlay(file_view->text_overlay_window_name.data, file_view->overlay_data[File_View_Overlay_Type_TEXT], overlay_rect, Vec2(0, 1));
            ImGuiEx::DebugRect( overlay_rect.min, overlay_rect.max, IM_COL32( 255, 255, 0, 127 ) );

            if ( HAS_FLAGS(cfg->flags, Config_Flag_EXPERIMENTAL_MULTI_SELECTION) )
            {
                command_manager_enable_text_editor_undo_buffer(false); // avoid recording events caused by graph serialisation
            }

            auto new_cursor_position = file_view->text_editor.GetCursorPosition();
            auto new_selected_text   = file_view->text_editor.GetSelectedText();
            auto new_line_text       = file_view->text_editor.GetCurrentLineText();

            auto is_line_text_modified = new_line_text != old_line_text &&
                                         new_cursor_position.mLine == old_cursor_position.mLine;
            auto is_selected_text_modified = new_cursor_position != old_cursor_position;

            text_view_changed  = is_line_text_modified;
            text_view_changed |= file_view->text_editor.mTextChanged;
            text_view_changed |= HAS_FLAGS(cfg->flags, Config_Flag_ISOLATION_ON) && is_selected_text_modified;
        }
        ImGui::EndChild();

         // Node_ViewItem EDITOR
        //-------------

        ImGui::SameLine();
        ImGuiWindowFlags flags = (ImGuiWindowFlags_)(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        Vec2 graph_editor_top_left_corner = ImGui::GetCursorPos();

        ImGui::BeginChild("graph", graph_editor_size, false, flags);
        {
            // Draw graph
            graph_view_changed |= graphview_draw(file_view->graph_view, dt);

            // Draw overlay: shortcuts
            Rect overlay_rect = ImGuiEx::GetContentRegion(WORLD_SPACE );
            overlay_rect.expand( -cfg->ui_textview_padding );
            fileview_draw_overlay(file_view->graph_overlay_window_name.data, file_view->overlay_data[File_View_Overlay_Type_GRAPH], overlay_rect, Vec2(1, 1));
            ImGuiEx::DebugRect( overlay_rect.min, overlay_rect.max, IM_COL32( 255, 255, 0, 127 ) );

            // Draw overlay: isolation mode ON/OFF
            if( HAS_FLAGS(cfg->flags, Config_Flag_ISOLATION_ON) )
            {
                Vec2 cursor_pos = graph_editor_top_left_corner + Vec2( cfg->ui_textview_padding);
                ImGui::SetCursorPos(cursor_pos);
                ImGui::Text("Isolation mode ON");
            }
        }
        ImGui::EndChild();

        if ( text_view_changed )
            file_view->signal_change.emit(File_View_Event_Type_TEXT_CHANGED);
        else if ( graph_view_changed )
            file_view->signal_change.emit(File_View_Event_Type_GRAPH_CHANGED);
    }
    ImGui::EndChild();
    ImGui::PopFont();
    ImGui::PopStyleColor();
}

bdc::String fileview_get_text( const File_View* file_view, bool isolation_on )
{
    std::string tmp;

    if ( !isolation_on )
    {
        tmp = file_view->text_editor.GetText();
    }
    else if ( file_view->text_editor.HasSelection() )
    {
        tmp = file_view->text_editor.GetSelectedText();
    }
    else
    {
        tmp = file_view->text_editor.GetCurrentLineText(); // By default, we consider the current line as the selection
    }

    ASSERT( tmp.size() < (u32_t)(-1));
    bdc::String result = bdc::string_tcopy( bdc::String{tmp.data(), (u32_t)tmp.size()} );
    return result;
}

void fileview_set_text(File_View* file_view, bdc::String new_text, bool isolation_on)
{
    String current_text =  fileview_get_text(file_view, isolation_on); 
    if ( bdc::string_compare(new_text, current_text) == 0 )
    {
        return;
    }

    if( isolation_on )
    {
        auto start = file_view->text_editor.GetCursorPosition();

        /* If there is no selection, selects current line */
        auto hasSelection = file_view->text_editor.HasSelection();
        auto selectionStart = file_view->text_editor.GetSelectionStart();
        auto selectionEnd = file_view->text_editor.GetSelectionEnd();

        // Select the whole line if no selection is set
        if (!hasSelection) {
            file_view->text_editor.MoveHome(false);
            file_view->text_editor.MoveEnd(true);
            file_view->text_editor.SetCursorPosition(Text_Editor::Coordinates(start.mLine, 0));
        }

        /* insert text (and select it) */
        file_view->text_editor.InsertText(new_text.c_str(), true);

        auto end = file_view->text_editor.GetCursorPosition();
        if (!hasSelection && start.mLine == end.mLine) // no selection and insert text is still on the same line
        {
            file_view->text_editor.SetSelection(selectionStart, selectionEnd);
        }
        NDBL_LOG(Verbosity_Message, "File_View", "Selected text updated from graph.\n");
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "File_View", "%s \n", new_text.c_str());
    }
    else
    {
        file_view->text_editor.SetText( new_text.c_str() );
        // auto cmd = std::make_shared<Cmd_ReplaceText>(current_content, text, &m_text_editor);
        // m_file->get_history()->push_command(cmd);

        NDBL_LOG(Verbosity_Message, "File_View", "Whole text updated from graph.\n");
        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "File_View", "%s \n", new_text.c_str());
    }

    file_view->is_syntax_dirty = true;
}

void fileview_set_AddUndoHandler(File_View* file_view, Text_Editor::Add_Undo_Handler handler )
{
	file_view->text_editor.mAddUndoHandler = handler;
}

void fileview_set_experimental_clipboard_auto_paste(File_View* file_view, bool enable)
{
    file_view->experimental_clipboard_auto_paste = enable;
    if( enable )
    {
        file_view->experimental_clipboard_prev = "";
    }
}

void fileview_draw_overlay(const bdc::String& title, const std::vector<File_View_Overlay_Data>& overlay_data, const Rect& rect, const Vec2& position)
{
    if( overlay_data.empty() ) return;

    Config* cfg = config();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, cfg->ui_overlay_window_bg_golor);
    ImGui::PushStyleColor(ImGuiCol_Border, cfg->ui_overlay_border_color);
    ImGui::PushStyleColor(ImGuiCol_Text, cfg->ui_overlay_text_color);
    Vec2 win_position = rect.top_left() + rect.size() * position;
    ImGui::SetNextWindowPos( win_position, ImGuiCond_Always, position);
    ImGui::SetNextWindowSize( rect.size(), ImGuiCond_Appearing);
    bool show = true;
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMouseInputs;

    if (ImGui::Begin(title.data, &show, flags) )
    {
        ImGui::Indent( cfg->ui_overlay_indent);
        std::for_each(overlay_data.begin(), overlay_data.end(), [](const File_View_Overlay_Data& _data) {
            ImGui::Text("%s:", _data.label.c_str());
            ImGui::SameLine(150);
            ImGui::Text("%s", _data.description.c_str());
        });
        ImGui::PopStyleColor(3);
    }
    ImGui::End();
}

void fileview_clear_overlay(File_View* file_view)
{
    for(auto& vec : file_view->overlay_data )
        vec.clear();
}

void fileview_push_overlay(File_View* file_view, File_View_Overlay_Data overlay_data, File_View_Overlay_Type overlay_type)
{
    file_view->overlay_data[overlay_type].push_back(overlay_data);
}

size_t fileview_size(const File_View* file_view)
{
    return file_view->text_editor.Size();
}

void fileview_refresh_overlay(File_View* file_view )
{
    Condition_Flags condition_flags = fileview_calc_condition_flags(file_view);
    for (const Action& action: action_manager()->actions )
    {
        if( ( action.condition_flags & condition_flags) == condition_flags && (action.condition_flags & Condition_Flags_HIGHLIGHTED) )
        {
            bdc::String label = action.label;
            label.size = label.size > 12 ? 12 : label.size;
            bdc::String shortcut_str = action.shortcut.to_string();
            File_View_Overlay_Type overlay_type;
            
            if (action.condition_flags & Condition_Flags_HIGHLIGHTED_IN_TEXT_EDITOR)
                overlay_type = File_View_Overlay_Type_TEXT;
            else
                overlay_type = File_View_Overlay_Type_GRAPH;

            fileview_push_overlay(file_view, {label, shortcut_str}, overlay_type);
        }
    }
}

} // namespace ndbl