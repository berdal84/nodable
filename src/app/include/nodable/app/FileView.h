#pragma once

#include <ImGuiColorTextEdit/TextEditor.h>
#include <observe/observer.h>

#include <nodable/core/reflection/reflection>
#include <nodable/app/types.h>
#include <nodable/app/View.h>

namespace Nodable
{
    // forward declarations
    class File;
    class IAppCtx;

	class FileView : public View
	{
	public:
		explicit FileView(IAppCtx& _ctx, File& _file);
		~FileView() override = default;

		void                           init();
		bool                           draw() override;
		bool                           text_has_changed() const { return m_text_has_changed; }
		void                           set_text(const std::string&);
		std::string                    get_selected_text()const;
		std::string                    get_text()const;
		void                           replace_selected_text(const std::string &_val);
		TextEditor*					   get_text_editor(){ return &m_text_editor; }
		void                           set_cursor_position(const TextEditor::Coordinates& _cursorPosition) { m_text_editor.SetCursorPosition(_cursorPosition); }
		TextEditor::Coordinates        get_cursor_position()const { return m_text_editor.GetCursorPosition(); }
		void						   set_undo_buffer(TextEditor::IExternalUndoBuffer*);
        void                           draw_info()const;
        void                           experimental_clipboard_auto_paste(bool);
        bool                           experimental_clipboard_auto_paste()const { return m_experimental_clipboard_auto_paste; }
	private:
		File&        m_file;
		TextEditor   m_text_editor;
		bool         m_text_has_changed;
		float        m_child1_size;
		float        m_child2_size;
        std::string  m_experimental_clipboard_curr;
        std::string  m_experimental_clipboard_prev;
        bool         m_experimental_clipboard_auto_paste;
        observe::Observer m_graph_change_obs;

        REFLECT_ENABLE(FileView, View)

    };
}
