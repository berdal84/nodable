#pragma once
#include "Nodable.h"
#include "Log.h"
#include "Node.h"
#include "GraphNode.h"
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
        ~File() { delete getHistory(); }
		std::string                      getName()const { return std::string {path.filename().u8string()}; }
        std::string                      getPath()const { return std::string {path.u8string()}; }
		void                             save();
		UpdateResult                     update();
		void                             setModified() { modified = true; }
		bool                             isModified() { return modified; }
		bool                             evaluateExpression(std::string&);
		bool                             evaluateSelectedExpression();
        inline const Language* getLanguage()const { return language; }
		static File*                     OpenFile(std::filesystem::path _filePath);

		inline History* getHistory() {
			return getComponent<History>();
		}
        bool& isOpen();

	private:
	    bool open = true;
		bool                      modified = false;
		std::filesystem::path     path;		
		const Language*           language;
		MIRROR_CLASS(File)(
		    MIRROR_PARENT(Node)
		);


    };
}
