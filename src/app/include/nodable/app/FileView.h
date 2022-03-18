#pragma once

#include <nodable/core/reflection/R.h>
#include <ImGuiColorTextEdit/TextEditor.h>
#include <observe/observer.h>

#include <nodable/app/types.h>
#include <nodable/app/View.h>

namespace Nodable
{
    // forward declarations
    class File;
    struct AppContext;

	class FileView : public View
	{
	public:
		explicit FileView(AppContext* _ctx, File* _file);
		~FileView() override = default;

		observe::Observer m_observer;

		void                           init();
		bool                           draw() override;
		bool                           hasChanged() const { return this->m_hasChanged; }
		void                           setText(const std::string&);
		std::string                    getSelectedText()const;
		std::string                    getText()const;
		void                           replaceSelectedText(const std::string &_val);
		TextEditor*					   getTextEditor(){ return &m_textEditor; }
		void                           setTextEditorCursorPosition(const TextEditor::Coordinates& _cursorPosition) { m_textEditor.SetCursorPosition(_cursorPosition); }
		TextEditor::Coordinates        getTextEditorCursorPosition()const { return m_textEditor.GetCursorPosition(); }
		void						   setUndoBuffer(TextEditor::IExternalUndoBuffer*);
        void                           drawFileInfo()const;
        void                           experimental_clipboard_auto_paste(bool val)
        {
            m_experimental_clipboard_auto_paste = val;
            if( val ) {
                m_experimental_clipboard_prev = "";
            }
        }
        bool                           experimental_clipboard_auto_paste()const { return m_experimental_clipboard_auto_paste; }
	private:
		File*        m_file;
		TextEditor   m_textEditor;
		bool         m_hasChanged;
		float        m_childSize1 = 0.3f;
		float        m_childSize2 = 0.7f;

        std::string  m_experimental_clipboard_curr;
        std::string  m_experimental_clipboard_prev;
        bool         m_experimental_clipboard_auto_paste = false;

        R_DERIVED(FileView)
        R_EXTENDS(View)
        R_END
    };
}
