#pragma once

#include <ImGuiColorTextEdit/TextEditor.h>
#include <observe/observer.h>

#include "fw/core/reflection/reflection"
#include "fw/gui/View.h"

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
    enum OverlayType_ {
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
		explicit FileView(File& _file);
		~FileView() override = default;

		void                           init();
		bool                           text_has_changed() const { return m_text_has_changed; }
		void                           set_text(const std::string&);
		std::string                    get_selected_text()const;
		std::string                    get_text()const;
		void                           replace_selected_text(const std::string&);
        void                           replace_text(const std::string&);
		TextEditor*					   get_text_editor(){ return &m_text_editor; }
		void                           set_cursor_position(const TextEditor::Coordinates& _cursorPosition) { m_text_editor.SetCursorPosition(_cursorPosition); }
		TextEditor::Coordinates        get_cursor_position()const { return m_text_editor.GetCursorPosition(); }
		void						   set_undo_buffer(TextEditor::IExternalUndoBuffer*);
        void                           draw_info()const;
        void                           experimental_clipboard_auto_paste(bool);
        bool                           experimental_clipboard_auto_paste()const { return m_experimental_clipboard_auto_paste; }
        void                           push_overlay(OverlayData, OverlayType) ;
        void                           clear_overlay();
        void                           draw_overlay(const char* title, const std::vector<OverlayData>& overlay_data, ImRect rect,  ImVec2 position);

    private:
        bool draw_implem() override;
        std::array<std::vector<OverlayData>, OverlayType_COUNT> m_overlay_data;

		File&        m_file;
        std::string  m_text_overlay_window_name;
        std::string  m_graph_overlay_window_name;
		TextEditor   m_text_editor;
		bool         m_text_has_changed;
		float        m_child1_size;
		float        m_child2_size;
        std::string  m_experimental_clipboard_curr;
        std::string  m_experimental_clipboard_prev;
        bool         m_experimental_clipboard_auto_paste;
        observe::Observer m_graph_change_obs;

        REFLECT_DERIVED_CLASS()

    };
}
