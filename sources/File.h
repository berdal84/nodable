#pragma once
#include "Nodable.h"
#include "Log.h"
#include "Entity.h"
#include "Container.h"
#include "History.h"

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
			const char* _name);

		std::string                      getName()const { return name; }	
		void                             save();
		bool                             update();
		void                             setModified() { modified = true; }
		bool                             isModified() { return modified; }
		bool                             evaluateExpression(std::string&);
		bool                             evaluateSelectedExpression();

		static File*                     CreateFileWithPath                    (const char* _filePath);
		static std::string               BrowseForFileAndReturnItsAbsolutePath (SDL_Window* currentWindow);

		inline Container* getContainer() {
			return getComponent<Container>("container");
		}
		
		inline History* getHistory() {
			return getComponent<History>("history");
		}

	private:
		bool                      modified = false;
		std::string               path;
		std::string               name;
	};
}
