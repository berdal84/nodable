#pragma once
#include "Nodable.h"
#include "Log.h"
#include "Node.h"
#include "Container.h"
#include "History.h"
#include <mirror.h>

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <filesystem>

#include "ImGuiColorTextEdit/TextEditor.h" // for coordinates

namespace Nodable
{
	class File: public Node
	{
	public:
		File(std::filesystem::path, const char* /*_content*/);

		std::string                      getName()const { return path.filename(); }	
		void                             save();
		bool                             update();
		void                             setModified() { modified = true; }
		bool                             isModified() { return modified; }
		bool                             evaluateExpression(std::string&);
		bool                             evaluateSelectedExpression();

		static File*                     CreateFileWithPath                    (std::filesystem::path _filePath);
		static std::string               BrowseForFileAndReturnItsAbsolutePath (SDL_Window* currentWindow);
		
		inline History* getHistory() {
			return getComponent<History>();
		}

	private:
		bool                      modified = false;
		std::filesystem::path     path;		
		const Language*           language;
		MIRROR_CLASS(File)();
	};
}
