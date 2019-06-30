#pragma once
#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include "Log.h"
#include <string>
#include <algorithm>

namespace Nodable
{
	class File
	{
	public:
		File(const char* _path, const char* _content, const char* _name):content(_content),path(_path),name(_name){}

		std::string        getContent()const { return content; }
		std::string        getName()const { return name; }	

		static File*       CreateFileWithPath                    (const char* _filePath);
		static std::string BrowseForFileAndReturnItsAbsolutePath (SDL_Window* currentWindow);
	private:
		std::string content;
		std::string path;
		std::string name;
	};
}
