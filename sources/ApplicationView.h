#pragma once

#include "Nodable.h"
#include <imgui/imgui.h>
#include <SDL2/include/SDL.h>
#include <string>
#include "View.h"
#include <ImGuiColorTextEdit/TextEditor.h>

namespace Nodable
{
	/*
		This class contain the basic setup for and OpenGL/SDL basic window.
	*/
	class ApplicationView : public View
	{
	public:
		ApplicationView(const char* _name, Application* _application);
		~ApplicationView();

		/* call this every frame */
		bool draw();

		/* call this once just after the instantiation. */
		bool init();

		/* Text Editor related getters/setters */
		void                           setTextEditorContent              (const std::string&);
		void                           setTextEditorCursorPosition       (const TextEditor::Coordinates& _cursorPosition) { textEditor->SetCursorPosition(_cursorPosition); }
		TextEditor::Coordinates        getTextEditorCursorPosition       ()const                                          { return textEditor!=nullptr?textEditor->GetCursorPosition():TextEditor::Coordinates(0,0); }
		std::string                    getTextEditorHighlightedExpression()const;
		std::string                    getTextEditorContent              ()const;
		void                           replaceHighlightedPortionInTextEditor(std::string _val);

	private:
		TextEditor        *textEditor;
		Application       *application;
		SDL_Window        *sdlWindow;
		SDL_GLContext     glcontext;
		ImVec4            clear_color = ImColor(50, 50, 50); // used to fill the framebuffer each frame.
	};
}