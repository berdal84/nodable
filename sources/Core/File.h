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
        virtual ~File(){};
		void                             save();
		UpdateResult                     update();
		void                             setModified() { modified = true; }
		bool                             isModified() { return modified; }
		bool                             evaluateExpression(std::string&);
		bool                             evaluateSelectedExpression();

		static std::unique_ptr<File>     OpenFile(std::filesystem::path _filePath);

		inline History* getHistory() {
			return getComponent<History>();
		}

        inline Container* getInnerContainer() {
            return this->innerContainer.get();
        }

        inline std::string getName()const
        {
            return std::string {path.filename().u8string()};
        }

	private:
	    std::unique_ptr<Container> innerContainer;
		bool                      modified = false;
		std::filesystem::path     path;		
		const Language*           language;
		MIRROR_CLASS(File)();
    };
}
