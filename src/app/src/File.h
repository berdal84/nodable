#pragma once
#include "Nodable.h"
#include "Log.h"
#include <mirror.h>
#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <filesystem>

#include "ImGuiColorTextEdit/TextEditor.h" // for coordinates

// forward declarations:
namespace Nodable
{
    namespace core
    {
        class GraphNode;
        class Language;
        class AbstractNodeFactory;
    }

    namespace app
    {
        class History;
        class FileView;
    }
}

namespace Nodable::app
{
    using namespace core;

	class File
	{
	public:
		File(std::filesystem::path, const char* /*_content*/);
        ~File();

		std::string                      getName()const { return std::string {m_path.filename().u8string()}; }
        std::string                      getPath()const { return std::string {m_path.u8string()}; }
		void                             save();
		bool                             update();
		inline void                      setModified() { m_modified = true; }
		inline bool                      isModified() { return m_modified; }
		bool                             evaluateExpression(std::string&);
		bool                             evaluateSelectedExpression();
        inline const Language*           getLanguage()const { return m_language; }
        inline FileView*                 getView()const { return m_view; };
		inline History*                  getHistory() { return m_history; }
        inline bool                      isOpen()  { return m_open; }
        inline GraphNode*                getGraph() { return m_graph; }

        static File*                     OpenFile(std::filesystem::path _filePath);

	private:
        bool                       m_open;
		bool                       m_modified;
		std::filesystem::path      m_path;
		const Language*            m_language;
		const AbstractNodeFactory* m_factory;
		FileView*                  m_view;
		History*                   m_history;
		GraphNode*                 m_graph;
    };
}
