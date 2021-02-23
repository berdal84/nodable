#pragma once
#include "Nodable.h"
#include "View.h"
#include "ImGuiColorTextEdit/TextEditor.h"
#include <mirror.h>

namespace Nodable {	

	class FileView : public View
	{
	public:
		FileView():m_textEditor(nullptr), m_hasChanged(false){};
		void                           init();
		bool                           draw();
		virtual bool update(){return true; };
		bool                           hasChanged() { return this->m_hasChanged; }
		void                           setText(const std::string&);
		std::string                    getSelectedText()const;
		std::string                    getText()const;
		void                           replaceSelectedText(std::string _val);
		TextEditor*					   getTextEditor(){ return m_textEditor; }
		void                           setTextEditorCursorPosition(const TextEditor::Coordinates& _cursorPosition) { m_textEditor->SetCursorPosition(_cursorPosition); }
		TextEditor::Coordinates        getTextEditorCursorPosition()const { return m_textEditor != nullptr ? m_textEditor->GetCursorPosition() : TextEditor::Coordinates(0, 0); }
		void						   setUndoBuffer(TextEditor::ExternalUndoBufferInterface*);
	private:
		File*        getFile();
		TextEditor*  m_textEditor;
		bool         m_hasChanged;
		MIRROR_CLASS(FileView)(
			MIRROR_PARENT(View));

        void drawFileInfo();
    };
}
