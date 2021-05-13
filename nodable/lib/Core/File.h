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
        ~File();
		std::string                      getName()const { return std::string {m_path.filename().u8string()}; }
        std::string                      getPath()const { return std::string {m_path.u8string()}; }
		void                             save();
		UpdateResult                     update();
		void                             setModified() { m_modified = true; }
		bool                             isModified() { return m_modified; }
		bool                             evaluateExpression(std::string&);
		bool                             evaluateSelectedExpression();
        inline const Language* getLanguage()const { return m_language; }
		static File*                     OpenFile(std::filesystem::path _filePath);

		inline History* getHistory() {
			return getComponent<History>();
		}
        bool& isOpen();

	private:
	    bool                   m_open = true;
		bool                   m_modified = false;
		std::filesystem::path  m_path;
		const Language*        m_language;
		const AbstractNodeFactory* m_factory;

		MIRROR_CLASS(File)(
		    MIRROR_PARENT(Node)
		);
    };
}
