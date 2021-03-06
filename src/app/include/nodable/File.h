#pragma once

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <filesystem>
#include <observe/event.h>
#include <ImGuiColorTextEdit/TextEditor.h> // for coordinates
#include <mirror.h>

#include <nodable/Nodable.h>
#include <nodable/Log.h>

namespace Nodable
{
    // forward declarations
    class GraphNode;
    class Language;
    class AbstractNodeFactory;
    class ProgramNode;
    class History;
    class FileView;

	class File
	{
	public:
		File(std::filesystem::path, const char* /*_content*/);
        ~File();

        observe::Event<ProgramNode*>     m_onExpressionParsedIntoGraph;

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
