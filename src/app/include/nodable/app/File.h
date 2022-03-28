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
#include <nodable/core/HeadlessNodeFactory.h>
#include <nodable/app/History.h>

namespace Nodable
{
    // forward declarations
    class Node;
    class GraphNode;
    class Language;
    class History;
    class FileView;
    class AppContext;

	class File
	{
	public:
		File(AppContext* _ctx, const std::string& _path, const std::string& _name);
        ~File();

        observe::Event<GraphNode*> m_on_graph_changed_evt;

        bool                             read_from_disk();
        void                             write_to_disk();
        bool                             update();
        AppContext*                      get_context() { return m_context; }
        inline GraphNode*                get_graph() { return m_graph; }
        inline History*                  get_history() { return &m_history; }
        const char*                      get_name()const { return m_name.c_str(); }
        const char*                      get_path()const { return m_path.c_str(); }
        inline FileView*                 get_view()const { return m_view; };
        inline void                      set_changed_flag(bool _value = true) { m_modified = _value; }
        inline bool                      has_changed() const { return m_modified; }
        bool                             update_graph();
        bool                             update_graph(std::string &_expression);
        inline bool                      is_open()  { return m_open; }
        static File*                     open(AppContext* _ctx, const std::string& _path, const std::string& _name);
    private:
		AppContext*                m_context;
        bool                       m_open;
		bool                       m_modified;
		std::string                m_name;
		std::string                m_path;
		const HeadlessNodeFactory  m_factory;
		FileView*                  m_view;
		History                    m_history;
		GraphNode*                 m_graph;
    };
}
