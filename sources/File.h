#pragma once
#include "Nodable.h"
#include "Log.h"
#include "Entity.h"
#include "Container.h"

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>

#include "ImGuiColorTextEdit/TextEditor.h" // for coordinates

namespace Nodable
{
	class File: public Entity
	{
	public:
		File(const char* _path,
			 const char* _content,
			 const char* _name):
			content(_content),
			path(_path),
			name(_name),
			textEditorCursorPosition(0,0){}

		std::string                      getText()const { return content; }
		std::string                      getName()const { return name; }	
		const TextEditor::Coordinates&   getCursorPosition()const { return textEditorCursorPosition; }
		void                             setCursorPosition(const TextEditor::Coordinates& _textEditorCursorPosition) { textEditorCursorPosition = _textEditorCursorPosition;  }
		void                             save()const;
		void                             setContent(std::string&);
		bool                             evaluateExpression(std::string&);
		bool                             update();
		bool                             clearContextAndEvalHighlightedExpression();

		static File*                     CreateFileWithPath                    (const char* _filePath);
		static std::string               BrowseForFileAndReturnItsAbsolutePath (SDL_Window* currentWindow);

		inline Container* getContainer() {
			return getComponent("container")->getAs<Container*>();
		}

	private:
		std::string               content;
		std::string               path;
		std::string               name;
		TextEditor::Coordinates   textEditorCursorPosition;
	};
}
