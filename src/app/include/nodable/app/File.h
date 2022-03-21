#pragma once

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <observe/event.h>
#include <ImGuiColorTextEdit/TextEditor.h> // for coordinates

#include <nodable/core/reflection/R.h>
#include <nodable/app/types.h>
#include <nodable/core/Log.h>
#include <nodable/app/AppNodeFactory.h>
#include <nodable/app/History.h>

namespace Nodable
{
    // forward declarations
    class Node;
    class GraphNode;
    class Language;
    class History;
    class FileView;
    struct AppContext;

	class File
	{
	public:
		File(AppContext* _ctx, const std::string& _path, const std::string& _name, const char* /*_content*/);
        ~File();

        observe::Event<GraphNode*> m_on_graph_changed_evt;

		std::string                      getName()const { return m_name; }
        std::string                      getPath()const { return m_path; }
		void                             save();
		bool                             update();
		inline void                      setModified() { m_modified = true; }
		inline bool                      isModified() { return m_modified; }
		bool                             evaluateExpression(std::string&);
		bool                             evaluateSelectedExpression();
        inline FileView*                 getView()const { return m_view; };
		inline History*                  getHistory() { return &m_history; }
        inline bool                      isOpen()  { return m_open; }
        inline GraphNode*                getGraph() { return m_graph; }
        static File*                     OpenFile(AppContext* _ctx, const std::string& _path, const std::string& _name);
        AppContext*                      get_context() { return m_context; }
	private:
		AppContext*                m_context;
        bool                       m_open;
		bool                       m_modified;
		std::string                m_name;
		std::string                m_path;
		const AppNodeFactory       m_factory;
		FileView*                  m_view;
		History                    m_history;
		GraphNode*                 m_graph;
    };
}
