#pragma once
#include "Nodable.h"
#include "View.h"
#include "ImGuiColorTextEdit/TextEditor.h"

class TextEditor;

namespace Nodable {	

	class FileView : public View
	{
	public:
		COMPONENT_CONSTRUCTOR(FileView)
		void                           init();
		bool                           draw();
		bool                           hasChanged() { return this->m_hasChanged; }
		void                           setTextEditorContent(const std::string&);
		std::string                    getTextEditorHighlightedExpression()const;
		std::string                    getTextEditorContent()const;
		void                           replaceHighlightedPortionInTextEditor(std::string _val);
		void                           setTextEditorCursorPosition(const TextEditor::Coordinates& _cursorPosition) { m_textEditor->SetCursorPosition(_cursorPosition); }
		TextEditor::Coordinates        getTextEditorCursorPosition()const { return m_textEditor != nullptr ? m_textEditor->GetCursorPosition() : TextEditor::Coordinates(0, 0); }

	private:
		File*        getFile();
		TextEditor*  m_textEditor;
		bool         m_hasChanged;
	};
}
