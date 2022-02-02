#pragma once

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <observe/event.h>
#include <ImGuiColorTextEdit/TextEditor.h> // for coordinates

#include <nodable/Reflect.h>
#include <nodable/Nodable.h>
#include <nodable/Log.h>

namespace Nodable
{
    // forward declarations
    class Node;
    class GraphNode;
    class Language;
    class AbstractNodeFactory;
    class History;
    class FileView;

	class File
	{
	public:
		File(std::string, const char* /*_content*/);
        ~File();

        observe::Event<Node*> m_onExpressionParsedIntoGraph;

		std::string                      getName()const
        {
            auto lastSlash = m_path.find_last_of('/');
            return m_path.substr(lastSlash);
        }

        std::string                      getPath()const { return m_path; }
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

        static File*                     OpenFile(std::string _filePath);

	private:
        bool                       m_open;
		bool                       m_modified;
		std::string                m_path;
		const Language*            m_language;
		const AbstractNodeFactory* m_factory;
		FileView*                  m_view;
		History*                   m_history;
		GraphNode*                 m_graph;
    };
}
