#pragma once

#include "mirror.h"
#include "View.h"
#include "ImGuiColorTextEdit/TextEditor.h"

namespace Nodable {	

    class File;

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
		TextEditor*					   getTextEditor(){ return m_textEditor.get(); }
		void                           setTextEditorCursorPosition(const TextEditor::Coordinates& _cursorPosition) { m_textEditor->SetCursorPosition(_cursorPosition); }
		TextEditor::Coordinates        getTextEditorCursorPosition()const { return m_textEditor != nullptr ? m_textEditor->GetCursorPosition() : TextEditor::Coordinates(0, 0); }
		void						   setUndoBuffer(TextEditor::ExternalUndoBufferInterface*);
	private:
        std::shared_ptr<File> getFile();
		std::unique_ptr<TextEditor>  m_textEditor;
		const TextEditor::LanguageDefinition* m_textEditorLanguageDefinition;
		bool         m_hasChanged;
		MIRROR_CLASS(FileView)(
			MIRROR_PARENT(View));
	};
}
