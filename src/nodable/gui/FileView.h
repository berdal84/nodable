#pragma once

#include <observe/observer.h>

#include "fw/core/reflection/reflection"
#include "fw/gui/View.h"

#include "Condition.h"
#include "Isolation.h"
#include "types.h"

namespace ndbl
{
    // forward declarations
    class File;
    class IAppCtx;

    enum OverlayPos {
        OverlayPos_Top,
        OverlayPos_Right,
        OverlayPos_Bottom,
        OverlayPos_Left,
    };

    using OverlayType = int;
    enum OverlayType_
    {
        OverlayType_TEXT,
        OverlayType_GRAPH,
        OverlayType_COUNT
    };

    typedef struct {
        std::string label;
        std::string description;
        OverlayPos position;
    } OverlayData;

    class FileView : public fw::View
	{
	public:
		explicit FileView( File& _file);
        FileView(const File&) = delete;
		~FileView() override = default;

		void                           init();
        bool                           onDraw() override;
        bool                           changed() const { return m_focused_text_changed || m_is_graph_dirty; }
        bool                           focused_text_changed() const { return m_focused_text_changed; }
        bool                           is_graph_dirty() const { return m_is_graph_dirty; }
        void                           set_dirty(bool b) { m_focused_text_changed = m_is_graph_dirty = b; }
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
        void                           push_overlay(OverlayData, OverlayType) ;
        void                           refresh_overlay(Condition condition);
        void                           draw_overlay(const char* title, const std::vector<OverlayData>& overlay_data, fw::Rect rect,  fw::Vec2 position);
        size_t                         size() const;

    private:
        std::array<std::vector<OverlayData>, OverlayType_COUNT> m_overlay_data;
        bool         m_focused_text_changed;
        bool         m_is_graph_dirty;
        File&        m_file;
        std::string  m_text_overlay_window_name;
        std::string  m_graph_overlay_window_name;
		TextEditor   m_text_editor;
		float        m_child1_size;
		float        m_child2_size;
        std::string  m_experimental_clipboard_curr;
        std::string  m_experimental_clipboard_prev;
        bool         m_experimental_clipboard_auto_paste;
        observe::Observer m_graph_changed_observer;

        REFLECT_DERIVED_CLASS()
    };
}
