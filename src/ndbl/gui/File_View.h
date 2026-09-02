#pragma once

#include "bdc/String.hpp"
#include "ImGuiColorTextEdit/TextEditor.h"
#include "bdc/Types.hpp"
#include "core/reflection/Type_Descriptor.h"
#include "gui/geometry/Rect.h"
#include "tools/core/Signals.h"

namespace ndbl
{
    // forward declarations
    class File;
    class Graph_View;
    class Graph;

    typedef u8_t File_View_Overlay_Pos;
    enum File_View_Overlay_Pos_ : u8_t
    {
        File_View_Overlay_Pos_Top,
        File_View_Overlay_Pos_Right,
        File_View_Overlay_Pos_Bottom,
        File_View_Overlay_Pos_Left,
    };

    using File_View_Overlay_Type = u8_t;
    enum File_View_Overlay_Type_ : u8_t
    {
        File_View_Overlay_Type_TEXT,
        File_View_Overlay_Type_GRAPH,
        File_View_Overlay_Type_COUNT
    };

    struct File_View_Overlay_Data
    {
        bdc::String           label;
        bdc::String           description;
        File_View_Overlay_Pos position;
    } ;

    typedef u8_t File_View_Event_Type;
    enum File_View_Event_Type_ : u8_t
    {
        File_View_Event_Type_GRAPH_CHANGED = 0,
        File_View_Event_Type_TEXT_CHANGED,
    };

    typedef u16_t Condition;
    enum Condition_ : u16_t
    {
        Condition_DISABLE                          = 0,
        Condition_ENABLE_IF_HAS_SELECTION          = 1 << 0,
        Condition_ENABLE_IF_HAS_NO_SELECTION       = 1 << 1,
        Condition_ENABLE_IF_HAS_GRAPH              = 1 << 3,
        Condition_DISABLE_IF_DRAGGING_THIS_SLOT    = 1 << 4,
        Condition_DISABLE_IF_DRAGGING_NON_THIS_SLOT= 1 << 5,
        Condition_ONLY_FROM_GRAPH_EDITOR_CONTEXTUAL= 1 << 6,
        Condition_ENABLE                           = Condition_ENABLE_IF_HAS_SELECTION
                                                   | Condition_ENABLE_IF_HAS_NO_SELECTION
                                                   | Condition_ENABLE_IF_HAS_GRAPH,
        Condition_HIGHLIGHTED_IN_GRAPH_EDITOR      = 1 << 10,
        Condition_HIGHLIGHTED_IN_TEXT_EDITOR       = 1 << 11,
        Condition_HIGHLIGHTED                      = Condition_HIGHLIGHTED_IN_GRAPH_EDITOR
                                                   | Condition_HIGHLIGHTED_IN_TEXT_EDITOR,
    };

    struct File_View
	{
        DECLARE_REFLECT

        std::array<std::vector<File_View_Overlay_Data>, File_View_Overlay_Type_COUNT> 
                                overlay_data                        = {};
        tools::Signal<void(File_View_Event_Type)>
                                signal_change                       = {};
        File*                   file                                = nullptr;
        Graph_View*             graph_view                          = nullptr;
        bdc::String             text_overlay_window_name;
        bdc::String             graph_overlay_window_name;
		TextEditor              text_editor                         = {};
		float                   text_child_size                     = 0.3f;
		float                   graph_child_size                    = 0.7f;
        bdc::String             experimental_clipboard_curr;
        bdc::String             experimental_clipboard_prev;
        bool                    experimental_clipboard_auto_paste   = false;
        bool                    is_history_dragged                  = false;
    };

    void                            fileview_init(File_View*, File*);
    void                            fileview_deinit(File_View*);
    void                            fileview_update(File_View*, float dt);
    void                            fileview_draw(File_View*, float dt);
    bdc::String                     fileview_get_text(const File_View*, bool isolation_on = false);
    void                            fileview_set_text(File_View*, bdc::String, bool isolation_on = false);
    static void                     fileview_set_cursor_position(File_View* file_view, const TextEditor::Coordinates& _cursorPosition) { file_view->text_editor.SetCursorPosition(_cursorPosition); }
    static TextEditor::Coordinates  fileview_get_cursor_position(const File_View* file_view) { return file_view->text_editor.GetCursorPosition(); }
    void	                        fileview_set_undo_buffer(File_View*, TextEditor::IExternalUndoBuffer*);
    void                            fileview_set_experimental_clipboard_auto_paste(File_View*, bool /* enable*/);
    void                            fileview_clear_overlay(File_View*);
    void                            fileview_push_overlay(File_View*, File_View_Overlay_Data, File_View_Overlay_Type) ;
    void                            fileview_refresh_overlay(File_View*, Condition);
    void                            fileview_draw_overlay(const bdc::String& title, const std::vector<File_View_Overlay_Data>& overlay_data, const tools::Rect& rect, const tools::Vec2& position);
    size_t                          fileview_size(const File_View*);
}
