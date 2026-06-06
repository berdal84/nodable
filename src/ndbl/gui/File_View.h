#pragma once

#include "Condition.h"
#include "ImGuiColorTextEdit/TextEditor.h"
#include "Isolation.h"
#include "core/reflection/Type_Descriptor.h"
#include "gui/geometry/Rect.h"
#include "tools/core/Signals.h"

namespace ndbl
{
    // forward declarations
    class File;
    class Graph_View;
    class Graph;

    enum Overlay_Pos {
        Overlay_Pos_Top,
        Overlay_Pos_Right,
        Overlay_Pos_Bottom,
        Overlay_Pos_Left,
    };

    using Overlay_Type = int;
    enum Overlay_Type_
    {
        Overlay_Type_TEXT,
        Overlay_Type_GRAPH,
        Overlay_Type_COUNT
    };

    struct Overlay_Data
    {
        std::string label;
        std::string description;
        Overlay_Pos position;
    } ;

    class File_View
	{
    public:
        DECLARE_REFLECT
        explicit File_View();
        File_View(const File_View&) = delete;
		~File_View() = default;

        tools::Simple_Signal signal_text_view_changed;
        tools::Simple_Signal signal_graph_view_changed;

        void                           update(float d);
        void                           init(File& _file);
        void                           draw(float dt);
        std::string                    get_text(Isolation = Isolation_OFF)const;
        void                           set_text(const std::string&, Isolation mode = Isolation_OFF);
        TextEditor*					   get_text_editor(){ return &m_text_editor; }
        void                           set_cursor_position(const TextEditor::Coordinates& _cursorPosition) { m_text_editor.SetCursorPosition(_cursorPosition); }
        TextEditor::Coordinates        get_cursor_position()const { return m_text_editor.GetCursorPosition(); }
        void						   set_undo_buffer(TextEditor::IExternalUndoBuffer*);
        void                           draw_info_panel()const;
        void                           experimental_clipboard_auto_paste(bool);
        bool                           experimental_clipboard_auto_paste()const { return m_experimental_clipboard_auto_paste; }
        void                           clear_overlay();
        void                           push_overlay(Overlay_Data, Overlay_Type) ;
        void                           refresh_overlay(Condition condition);
        void                           draw_overlay(const char* title, const std::vector<Overlay_Data>& overlay_data, const tools::Rect& rect, const tools::Vec2& position);
        size_t                         size() const;
    private:
        std::array<std::vector<Overlay_Data>, Overlay_Type_COUNT> m_overlay_data;
        File*        m_file;
        Graph_View*  m_graph_view;
        std::string  m_text_overlay_window_name;
        std::string  m_graph_overlay_window_name;
		TextEditor   m_text_editor;
		float        m_child1_size;
		float        m_child2_size;
        std::string  m_experimental_clipboard_curr;
        std::string  m_experimental_clipboard_prev;
        bool         m_experimental_clipboard_auto_paste;
        bool         m_is_history_dragged = false;
    };
}
